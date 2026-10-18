/*=============================================================================
    Debug.cpp: spdlog-backed implementation of the dbg* logging API.

    All runtime messages (info, warning, fatal) are written to cataclysm.log
    in the process working directory (the game install root) and echoed to
    stdout and the MSVC output window. Worktree builds suffix the log filename
    with the generated worktree id.

    CrashLog.cpp appends fatal context directly to the same generated log file.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <intrin.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <memory>

// Use spdlog in header-only mode with std::format to avoid linking external
// fmt/spdlog static libs (which would cause CRT or ASan annotation mismatches).
#define SPDLOG_HEADER_ONLY
#define SPDLOG_USE_STD_FORMAT
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/msvc_sink.h>

#include "BuildContext.generated.h"

#ifndef CATACLYSM_BUILD_BRANCH
#define CATACLYSM_BUILD_BRANCH "unknown"
#endif

#ifndef CATACLYSM_BUILD_WORKTREE
#define CATACLYSM_BUILD_WORKTREE "unknown"
#endif

#ifndef CATACLYSM_BUILD_LOG_FILE
#define CATACLYSM_BUILD_LOG_FILE "cataclysm.log"
#endif

// HW1 types via extern "C" so the symbols link correctly with C translation units
extern "C" {
#include "types.h"
#include "debug.h"
#include "CrashLog.h"
}

// Undefine conflicting macros introduced by the HW1 headers
#ifdef swap
#undef swap
#endif

/*=============================================================================
    Data - exported as C symbols so C translation units can reference them
=============================================================================*/

extern "C" sdword dbgInt3Enabled = 1;
extern "C" sdword dbgAllowInterrupts = 1;
extern "C" char dbgFatalErrorString[DBG_BufferLength] = {0};

#if DBG_STACK_CONTEXT
extern "C" udword dbgStackBase = 0;
#endif

static bool s_dbgLogInitialized = false;

namespace
{
constexpr char kLogFileName[] = CATACLYSM_BUILD_LOG_FILE;

struct DbgActiveLogContext
{
    const char *api;
    const char *file;
    sdword line;
    char format[DBG_BufferLength];
    char rendered[DBG_BufferLength];
};

thread_local DbgActiveLogContext s_dbgActiveLogContext = {};
thread_local bool s_dbgHandlingLoggingFailure = false;
thread_local char s_dbgResolvedCallsiteFile[MAX_PATH] = {0};
bool s_dbgSymbolsInitialized = false;

#pragma intrinsic(_ReturnAddress)

void dbgCopyText(char *dest, const char *src)
{
    if (dest == nullptr)
    {
        return;
    }

    if (src == nullptr)
    {
        dest[0] = '\0';
        return;
    }

    _snprintf(dest, DBG_BufferMax, "%s", src);
    dest[DBG_BufferMax] = '\0';
}

void dbgInitSymbols(void)
{
    if (s_dbgSymbolsInitialized)
    {
        return;
    }

    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    if (SymInitialize(GetCurrentProcess(), nullptr, TRUE) || GetLastError() == ERROR_INVALID_PARAMETER)
    {
        s_dbgSymbolsInitialized = true;
    }
}

void dbgResolveCallerLocation(void *returnAddress, const char **file, sdword *line)
{
    IMAGEHLP_LINE64 lineInfo;
    DWORD displacement = 0;

    if (file != nullptr)
    {
        *file = nullptr;
    }
    if (line != nullptr)
    {
        *line = 0;
    }

    if (returnAddress == nullptr)
    {
        return;
    }

    dbgInitSymbols();
    if (!s_dbgSymbolsInitialized)
    {
        return;
    }

    memset(&lineInfo, 0, sizeof(lineInfo));
    lineInfo.SizeOfStruct = sizeof(lineInfo);
    if (SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64)returnAddress, &displacement, &lineInfo))
    {
        _snprintf(s_dbgResolvedCallsiteFile, sizeof(s_dbgResolvedCallsiteFile) - 1, "%s", lineInfo.FileName);
        s_dbgResolvedCallsiteFile[sizeof(s_dbgResolvedCallsiteFile) - 1] = '\0';
        if (file != nullptr)
        {
            *file = s_dbgResolvedCallsiteFile;
        }
        if (line != nullptr)
        {
            *line = (sdword)lineInfo.LineNumber;
        }
    }
}

