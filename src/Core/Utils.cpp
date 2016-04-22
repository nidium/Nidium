/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Utils.h"

#include <stdio.h>
#include <string.h>

#include <ape_sha1.h>
#include <ape_base64.h>
#include <ape_blowfish.h>

namespace Nidium {
namespace Core {

// {{{ Time related

/* TODO : http://nadeausoftware.com/articles/2012/04/c_c_tip_how_measure_elapsed_real_time_benchmarking */

#if defined(__APPLE__)
  #include <mach/mach_time.h>
#else
  #include <time.h>
  #include <arpa/inet.h>

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
uint64_t Utils::getTick(bool ms)
{
    return mach_absolute_time() / (ms ? 1000000LL : 1LL);
}

static const char  *week[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char  *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/*
    Taken from ngx_time.c
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

void Utils::HTTPTime(char *buf)
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

// {{{ Conversion related

static uint8_t nibbleFromChar(char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;

    return 255;
}

void Utils::sha1hmac(const unsigned char *key, uint32_t keylen,
    const unsigned char *buf, uint32_t buflen, unsigned char out[20])
{
    sha1_hmac((unsigned char *)key, keylen, (unsigned char *)buf, buflen, out);
}

void Utils::sha1(const unsigned char *buf, uint32_t buflen, unsigned char out[20])
{
    sha1_csum((unsigned char *)buf, buflen, out);
}

char *Utils::b64Encode(const unsigned char *buf, size_t len)
{
    return base64_encode((unsigned char *)buf, len);
}

int Utils::b64Decode(unsigned char *out, const char *in, int out_length)
{
    return base64_decode(out, in, out_length);
}

int Utils::b16Decode(unsigned char *out, const char *in, int out_length)
{
    int len, i;
    int inlen = strlen(in);

    if ((inlen % 2)) {
        return 0;
    }

    len = strlen(in) / 2;

    for (i = 0; i < len; i++) {
        out[i] = nibbleFromChar(in[i * 2]) * 16 + nibbleFromChar(in[i * 2 + 1]);
    }

    return len;
}

void Utils::blowfishDecrypt(uint8_t *data, const uint8_t *key, int key_len)
{
    struct APEBlowfish ctx;

    uint32_t xl = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    uint32_t xr = data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7];

    APE_blowfish_init(&ctx, key, key_len);

    APE_blowfish_crypt_ecb(&ctx, &xl, &xr, 1);

    xl = ntohl(xl);
    xr = ntohl(xr);

    memcpy(data, &xl, sizeof(uint32_t));
    memcpy(data+sizeof(uint32_t), &xr, sizeof(uint32_t));
}

} // namespace Core
} // namespace Nidium

