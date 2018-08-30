/////////////////////////////////////////////////////////////////////////////
// This is a Linux version of the O/S abstraction layer which implements
// O/S-dependent primitives and can help write portable apps.
/////////////////////////////////////////////////////////////////////////////

#include "OsDep.h"
#include <time.h>
#include <unistd.h> // for usleep

// Get system tick count in milliseconds
DWORD GetTickCount()
{
    DWORD count;
    timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts))
    {
        return 0;
    }

    count = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;

    return count;
}

void Sleep( DWORD msec)
{
    usleep( msec * 1000);
}
