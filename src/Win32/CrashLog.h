#ifndef ___CRASHLOG_H
#define ___CRASHLOG_H

#ifdef __cplusplus
extern "C" {
#endif

void crashLogInit(void);
void crashLogShutdown(void);
void crashLogWrite(const char *text);
void crashLogWriteCurrentContext(const char *reason, const char *detail);
void crashLogWriteException(const char *reason, void *exceptionPointers);

#ifdef __cplusplus
}
#endif

#endif