void dbgSetActiveLogContext(
    const char *api,
    const char *file,
    sdword line,
    const char *format,
    const char *rendered)
{
    s_dbgActiveLogContext.api = api;
    s_dbgActiveLogContext.file = file;
    s_dbgActiveLogContext.line = line;
    dbgCopyText(s_dbgActiveLogContext.format, format);
    dbgCopyText(s_dbgActiveLogContext.rendered, rendered);
}

void dbgClearActiveLogContext(void)
{
    s_dbgActiveLogContext.api = nullptr;
    s_dbgActiveLogContext.file = nullptr;
    s_dbgActiveLogContext.line = 0;
    s_dbgActiveLogContext.format[0] = '\0';
    s_dbgActiveLogContext.rendered[0] = '\0';
}

void dbgAppendLineToFile(const char *path, const char *text)
{
    FILE *file;

    if (path == nullptr || text == nullptr || text[0] == '\0')
    {
        return;
    }

    file = fopen(path, "ab");
    if (file == nullptr)
    {
        return;
    }

    fwrite(text, 1, strlen(text), file);
    fwrite("\r\n", 1, 2, file);
    fclose(file);
}

void dbgWriteFailureLine(const char *text)
{
    if (text == nullptr || text[0] == '\0')
    {
        return;
    }

    OutputDebugStringA(text);
    OutputDebugStringA("\r\n");

    fprintf(stderr, "%s\n", text);
    fflush(stderr);

    dbgAppendLineToFile(kLogFileName, text);
}

[[noreturn]] void dbgAbortForLoggingFailure(const char *reason)
{
    char buffer[4096];
    const char *api = s_dbgActiveLogContext.api ? s_dbgActiveLogContext.api : "unknown";
    const char *file = s_dbgActiveLogContext.file;
    const char *format = s_dbgActiveLogContext.format[0] != '\0' ? s_dbgActiveLogContext.format : "<none>";
    const char *rendered = s_dbgActiveLogContext.rendered[0] != '\0' ? s_dbgActiveLogContext.rendered : "<none>";

    if (s_dbgHandlingLoggingFailure)
    {
        dbgWriteFailureLine("Logging failure while reporting a previous logging failure.");
        abort();
    }

    s_dbgHandlingLoggingFailure = true;

    if (file != nullptr)
    {
        _snprintf(
            buffer,
            sizeof(buffer) - 1,
            "LOGGING FAILURE in %s at %s(%d): %s | format=\"%s\" | rendered=\"%s\"",
            api,
            file,
            s_dbgActiveLogContext.line,
            reason != nullptr ? reason : "<no detail>",
            format,
            rendered);
    }
    else
    {
        _snprintf(
            buffer,
            sizeof(buffer) - 1,
            "LOGGING FAILURE in %s: %s | format=\"%s\" | rendered=\"%s\"",
            api,
            reason != nullptr ? reason : "<no detail>",
            format,
            rendered);
    }
    buffer[sizeof(buffer) - 1] = '\0';

    dbgWriteFailureLine(buffer);

    if (dbgInt3Enabled && dbgAllowInterrupts)
    {
        __debugbreak();
    }

    abort();
}
}


/*=============================================================================
    Internal helpers
=============================================================================*/

static spdlog::logger *dbgGetLogger(void)
{
    auto *raw = spdlog::default_logger_raw();
    return raw;
}

/*=============================================================================
    Public C API
=============================================================================*/

extern "C" void dbgLogInit(void)
{
    if (s_dbgLogInitialized)
        return;

    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("%v");
    consoleSink->set_level(spdlog::level::trace);

    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        kLogFileName, /*truncate=*/false);
    fileSink->set_pattern("%v");
    fileSink->set_level(spdlog::level::trace);

    auto msvcSink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    msvcSink->set_pattern("%v");
    msvcSink->set_level(spdlog::level::trace);

    auto logger = std::make_shared<spdlog::logger>(
        "cataclysm",
        spdlog::sinks_init_list{consoleSink, fileSink, msvcSink});

    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::trace);
    logger->set_pattern("%v");
    logger->set_error_handler([](const std::string &msg)
    {
        dbgAbortForLoggingFailure(msg.c_str());
    });

    spdlog::set_default_logger(logger);
    s_dbgLogInitialized = true;
    SPDLOG_INFO(
        "Build context: branch={} worktree={}",
        CATACLYSM_BUILD_BRANCH,
        CATACLYSM_BUILD_WORKTREE);
    SPDLOG_INFO("Logging initialized (file: {})", kLogFileName);
}

