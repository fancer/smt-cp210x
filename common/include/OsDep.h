/////////////////////////////////////////////////////////////////////////////
// CCriticalSectionLock
/////////////////////////////////////////////////////////////////////////////

#ifndef __OS_DEP_H__
#define __OS_DEP_H__

#if	defined(_WIN32)
#include "Types.h"
#else
#if	defined(__APPLE__)
#include <CoreServices/CoreServices.h>
#include <pthread.h>
#include "Types.h"
#elif	defined(__linux__)
#include "Types.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#else
	#error "error: Unsupported OS type"
#endif

DWORD GetTickCount();
void Sleep(DWORD msec);

#endif	// defined(_WIN32)

#include "CriticalSectionLock.h"

#endif // __OS_DEP_H__
