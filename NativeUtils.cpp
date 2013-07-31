#include "NativeUtils.h"

#include <stdlib.h>
#include <stdio.h>

/* TODO : http://nadeausoftware.com/articles/2012/04/c_c_tip_how_measure_elapsed_real_time_benchmarking */

#if defined(__APPLE__)
  #include <mach/mach_time.h>
#else
  #include <time.h>
  
#ifdef __WIN32
LARGE_INTEGER
getFILETIMEoffset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}

int
clock_gettime(int X, struct timeval *tv)
{
    LARGE_INTEGER           t;
    FILETIME            f;
    double                  microseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = 0;

    if (!initialized) {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter) {
            QueryPerformanceCounter(&offset);
            frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
        } else {
            offset = getFILETIMEoffset();
            frequencyToMicroseconds = 10.;
        }
    }
    if (usePerformanceCounter) QueryPerformanceCounter(&t);
    else {
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = f.dwHighDateTime;
        t.QuadPart <<= 32;
        t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = microseconds;
    tv->tv_sec = t.QuadPart / 1000000;
    tv->tv_usec = t.QuadPart % 1000000;
    return (0);
}
static __inline uint64_t mach_absolute_time()
{
    struct timeval t;
    clock_gettime(0, &t);

    return ((uint64_t)t.tv_sec * 1000000 + (uint64_t)t.tv_usec)*1000;
}
#else // !__WIN32
static __inline uint64_t mach_absolute_time()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return (uint64_t)t.tv_sec * 1000000000 + (uint64_t)t.tv_nsec;
}
#endif
#endif

/* TODO: iOS : http://shiftedbits.org/2008/10/01/mach_absolute_time-on-the-iphone/ */
uint64_t NativeUtils::getTick()
{
    return mach_absolute_time();
}
