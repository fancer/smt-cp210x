/////////////////////////////////////////////////////////////////////////////
// CCriticalSectionLock.h
/////////////////////////////////////////////////////////////////////////////

#ifndef CRITICAL_SECTION_LOCK_H
#define CRITICAL_SECTION_LOCK_H

#if	defined(_WIN32)
#include "Types.h"
#elif	defined(__APPLE__)
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

#include "silabs_sal.h"

class CCriticalSectionLock
{
	// Constructor/Destructor
public:
	inline CCriticalSectionLock() throw()
	{
#if	defined(_WIN32)
		InitializeCriticalSection(&m_cs);
#else
		pthread_mutexattr_t attr;
		
		// Create a mutex object with recursive behavior
		//
		// This means that you can call pthread_mutex_lock()
		// more than once from the same thread before calling
		// pthread_mutex_unlock().
		//
		// Doing the same using the default mutex attributes
		// would cause deadlock.
		//
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&m_cs, &attr);
		pthread_mutexattr_destroy(&attr);
#endif
	}
	inline ~CCriticalSectionLock() throw()
	{
#if	defined(_WIN32)
		DeleteCriticalSection(&m_cs);
#else
		pthread_mutex_destroy(&m_cs);
#endif
	}

	// Public Methods
public:
#pragma warning(suppress: 26135)	// The _Acquires_lock_() annotation seems like it is correct, but clearly isn't as Warning 26135 is still generated. TODO: resolve.
	_Acquires_lock_(m_cs)
	inline void Lock() throw()
	{
#if	defined(_WIN32)
		EnterCriticalSection(&m_cs);
#else
		pthread_mutex_lock(&m_cs);
#endif
	}
#pragma warning(suppress: 26135)	// The _Acquires_lock_() annotation seems like it is correct, but clearly isn't as Warning 26135 is still generated. TODO: resolve.
	_Releases_lock_(m_cs)
	inline void Unlock() throw()
	{
#if	defined(_WIN32)
		LeaveCriticalSection(&m_cs);
#else
		pthread_mutex_unlock(&m_cs);
#endif
	}

	// Protected Members
protected:
#if	defined(_WIN32)
	CRITICAL_SECTION m_cs;
#else
	pthread_mutex_t m_cs;
#endif
};

#endif // CRITICAL_SECTION_LOCK_H
