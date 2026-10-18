#ifndef QPCFIX_H
#define QPCFIX_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

BOOL catQueryPerformanceCounter(LARGE_INTEGER *performanceCount);

#endif