extern "C" void dbgLogShutdown(void)
{
    if (!s_dbgLogInitialized)
        return;

    spdlog::default_logger()->flush();
    spdlog::shutdown();
    s_dbgLogInitialized = false;
}

// Used by CrashLog.cpp to mirror crash-context writes into the debug log.
// Must be async-signal-safe enough to call from the crash handler:
// spdlog's synchronous file sink uses a plain mutex which is acceptable here.
extern "C" void dbgLogWriteRaw(const char *text)
{
    if (!s_dbgLogInitialized || text == nullptr || text[0] == '\0')
        return;

    // Strip any leading newline from the message for cleaner log output
    const char *msg = text;
    while (*msg == '\n' || *msg == '\r')
        ++msg;

    if (msg[0] != '\0')
    {
        try
        {
            SPDLOG_INFO("{}", msg);
        }
        catch (const std::exception &e)
        {
            dbgSetActiveLogContext("dbgLogWriteRaw", nullptr, 0, text, msg);
            dbgAbortForLoggingFailure(e.what());
        }
        catch (...)
        {
            dbgSetActiveLogContext("dbgLogWriteRaw", nullptr, 0, text, msg);
            dbgAbortForLoggingFailure("unknown C++ exception");
        }
    }
}

/*=============================================================================
    dbg* API
=============================================================================*/

extern "C" void dbgMessage(char *string)
{
    const char *file = nullptr;
    sdword line = 0;
    dbgResolveCallerLocation(_ReturnAddress(), &file, &line);

    if (!s_dbgLogInitialized)
    {
        printf("%s\n", string);
        return;
    }

    dbgSetActiveLogContext("dbgMessage", file, line, string, string);
    try
    {
        SPDLOG_INFO("{}", string);
        dbgClearActiveLogContext();
    }
    catch (const std::exception &e)
    {
        dbgAbortForLoggingFailure(e.what());
    }
    catch (...)
    {
        dbgAbortForLoggingFailure("unknown C++ exception");
    }
}

extern "C" void dbgMessagef(char *format, ...)
{
    char buf[DBG_BufferLength];
    const char *file = nullptr;
    sdword line = 0;
    va_list args;
    dbgResolveCallerLocation(_ReturnAddress(), &file, &line);
    va_start(args, format);
    _vsnprintf(buf, DBG_BufferMax, format, args);
    buf[DBG_BufferMax] = '\0';
    va_end(args);

    // Strip leading newlines that HW1 code prepends to messages
    const char *msg = buf;
    while (*msg == '\n' || *msg == '\r')
        ++msg;

    if (!s_dbgLogInitialized)
    {
        printf("%s\n", msg);
        return;
    }

    dbgSetActiveLogContext("dbgMessagef", file, line, format, msg);
    try
    {
        SPDLOG_INFO("{}", msg);
        dbgClearActiveLogContext();
    }
    catch (const std::exception &e)
    {
        dbgAbortForLoggingFailure(e.what());
    }
    catch (...)
    {
        dbgAbortForLoggingFailure("unknown C++ exception");
    }
}

extern "C" void dbgWarning(char *file, sdword line, char *string)
{
    if (!s_dbgLogInitialized)
    {
        printf("Warning: %s\n", string);
        return;
    }

    dbgSetActiveLogContext("dbgWarning", file, line, string, string);
    try
    {
        if (file != nullptr)
            SPDLOG_WARN("{}({}): {}", file, line, string);
        else
            SPDLOG_WARN("{}", string);
        dbgClearActiveLogContext();
    }
    catch (const std::exception &e)
    {
        dbgAbortForLoggingFailure(e.what());
    }
    catch (...)
    {
        dbgAbortForLoggingFailure("unknown C++ exception");
    }
}

extern "C" void dbgWarningf(char *file, sdword line, char *format, ...)
{
    char buf[DBG_BufferLength];
    va_list args;
    va_start(args, format);
    _vsnprintf(buf, DBG_BufferMax, format, args);
    buf[DBG_BufferMax] = '\0';
    va_end(args);

    if (!s_dbgLogInitialized)
    {
        printf("Warning: %s\n", buf);
        return;
    }

    dbgSetActiveLogContext("dbgWarningf", file, line, format, buf);
    try
    {
        if (file != nullptr)
            SPDLOG_WARN("{}({}): {}", file, line, buf);
        else
            SPDLOG_WARN("{}", buf);
        dbgClearActiveLogContext();
    }
    catch (const std::exception &e)
    {
        dbgAbortForLoggingFailure(e.what());
    }
    catch (...)
    {
        dbgAbortForLoggingFailure("unknown C++ exception");
    }
}

