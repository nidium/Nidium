/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "NativeUtils.h"

#include <stdlib.h>
#include <stdio.h>
#include <jsstr.h>
#include <ape_sha1.h>
#include <ape_base64.h>

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
#ifdef CLOCK_MONOTONIC_RAW
  #define USED_CLOCK CLOCK_MONOTONIC_RAW
#else
  #define USED_CLOCK CLOCK_MONOTONIC
#endif
    clock_gettime(USED_CLOCK, &t);

    return (uint64_t)t.tv_sec * 1000000000 + (uint64_t)t.tv_nsec;
}
#endif
#endif

/* TODO: iOS : http://shiftedbits.org/2008/10/01/mach_absolute_time-on-the-iphone/ */
uint64_t NativeUtils::getTick(bool ms)
{
    return mach_absolute_time() / (ms ? 1000000LL : 1LL);
}

uint16_t *NativeUtils::Utf8ToUtf16(const char *str, size_t len, size_t *outputlen)
{
    *outputlen = sizeof(jschar) * (len + 1);

    jschar *jsc = (jschar *)malloc(*outputlen);

    if (!js::InflateUTF8StringToBufferReplaceInvalid(NULL, str, len, jsc, outputlen)) {
        free(jsc);
        return NULL;
    }

    // null terminate
    jsc[*outputlen] = jschar(0);

    return jsc;
}

void NativeUtils::sha1hmac(const unsigned char *key, uint32_t keylen,
    const unsigned char *buf, uint32_t buflen, unsigned char out[20])
{
    sha1_hmac((unsigned char *)key, keylen, (unsigned char *)buf, buflen, out);
}

void NativeUtils::sha1(const unsigned char *buf, uint32_t buflen, unsigned char out[20])
{
    sha1_csum((unsigned char *)buf, buflen, out);
}

char *NativeUtils::b64Encode(const unsigned char *buf, size_t len)
{
    return base64_encode((unsigned char *)buf, len);
}

int NativeUtils::b64Decode(unsigned char *out, const char *in, int out_length)
{
    return base64_decode(out, in, out_length);
}

static const char  *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char  *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/*
    Taken from
    ngx_time.c
*/
static void
ngx_gmtime(time_t t, struct tm *tp)
{
    int32_t   yday;
    uint32_t  n, sec, min, hour, mday, mon, year, wday, days, leap;

    /* the calculation is valid for positive time_t only */

    n = (uint32_t) t;

    days = n / 86400;

    /* January 1, 1970 was Thursday */

    wday = (4 + days) % 7;

    n %= 86400;
    hour = n / 3600;
    n %= 3600;
    min = n / 60;
    sec = n % 60;

    /*
     * the algorithm based on Gauss' formula,
     * see src/http/ngx_http_parse_time.c
     */

    /* days since March 1, 1 BC */
    days = days - (31 + 28) + 719527;

    /*
     * The "days" should be adjusted to 1 only, however, some March 1st's go
     * to previous year, so we adjust them to 2.  This causes also shift of the
     * last February days to next year, but we catch the case when "yday"
     * becomes negative.
     */

    year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);

    yday = days - (365 * year + year / 4 - year / 100 + year / 400);

    if (yday < 0) {
        leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));
        yday = 365 + leap + yday;
        year--;
    }

    /*
     * The empirical formula that maps "yday" to month.
     * There are at least 10 variants, some of them are:
     *     mon = (yday + 31) * 15 / 459
     *     mon = (yday + 31) * 17 / 520
     *     mon = (yday + 31) * 20 / 612
     */

    mon = (yday + 31) * 10 / 306;

    /* the Gauss' formula that evaluates days before the month */

    mday = yday - (367 * mon / 12 - 30) + 1;

    if (yday >= 306) {

        year++;
        mon -= 10;

        /*
         * there is no "yday" in Win32 SYSTEMTIME
         *
         * yday -= 306;
         */

    } else {

        mon += 2;

        /*
         * there is no "yday" in Win32 SYSTEMTIME
         *
         * yday += 31 + 28 + leap;
         */
    }

    tp->tm_sec = (int)sec;
    tp->tm_min =  (int)min;
    tp->tm_hour = (int)hour;
    tp->tm_mday = (int)mday;
    tp->tm_mon = (int)mon;
    tp->tm_year = (int)year;
    tp->tm_wday = (int)wday;
}

void NativeUtils::HTTPTime(char *buf)
{
    struct tm timenow;

    ngx_gmtime(time(NULL), &timenow);

    sprintf(buf, "%s, %02d %s %4d %02d:%02d:%02d GMT",
                           week[timenow.tm_wday],
                           timenow.tm_mday,
                           months[timenow.tm_mon - 1],
                           timenow.tm_year,
                           timenow.tm_hour,
                           timenow.tm_min,
                           timenow.tm_sec);
}
