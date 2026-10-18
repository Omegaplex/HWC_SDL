/*=============================================================================
    CrashLog.cpp: Crash-safe logging for fatal events and AddressSanitizer reports.
=============================================================================*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <eh.h>
#include <signal.h>

#pragma comment(lib, "Dbghelp.lib")

#ifdef swap
#undef swap
#endif

#include <array>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <io.h>

#include "BuildContext.generated.h"
#include "CrashLog.h"
#include "debug.h"

#ifndef CATACLYSM_BUILD_BRANCH
#define CATACLYSM_BUILD_BRANCH "unknown"
#endif

#ifndef CATACLYSM_BUILD_WORKTREE
#define CATACLYSM_BUILD_WORKTREE "unknown"
#endif

#ifndef CATACLYSM_BUILD_LOG_FILE
#define CATACLYSM_BUILD_LOG_FILE "cataclysm.log"
#endif

namespace
{
constexpr char kCrashLogFileName[] = CATACLYSM_BUILD_LOG_FILE;
constexpr size_t kCrashLogBufferLength = 4096;
constexpr unsigned int kCrashLogStackFrames = 32;
constexpr size_t kCrashDumpPathLength = 1024;
constexpr char kCrashDumpSubdirectory[] = "Homeworld Cataclysm\\CrashDumps\\";

HANDLE s_crashLogFile = INVALID_HANDLE_VALUE;
bool32 s_crashLogInitialized = FALSE;
bool32 s_crashLogShuttingDown = FALSE;
bool32 s_crashLogStdErrRedirected = FALSE;
bool s_symbolsInitialized = false;
bool32 s_crashDumpDirectoryReady = FALSE;
HANDLE s_prevStdErrorHandle = INVALID_HANDLE_VALUE;
HANDLE s_redirectedStdErrorHandle = INVALID_HANDLE_VALUE;
PVOID s_vectoredExceptionHandler = nullptr;
LPTOP_LEVEL_EXCEPTION_FILTER s_prevUnhandledExceptionFilter = nullptr;
std::terminate_handler s_prevTerminateHandler = nullptr;
_purecall_handler s_prevPurecallHandler = nullptr;
_invalid_parameter_handler s_prevInvalidParameterHandler = nullptr;
void(__cdecl *s_prevAbortHandler)(int) = SIG_DFL;
void(__cdecl *s_prevFpeHandler)(int) = SIG_DFL;
void(__cdecl *s_prevIllHandler)(int) = SIG_DFL;
void(__cdecl *s_prevSegvHandler)(int) = SIG_DFL;

// Exception context cached by the VEH for use by the signal handler, which fires
// after ASan has already consumed the original exception pointers.
volatile LONG s_pendingSignalExceptionFlag = 0;
DWORD s_pendingSignalThreadId = 0;
EXCEPTION_RECORD s_pendingSignalExceptionRecord = {};
CONTEXT s_pendingSignalContext = {};
char s_crashDumpDirectory[kCrashDumpPathLength] = {};

#ifdef __SANITIZE_ADDRESS__
extern "C" void __asan_set_error_report_callback(void(__cdecl *callback)(const char *));

extern "C" const char *__asan_default_options()
{
    return "quarantine_size_mb=32";
}
#endif

void crashLogOpenFile(void);
void crashLogWriteBytes(const char *text, DWORD length);
void crashLogWriteFormat(const char *format, ...);
void crashLogWriteSectionHeader(const char *title);
void crashLogWriteRegisters(const CONTEXT *context);
void crashLogInitSymbols(void);
void crashLogShutdownSymbols(void);
void crashLogWriteStackFrame(unsigned int frameIndex, DWORD64 address);
void crashLogWriteStackTrace(const CONTEXT *context, udword framesToSkip);
void crashLogWriteExceptionContext(const char *reason, const char *detail, const EXCEPTION_POINTERS *exceptionPointers);
bool crashLogEnsureDirectoryTree(char *path);
bool crashLogPrepareDumpDirectory(void);
bool crashLogBuildDumpPath(char *dumpPath, size_t dumpPathLength);
void crashLogWriteMinidump(const char *reason, const EXCEPTION_POINTERS *exceptionPointers);
void crashLogRedirectStandardError(void);
const char *crashLogSignalName(int signalNumber);
bool crashLogIsSignalConvertibleException(DWORD exceptionCode);
void crashLogCacheSignalException(const EXCEPTION_POINTERS *exceptionPointers);
const EXCEPTION_POINTERS *crashLogGetPendingSignalException(void);
void crashLogClearPendingSignalException(void);
LONG CALLBACK crashLogVectoredExceptionHandler(EXCEPTION_POINTERS *exceptionPointers);
LONG CALLBACK crashLogUnhandledExceptionFilter(EXCEPTION_POINTERS *exceptionPointers);
void __cdecl crashLogTerminateHandler(void);
void __cdecl crashLogPurecallHandler(void);
void __cdecl crashLogInvalidParameterHandler(
    const wchar_t *expression,
    const wchar_t *functionName,
    const wchar_t *fileName,
    unsigned int lineNumber,
    uintptr_t reserved);
void __cdecl crashLogSignalHandler(int signalNumber);

#ifdef __SANITIZE_ADDRESS__
void __cdecl crashLogAsanReportCallback(const char *report);
#endif

void crashLogOpenFile(void)
{
    if (s_crashLogFile != INVALID_HANDLE_VALUE)
    {
        return;
    }

    s_crashLogFile = CreateFileA(
        kCrashLogFileName,
        FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
}

void crashLogWriteBytes(const char *text, DWORD length)
{
    DWORD written;
    HANDLE stdError;

    if (text == nullptr || length == 0)
    {
        return;
    }

    OutputDebugStringA(text);

    // Write to file only, or fall back to stderr.  Never write to both —
    // we redirect stderr to the crash log file, so writing to both would
    // produce duplicated output.
    if (s_crashLogFile != INVALID_HANDLE_VALUE)
    {
        WriteFile(s_crashLogFile, text, length, &written, nullptr);
        FlushFileBuffers(s_crashLogFile);
        return;
    }

    stdError = GetStdHandle(STD_ERROR_HANDLE);
    if (stdError != nullptr && stdError != INVALID_HANDLE_VALUE)
    {
        WriteFile(stdError, text, length, &written, nullptr);
    }
}

void crashLogWriteFormat(const char *format, ...)
{
    char buffer[kCrashLogBufferLength];
    va_list argList;

    va_start(argList, format);
    _vsnprintf(buffer, sizeof(buffer) - 1, format, argList);
    va_end(argList);
    buffer[sizeof(buffer) - 1] = '\0';
    crashLogWrite(buffer);
}

void crashLogWriteSectionHeader(const char *title)
{
    SYSTEMTIME localTime;

    GetLocalTime(&localTime);
    crashLogWriteFormat(
        "\n========== %s [%04u-%02u-%02u %02u:%02u:%02u.%03u] ==========\n",
        title,
        localTime.wYear,
        localTime.wMonth,
        localTime.wDay,
        localTime.wHour,
        localTime.wMinute,
        localTime.wSecond,
        localTime.wMilliseconds);
}

void crashLogRedirectStandardError(void)
{
    HANDLE stdErrorHandle;
    HANDLE crtErrorHandle;
    int errorFd;
    int stderrFd;

    if (s_crashLogFile == INVALID_HANDLE_VALUE || s_crashLogStdErrRedirected)
    {
        return;
    }

    s_prevStdErrorHandle = GetStdHandle(STD_ERROR_HANDLE);

    stdErrorHandle = INVALID_HANDLE_VALUE;
    if (DuplicateHandle(GetCurrentProcess(),
                        s_crashLogFile,
                        GetCurrentProcess(),
                        &stdErrorHandle,
                        0,
                        TRUE,
                        DUPLICATE_SAME_ACCESS))
    {
        SetStdHandle(STD_ERROR_HANDLE, stdErrorHandle);
        s_redirectedStdErrorHandle = stdErrorHandle;
    }

    crtErrorHandle = INVALID_HANDLE_VALUE;
    if (!DuplicateHandle(GetCurrentProcess(),
                         s_crashLogFile,
                         GetCurrentProcess(),
                         &crtErrorHandle,
                         0,
                         TRUE,
                         DUPLICATE_SAME_ACCESS))
    {
        return;
    }

    errorFd = _open_osfhandle((intptr_t)crtErrorHandle, _O_APPEND | _O_TEXT);
    if (errorFd == -1)
    {
        CloseHandle(crtErrorHandle);
        return;
    }

    stderrFd = _fileno(stderr);
    if (stderrFd != -1 && _dup2(errorFd, stderrFd) == 0)
    {
        setvbuf(stderr, nullptr, _IONBF, 0);
        s_crashLogStdErrRedirected = TRUE;
    }

    _close(errorFd);
}

bool crashLogEnsureDirectoryTree(char *path)
{
    char temp[kCrashDumpPathLength];
    char *cursor;

    if (path == nullptr || path[0] == '\0')
    {
        return false;
    }

    strncpy(temp, path, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    for (cursor = temp + 1; *cursor != '\0'; cursor++)
    {
        if (*cursor == '\\' || *cursor == '/')
        {
            DWORD error;
            char saved = *cursor;

            *cursor = '\0';
            if (!(temp[1] == ':' && temp[2] == '\0'))
            {
                if (!CreateDirectoryA(temp, nullptr))
                {
                    error = GetLastError();
                    if (error != ERROR_ALREADY_EXISTS && error != ERROR_FILE_EXISTS)
                    {
                        crashLogWriteFormat("MiniDump directory create failed: %s (error %lu)\n", temp, error);
                        return false;
                    }
                }
            }
            *cursor = saved;
        }
    }

    if (temp[0] != '\0' && temp[strlen(temp) - 1] == '\\')
    {
        temp[strlen(temp) - 1] = '\0';
    }

    if (temp[0] != '\0' && !(temp[1] == ':' && temp[2] == '\0'))
    {
        DWORD error;

        if (!CreateDirectoryA(temp, nullptr))
        {
            error = GetLastError();
            if (error != ERROR_ALREADY_EXISTS && error != ERROR_FILE_EXISTS)
            {
                crashLogWriteFormat("MiniDump directory create failed: %s (error %lu)\n", temp, error);
                return false;
            }
        }
    }

    return true;
}

bool crashLogPrepareDumpDirectory(void)
{
    DWORD length;
    size_t baseLength;

    if (s_crashDumpDirectoryReady)
    {
        return true;
    }

    length = GetEnvironmentVariableA("LOCALAPPDATA", s_crashDumpDirectory, sizeof(s_crashDumpDirectory));
    if (length == 0 || length >= sizeof(s_crashDumpDirectory))
    {
        crashLogWriteFormat("MiniDump directory unavailable: LOCALAPPDATA missing or too long (error %lu)\n", GetLastError());
        return false;
    }

    baseLength = strlen(s_crashDumpDirectory);
    if (baseLength == 0)
    {
        crashLogWrite("MiniDump directory unavailable: LOCALAPPDATA resolved to an empty path\n");
        return false;
    }

    if (s_crashDumpDirectory[baseLength - 1] != '\\')
    {
        if (baseLength + 1 >= sizeof(s_crashDumpDirectory))
        {
            crashLogWrite("MiniDump directory unavailable: LOCALAPPDATA path is too long\n");
            s_crashDumpDirectory[0] = '\0';
            return false;
        }

        s_crashDumpDirectory[baseLength] = '\\';
        s_crashDumpDirectory[baseLength + 1] = '\0';
        baseLength++;
    }

    if (baseLength + strlen(kCrashDumpSubdirectory) >= sizeof(s_crashDumpDirectory))
    {
        crashLogWrite("MiniDump directory unavailable: dump path is too long\n");
        s_crashDumpDirectory[0] = '\0';
        return false;
    }

    strcat(s_crashDumpDirectory, kCrashDumpSubdirectory);
    if (!crashLogEnsureDirectoryTree(s_crashDumpDirectory))
    {
        s_crashDumpDirectory[0] = '\0';
        return false;
    }

    s_crashDumpDirectoryReady = TRUE;
    return true;
}

bool crashLogBuildDumpPath(char *dumpPath, size_t dumpPathLength)
{
    SYSTEMTIME localTime;
    int written;

    if (dumpPath == nullptr || dumpPathLength == 0)
    {
        return false;
    }

    if (!crashLogPrepareDumpDirectory())
    {
        return false;
    }

    GetLocalTime(&localTime);
    written = _snprintf(
        dumpPath,
        dumpPathLength - 1,
        "%scataclysm_%04u%02u%02u_%02u%02u%02u_%03u_pid%lu_tid%lu.dmp",
        s_crashDumpDirectory,
        localTime.wYear,
        localTime.wMonth,
        localTime.wDay,
        localTime.wHour,
        localTime.wMinute,
        localTime.wSecond,
        localTime.wMilliseconds,
        GetCurrentProcessId(),
        GetCurrentThreadId());
    dumpPath[dumpPathLength - 1] = '\0';

    return written > 0 && static_cast<size_t>(written) < dumpPathLength;
}

void crashLogWriteMinidump(const char *reason, const EXCEPTION_POINTERS *exceptionPointers)
{
    char dumpPath[kCrashDumpPathLength];
    HANDLE dumpFile;
    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
    MINIDUMP_TYPE dumpType;
    BOOL wroteDump;
    DWORD error;

    if (exceptionPointers == nullptr ||
        exceptionPointers->ExceptionRecord == nullptr ||
        exceptionPointers->ContextRecord == nullptr)
    {
        return;
    }

    if (IsDebuggerPresent())
    {
        crashLogWriteFormat("MiniDump skipped for %s: debugger is attached\n", reason);
        return;
    }

    if (!crashLogBuildDumpPath(dumpPath, sizeof(dumpPath)))
    {
        crashLogWriteFormat("MiniDump unavailable for %s: failed to build dump path\n", reason);
        return;
    }

    dumpFile = CreateFileA(
        dumpPath,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (dumpFile == INVALID_HANDLE_VALUE)
    {
        crashLogWriteFormat("MiniDump create failed for %s: %s (error %lu)\n", reason, dumpPath, GetLastError());
        return;
    }

    exceptionInfo.ThreadId = GetCurrentThreadId();
    exceptionInfo.ExceptionPointers = const_cast<EXCEPTION_POINTERS *>(exceptionPointers);
    exceptionInfo.ClientPointers = FALSE;

    dumpType = static_cast<MINIDUMP_TYPE>(
        MiniDumpNormal |
        MiniDumpWithDataSegs |
        MiniDumpWithHandleData |
        MiniDumpWithIndirectlyReferencedMemory |
        MiniDumpScanMemory);
#ifdef MiniDumpWithThreadInfo
    dumpType = static_cast<MINIDUMP_TYPE>(dumpType | MiniDumpWithThreadInfo);
#endif

    wroteDump = MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        dumpFile,
        dumpType,
        &exceptionInfo,
        nullptr,
        nullptr);
    error = wroteDump ? ERROR_SUCCESS : GetLastError();

    CloseHandle(dumpFile);

    if (!wroteDump)
    {
        DeleteFileA(dumpPath);
        crashLogWriteFormat("MiniDump write failed for %s (error %lu)\n", reason, error);
        return;
    }

    crashLogWriteFormat("MiniDump=%s\n", dumpPath);
}

void crashLogWriteRegisters(const CONTEXT *context)
{
    if (context == nullptr)
    {
        return;
    }

#if defined(_M_IX86)
    crashLogWrite("Registers:\n");
    crashLogWriteFormat("  EAX=%08lX EBX=%08lX ECX=%08lX EDX=%08lX\n",
                        context->Eax, context->Ebx, context->Ecx, context->Edx);
    crashLogWriteFormat("  ESI=%08lX EDI=%08lX EBP=%08lX ESP=%08lX\n",
                        context->Esi, context->Edi, context->Ebp, context->Esp);
    crashLogWriteFormat("  EIP=%08lX EFLAGS=%08lX\n",
                        context->Eip, context->EFlags);
#endif
}

void crashLogInitSymbols(void)
{
    if (s_symbolsInitialized)
    {
        return;
    }

    SymSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    if (!SymInitialize(GetCurrentProcess(), nullptr, TRUE))
    {
        // DbgHelp may already be initialized by ASan's runtime. If its functions still
        // work, proceed — symbols will be available even without our SymInitialize call.
        SYMBOL_INFO sym = {};
        sym.SizeOfStruct = sizeof(SYMBOL_INFO);
        sym.MaxNameLen = 0;
        DWORD64 disp = 0;
        if (SymFromAddr(GetCurrentProcess(), (DWORD64)(uintptr_t)&crashLogInitSymbols, &disp, &sym))
        {
            s_symbolsInitialized = true;
            return;
        }

        DWORD dwError = GetLastError();
        crashLogWriteFormat("SymInitialize failed (GetLastError=%lu) - stack trace unavailable\n", dwError);
        return;
    }
    s_symbolsInitialized = true;
}

void crashLogShutdownSymbols(void)
{
    if (!s_symbolsInitialized)
    {
        return;
    }

    SymCleanup(GetCurrentProcess());
    s_symbolsInitialized = false;
}

void crashLogWriteStackFrame(unsigned int frameIndex, DWORD64 address)
{
    const HANDLE hProcess = GetCurrentProcess();
    const void *pAddress = reinterpret_cast<void *>(static_cast<uintptr_t>(address));

    char symbolBuffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {};
    SYMBOL_INFO *pSymbol = reinterpret_cast<SYMBOL_INFO *>(symbolBuffer);
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    DWORD64 symbolDisplacement = 0;
    const BOOL haveSymbol = s_symbolsInitialized &&
                            SymFromAddr(hProcess, address, &symbolDisplacement, pSymbol);

    IMAGEHLP_LINE64 lineInfo = {};
    lineInfo.SizeOfStruct = sizeof(lineInfo);
    DWORD lineDisplacement = 0;
    const BOOL haveLine = s_symbolsInitialized &&
                          SymGetLineFromAddr64(hProcess, address, &lineDisplacement, &lineInfo);

    IMAGEHLP_MODULE64 moduleInfo = {};
    moduleInfo.SizeOfStruct = sizeof(moduleInfo);
    const BOOL haveModule = s_symbolsInitialized &&
                            SymGetModuleInfo64(hProcess, address, &moduleInfo);

    if (haveSymbol)
    {
        crashLogWriteFormat("  #%02u %p %s + 0x%llX\n",
                            frameIndex, pAddress,
                            pSymbol->Name,
                            static_cast<unsigned long long>(symbolDisplacement));
    }
    else
    {
        crashLogWriteFormat("  #%02u %p\n", frameIndex, pAddress);
    }

    if (haveLine)
    {
        crashLogWriteFormat("       at %s:%lu\n", lineInfo.FileName, lineInfo.LineNumber);
    }

    if (haveModule)
    {
        crashLogWriteFormat("       module %s\n", moduleInfo.ModuleName);
    }
}

void crashLogWriteStackTrace(const CONTEXT *context, udword framesToSkip)
{
    if (context == nullptr)
    {
        return;
    }

    crashLogInitSymbols();

    CONTEXT stackContext = *context;
    STACKFRAME64 stackFrame = {};
    DWORD machineType = 0;

#if defined(_M_IX86)
    machineType = IMAGE_FILE_MACHINE_I386;
    stackFrame.AddrPC.Offset = stackContext.Eip;
    stackFrame.AddrFrame.Offset = stackContext.Ebp;
    stackFrame.AddrStack.Offset = stackContext.Esp;
#else
#error Unsupported platform for stack walking.
#endif

    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Mode = AddrModeFlat;

    crashLogWrite("Stack trace:\n");

    unsigned int printedFrames = 0;
    for (unsigned int iFrame = 0; iFrame < kCrashLogStackFrames; ++iFrame)
    {
        const DWORD64 address = stackFrame.AddrPC.Offset;
        if (address == 0)
        {
            break;
        }

        if (iFrame >= framesToSkip)
        {
            crashLogWriteStackFrame(printedFrames, address);
            ++printedFrames;
        }

        if (!StackWalk64(
                machineType,
                GetCurrentProcess(),
                GetCurrentThread(),
                &stackFrame,
                &stackContext,
                nullptr,
                SymFunctionTableAccess64,
                SymGetModuleBase64,
                nullptr))
        {
            if (iFrame == 0)
            {
                crashLogWriteFormat("  StackWalk64 failed (GetLastError=%lu)\n", GetLastError());
            }
            break;
        }
    }
}

void crashLogWriteExceptionContext(const char *reason, const char *detail, const EXCEPTION_POINTERS *exceptionPointers)
{
    const EXCEPTION_RECORD *record;

    crashLogWriteSectionHeader(reason);
    crashLogWriteFormat("Process=%lu Thread=%lu\n", GetCurrentProcessId(), GetCurrentThreadId());
    crashLogWriteFormat(
        "BuildContext=branch=%s worktree=%s\n",
        CATACLYSM_BUILD_BRANCH,
        CATACLYSM_BUILD_WORKTREE);
    crashLogWriteFormat("CommandLine=%s\n", GetCommandLineA());

    if (detail != nullptr && detail[0] != '\0')
    {
        crashLogWriteFormat("%s\n", detail);
    }

    if (exceptionPointers == nullptr || exceptionPointers->ExceptionRecord == nullptr)
    {
        crashLogWrite("No exception record.\n");
    }
    else
    {
        record = exceptionPointers->ExceptionRecord;
        crashLogWriteFormat("ExceptionCode=0x%08lX\n", record->ExceptionCode);
        crashLogWriteFormat("ExceptionFlags=0x%08lX\n", record->ExceptionFlags);
        crashLogWriteFormat("ExceptionAddress=%p\n", record->ExceptionAddress);

        if ((record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ||
             record->ExceptionCode == EXCEPTION_IN_PAGE_ERROR) &&
            record->NumberParameters >= 2)
        {
            crashLogWriteFormat("AccessType=%Iu FaultAddress=%p\n",
                                record->ExceptionInformation[0],
                                reinterpret_cast<void *>(record->ExceptionInformation[1]));
        }
    }

    if (exceptionPointers != nullptr && exceptionPointers->ContextRecord != nullptr)
    {
        crashLogWriteRegisters(exceptionPointers->ContextRecord);
        crashLogWriteStackTrace(exceptionPointers->ContextRecord, 0);
    }
    crashLogWriteMinidump(reason, exceptionPointers);
    crashLogWrite("========== end fatal event ==========\n");
}

const char *crashLogSignalName(int signalNumber)
{
    switch (signalNumber)
    {
        case SIGABRT: return "SIGABRT";
        case SIGFPE:  return "SIGFPE";
        case SIGILL:  return "SIGILL";
        case SIGSEGV: return "SIGSEGV";
        default:      return "SIGNAL";
    }
}

bool crashLogIsSignalConvertibleException(DWORD exceptionCode)
{
    switch (exceptionCode)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_IN_PAGE_ERROR:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_PRIV_INSTRUCTION:
    case EXCEPTION_STACK_OVERFLOW:
        return true;
    default:
        return false;
    }
}

void crashLogCacheSignalException(const EXCEPTION_POINTERS *exceptionPointers)
{
    if (exceptionPointers == nullptr ||
        exceptionPointers->ExceptionRecord == nullptr ||
        exceptionPointers->ContextRecord == nullptr)
    {
        return;
    }

    if (!crashLogIsSignalConvertibleException(exceptionPointers->ExceptionRecord->ExceptionCode))
    {
        return;
    }

    s_pendingSignalExceptionRecord = *exceptionPointers->ExceptionRecord;
    s_pendingSignalContext = *exceptionPointers->ContextRecord;
    s_pendingSignalThreadId = GetCurrentThreadId();
    InterlockedExchange(&s_pendingSignalExceptionFlag, 1);
}

const EXCEPTION_POINTERS *crashLogGetPendingSignalException(void)
{
    if (InterlockedCompareExchange(&s_pendingSignalExceptionFlag, 0, 0) == 0)
    {
        return nullptr;
    }

    if (s_pendingSignalThreadId != GetCurrentThreadId())
    {
        return nullptr;
    }

    static EXCEPTION_POINTERS s_pendingExceptionPointers = {};
    s_pendingExceptionPointers.ExceptionRecord = &s_pendingSignalExceptionRecord;
    s_pendingExceptionPointers.ContextRecord = &s_pendingSignalContext;
    return &s_pendingExceptionPointers;
}

void crashLogClearPendingSignalException(void)
{
    s_pendingSignalThreadId = 0;
    InterlockedExchange(&s_pendingSignalExceptionFlag, 0);
}

LONG CALLBACK crashLogVectoredExceptionHandler(EXCEPTION_POINTERS *exceptionPointers)
{
    crashLogCacheSignalException(exceptionPointers);
    return EXCEPTION_CONTINUE_SEARCH;
}

LONG CALLBACK crashLogUnhandledExceptionFilter(EXCEPTION_POINTERS *exceptionPointers)
{
    crashLogWriteExceptionContext("Unhandled exception", nullptr, exceptionPointers);

    if (s_prevUnhandledExceptionFilter != nullptr &&
        s_prevUnhandledExceptionFilter != crashLogUnhandledExceptionFilter)
    {
        return s_prevUnhandledExceptionFilter(exceptionPointers);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void __cdecl crashLogTerminateHandler(void)
{
    const char *detail = "No active exception.";
    std::exception_ptr currentException = std::current_exception();

    if (currentException != nullptr)
    {
        try
        {
            std::rethrow_exception(currentException);
        }
        catch (const std::exception &exception)
        {
            detail = exception.what();
        }
        catch (...)
        {
            detail = "Unhandled non-std exception.";
        }
    }

    crashLogWriteCurrentContext("std::terminate", detail);
    abort();
}

void __cdecl crashLogSignalHandler(int signalNumber)
{
    char detail[128];
    _snprintf(detail, sizeof(detail) - 1, "CRT signal %s (%d)", crashLogSignalName(signalNumber), signalNumber);
    detail[sizeof(detail) - 1] = '\0';

    // Prefer the exception context cached by the VEH, which captured the original fault
    // address before ASan consumed it. Fall back to the current stack if nothing was cached.
    const EXCEPTION_POINTERS *cachedExceptionPointers = crashLogGetPendingSignalException();
    if (cachedExceptionPointers != nullptr)
    {
        crashLogWriteExceptionContext(crashLogSignalName(signalNumber), detail, cachedExceptionPointers);
        crashLogClearPendingSignalException();
    }
    else
    {
        crashLogWriteCurrentContext(crashLogSignalName(signalNumber), detail);
    }
}

void __cdecl crashLogPurecallHandler(void)
{
    crashLogWriteCurrentContext("Pure virtual function call", "A pure virtual function was invoked.");
}

void __cdecl crashLogInvalidParameterHandler(
    const wchar_t *expression,
    const wchar_t *functionName,
    const wchar_t *fileName,
    unsigned int lineNumber,
    uintptr_t reserved)
{
    char detail[kCrashLogBufferLength];
    char expressionBuffer[256];
    char functionBuffer[256];
    char fileBuffer[256];

    (void)reserved;

    expressionBuffer[0] = '\0';
    functionBuffer[0] = '\0';
    fileBuffer[0] = '\0';

    if (expression != nullptr)
    {
        WideCharToMultiByte(CP_ACP, 0, expression, -1, expressionBuffer, sizeof(expressionBuffer), nullptr, nullptr);
    }
    if (functionName != nullptr)
    {
        WideCharToMultiByte(CP_ACP, 0, functionName, -1, functionBuffer, sizeof(functionBuffer), nullptr, nullptr);
    }
    if (fileName != nullptr)
    {
        WideCharToMultiByte(CP_ACP, 0, fileName, -1, fileBuffer, sizeof(fileBuffer), nullptr, nullptr);
    }

    _snprintf(detail, sizeof(detail) - 1,
              "Expression=%s Function=%s File=%s Line=%u",
              expressionBuffer[0] != '\0' ? expressionBuffer : "<unknown>",
              functionBuffer[0] != '\0' ? functionBuffer : "<unknown>",
              fileBuffer[0] != '\0' ? fileBuffer : "<unknown>",
              lineNumber);
    detail[sizeof(detail) - 1] = '\0';
    crashLogWriteCurrentContext("Invalid parameter", detail);
}

#ifdef __SANITIZE_ADDRESS__
void __cdecl crashLogAsanReportCallback(const char *report)
{
    CONTEXT context;
    EXCEPTION_RECORD record;
    EXCEPTION_POINTERS pointers;

    memset(&context, 0, sizeof(context));
    memset(&record, 0, sizeof(record));
    memset(&pointers, 0, sizeof(pointers));
    RtlCaptureContext(&context);
    record.ExceptionCode = STATUS_STACK_BUFFER_OVERRUN;
    record.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
#if defined(_M_IX86)
    record.ExceptionAddress = reinterpret_cast<void *>(context.Eip);
#endif
    pointers.ExceptionRecord = &record;
    pointers.ContextRecord = &context;

    crashLogWriteSectionHeader("AddressSanitizer report");
    crashLogWriteFormat("Process=%lu Thread=%lu\n", GetCurrentProcessId(), GetCurrentThreadId());
    crashLogWriteFormat(
        "BuildContext=branch=%s worktree=%s\n",
        CATACLYSM_BUILD_BRANCH,
        CATACLYSM_BUILD_WORKTREE);
    crashLogWriteFormat("CommandLine=%s\n", GetCommandLineA());
    if (report != nullptr && report[0] != '\0')
    {
        crashLogWrite(report);
        if (report[strlen(report) - 1] != '\n')
        {
            crashLogWrite("\n");
        }
    }
    else
    {
        crashLogWrite("AddressSanitizer emitted an empty report.\n");
    }
    crashLogWriteMinidump("AddressSanitizer report", &pointers);
    crashLogWrite("========== end fatal event ==========\n");
}
#endif
}

extern "C" void crashLogInit(void)
{
    if (s_crashLogInitialized)
    {
        return;
    }

    crashLogOpenFile();
    crashLogRedirectStandardError();
    // The VEH captures the original fault context before ASan or the CRT unwind their own
    // handlers, giving signal handlers the correct crash address rather than the termination
    // address. Install it at the front of the chain (first=1) so it fires before ASan.
    s_vectoredExceptionHandler = AddVectoredExceptionHandler(1, crashLogVectoredExceptionHandler);
    s_prevUnhandledExceptionFilter = SetUnhandledExceptionFilter(crashLogUnhandledExceptionFilter);
    s_prevTerminateHandler = std::set_terminate(crashLogTerminateHandler);
    s_prevPurecallHandler = _set_purecall_handler(crashLogPurecallHandler);
    s_prevInvalidParameterHandler = _set_invalid_parameter_handler(crashLogInvalidParameterHandler);
    s_prevAbortHandler = signal(SIGABRT, crashLogSignalHandler);
    s_prevFpeHandler = signal(SIGFPE, crashLogSignalHandler);
    s_prevIllHandler = signal(SIGILL, crashLogSignalHandler);
    s_prevSegvHandler = signal(SIGSEGV, crashLogSignalHandler);
#ifdef __SANITIZE_ADDRESS__
    __asan_set_error_report_callback(crashLogAsanReportCallback);
#endif
    atexit(crashLogShutdown);
    s_crashLogInitialized = TRUE;
    crashLogWriteSectionHeader("Crash logging initialized");
}

extern "C" void crashLogShutdown(void)
{
    if (!s_crashLogInitialized || s_crashLogShuttingDown)
    {
        return;
    }

    s_crashLogShuttingDown = TRUE;

    if (s_vectoredExceptionHandler != nullptr)
    {
        RemoveVectoredExceptionHandler(s_vectoredExceptionHandler);
        s_vectoredExceptionHandler = nullptr;
    }
    if (s_prevUnhandledExceptionFilter != nullptr)
    {
        SetUnhandledExceptionFilter(s_prevUnhandledExceptionFilter);
        s_prevUnhandledExceptionFilter = nullptr;
    }
    if (s_prevStdErrorHandle != INVALID_HANDLE_VALUE)
    {
        SetStdHandle(STD_ERROR_HANDLE, s_prevStdErrorHandle);
        s_prevStdErrorHandle = INVALID_HANDLE_VALUE;
    }
    if (s_redirectedStdErrorHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(s_redirectedStdErrorHandle);
        s_redirectedStdErrorHandle = INVALID_HANDLE_VALUE;
    }
    std::set_terminate(s_prevTerminateHandler);
    _set_purecall_handler(s_prevPurecallHandler);
    _set_invalid_parameter_handler(s_prevInvalidParameterHandler);
    signal(SIGABRT, s_prevAbortHandler);
    signal(SIGFPE, s_prevFpeHandler);
    signal(SIGILL, s_prevIllHandler);
    signal(SIGSEGV, s_prevSegvHandler);

    crashLogShutdownSymbols();

    if (s_crashLogFile != INVALID_HANDLE_VALUE)
    {
        FlushFileBuffers(s_crashLogFile);
        CloseHandle(s_crashLogFile);
        s_crashLogFile = INVALID_HANDLE_VALUE;
    }

    s_crashLogInitialized = FALSE;
    s_crashLogStdErrRedirected = FALSE;
}

extern "C" void crashLogWrite(const char *text)
{
    if (text == nullptr || text[0] == '\0')
    {
        return;
    }

    if (!s_crashLogInitialized)
    {
        crashLogOpenFile();
    }

    crashLogWriteBytes(text, static_cast<DWORD>(strlen(text)));
}

extern "C" void crashLogWriteCurrentContext(const char *reason, const char *detail)
{
    CONTEXT context;
    EXCEPTION_RECORD record;
    EXCEPTION_POINTERS pointers;

    memset(&context, 0, sizeof(context));
    memset(&record, 0, sizeof(record));
    memset(&pointers, 0, sizeof(pointers));

    RtlCaptureContext(&context);
    record.ExceptionCode = STATUS_FATAL_APP_EXIT;
    record.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
#if defined(_M_IX86)
    record.ExceptionAddress = reinterpret_cast<void *>(context.Eip);
#endif
    pointers.ExceptionRecord = &record;
    pointers.ContextRecord = &context;

    crashLogWriteExceptionContext(reason, detail, &pointers);
}

extern "C" void crashLogWriteException(const char *reason, void *exceptionPointers)
{
    crashLogWriteExceptionContext(reason, nullptr, static_cast<EXCEPTION_POINTERS *>(exceptionPointers));
}