extern "C" void dbgFatal(char *file, sdword line, char *string)
{
    char buf[DBG_BufferLength];
    if (file != nullptr)
        _snprintf(buf, DBG_BufferMax, "%s(%d): FATAL - %s", file, line, string);
    else
        _snprintf(buf, DBG_BufferMax, "FATAL - %s", string);
    buf[DBG_BufferMax] = '\0';

    if (s_dbgLogInitialized)
    {
        dbgSetActiveLogContext("dbgFatal", file, line, string, buf);
        try
        {
            SPDLOG_CRITICAL("{}", buf);
            dbgClearActiveLogContext();
        }
        catch (const std::exception &e)
        {
            dbgAbortForLoggingFailure(e.what());
        }
        catch (...)
        {
            dbgAbortForLoggingFailure("unknown C++ exception");
        }
    }
    else
        fprintf(stderr, "%s\n", buf);

    crashLogWriteCurrentContext("dbgFatal", buf);
    exit(1);
}

extern "C" void dbgFatalf(char *file, sdword line, char *format, ...)
{
    char msg[DBG_BufferLength];
    char buf[DBG_BufferLength];
    va_list args;
    va_start(args, format);
    _vsnprintf(msg, DBG_BufferMax, format, args);
    msg[DBG_BufferMax] = '\0';
    va_end(args);

    if (file != nullptr)
        _snprintf(buf, DBG_BufferMax, "%s(%d): FATAL - %s", file, line, msg);
    else
        _snprintf(buf, DBG_BufferMax, "FATAL - %s", msg);
    buf[DBG_BufferMax] = '\0';

    if (s_dbgLogInitialized)
    {
        dbgSetActiveLogContext("dbgFatalf", file, line, format, msg);
        try
        {
            SPDLOG_CRITICAL("{}", buf);
            dbgClearActiveLogContext();
        }
        catch (const std::exception &e)
        {
            dbgAbortForLoggingFailure(e.what());
        }
        catch (...)
        {
            dbgAbortForLoggingFailure("unknown C++ exception");
        }
    }
    else
        fprintf(stderr, "%s\n", buf);

#ifdef _DEBUG
    __debugbreak();
#endif

    crashLogWriteCurrentContext("dbgFatalf", buf);
    exit(1);
}

extern "C" void dbgNonFatal(char *file, sdword line, char *string)
{
    if (!s_dbgLogInitialized)
    {
        fprintf(stderr, "NonFatal: %s\n", string);
        return;
    }

    dbgSetActiveLogContext("dbgNonFatal", file, line, string, string);
    try
    {
        if (file != nullptr)
            SPDLOG_ERROR("{}({}): NonFatal - {}", file, line, string);
        else
            SPDLOG_ERROR("NonFatal - {}", string);
        dbgClearActiveLogContext();
    }
    catch (const std::exception &e)
    {
        dbgAbortForLoggingFailure(e.what());
    }
    catch (...)
    {
        dbgAbortForLoggingFailure("unknown C++ exception");
    }
}

extern "C" void dbgNonFatalf(char *file, sdword line, char *format, ...)
{
    char buf[DBG_BufferLength];
    va_list args;
    va_start(args, format);
    _vsnprintf(buf, DBG_BufferMax, format, args);
    buf[DBG_BufferMax] = '\0';
    va_end(args);

    if (!s_dbgLogInitialized)
    {
        fprintf(stderr, "NonFatal: %s\n", buf);
        return;
    }

    dbgSetActiveLogContext("dbgNonFatalf", file, line, format, buf);
    try
    {
        if (file != nullptr)
            SPDLOG_ERROR("{}({}): NonFatal - {}", file, line, buf);
        else
            SPDLOG_ERROR("NonFatal - {}", buf);
        dbgClearActiveLogContext();
    }
    catch (const std::exception &e)
    {
        dbgAbortForLoggingFailure(e.what());
    }
    catch (...)
    {
        dbgAbortForLoggingFailure("unknown C++ exception");
    }
}

extern "C" char *dbgStackDump(void)
{
    return nullptr;
}
