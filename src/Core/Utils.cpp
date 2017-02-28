/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Core/Utils.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#ifndef _MSC_VER
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#include <ape_sha1.h>
#include <ape_base64.h>
#include <ape_blowfish.h>

namespace Nidium {
namespace Core {

// {{{ Time related

/* TODO :
 * http://nadeausoftware.com/articles/2012/04/c_c_tip_how_measure_elapsed_real_time_benchmarking
 */

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

    s.wYear         = 1970;
    s.wMonth        = 1;
    s.wDay          = 1;
    s.wHour         = 0;
    s.wMinute       = 0;
    s.wSecond       = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return (t);
}

int clock_gettime(int X, struct timeval *tv)
{
    LARGE_INTEGER t;
    FILETIME f;
    double microseconds;
    static LARGE_INTEGER offset;
    static double frequencyToMicroseconds;
    static int initialized            = 0;
    static BOOL usePerformanceCounter = 0;

    if (!initialized) {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter
            = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter) {
            QueryPerformanceCounter(&offset);
            frequencyToMicroseconds
                = static_cast<double>(performanceFrequency.QuadPart) / 1000000.;
        } else {
            offset                  = getFILETIMEoffset();
            frequencyToMicroseconds = 10.;
        }
    }
    if (usePerformanceCounter)
        QueryPerformanceCounter(&t);
    else {
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = f.dwHighDateTime;
        t.QuadPart <<= 32;
        t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    microseconds = static_cast<double>(t.QuadPart) / frequencyToMicroseconds;
    t.QuadPart   = microseconds;
    tv->tv_sec   = t.QuadPart / 1000000;
    tv->tv_usec = t.QuadPart % 1000000;
    return (0);
}
static __inline uint64_t mach_absolute_time()
{
    struct timeval t;
    clock_gettime(0, &t);

    return ((uint64_t)t.tv_sec * 1000000 + (uint64_t)t.tv_usec) * 1000;
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

    return static_cast<uint64_t>(t.tv_sec) * 1000000000
           + static_cast<uint64_t>(t.tv_nsec);
}
#endif
#endif

/* TODO: iOS :
 * http://shiftedbits.org/2008/10/01/mach_absolute_time-on-the-iphone/ */
uint64_t Utils::GetTick(bool ms)
{
    return mach_absolute_time() / (ms ? 1000000LL : 1LL);
}

static const char *week[]   = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

/*
    Taken from ngx_time.c
*/
static void ngx_gmtime(time_t t, struct tm *tp)
{
    int32_t yday;
    uint32_t n, sec, min, hour, mday, mon, year, wday, days;

    /* the calculation is valid for positive time_t only */

    n = static_cast<uint32_t>(t);

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
        uint32_t leap;

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

    tp->tm_sec  = static_cast<int>(sec);
    tp->tm_min  = static_cast<int>(min);
    tp->tm_hour = static_cast<int>(hour);
    tp->tm_mday = static_cast<int>(mday);
    tp->tm_mon  = static_cast<int>(mon);
    tp->tm_year = static_cast<int>(year);
    tp->tm_wday = static_cast<int>(wday);
}

void Utils::HTTPTime(char *buf)
{
    struct tm timenow;

    ngx_gmtime(time(NULL), &timenow);

    sprintf(buf, "%s, %02d %s %4d %02d:%02d:%02d GMT", week[timenow.tm_wday],
            timenow.tm_mday, months[timenow.tm_mon - 1], timenow.tm_year,
            timenow.tm_hour, timenow.tm_min, timenow.tm_sec);
}
// }}}

// {{{ Conversion/Hashing related
static uint8_t nibbleFromChar(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;

    return 255;
}

void Utils::SHA1hmac(const unsigned char *key,
                     uint32_t keylen,
                     const unsigned char *buf,
                     uint32_t buflen,
                     unsigned char out[20])
{
    // TODO: new style cast
    sha1_hmac((unsigned char *)(key), keylen, (unsigned char *)(buf), buflen,
              out);
}

void Utils::SHA1(const unsigned char *buf,
                 uint32_t buflen,
                 unsigned char out[20])
{
    // TODO: new style cast
    sha1_csum((unsigned char *)(buf), buflen, out);
}

char *Utils::B64Encode(const unsigned char *buf, size_t len)
{
    // TODO: new style cast
    return base64_encode((unsigned char *)(buf), len);
}

int Utils::B64Decode(unsigned char *out, const char *in, int out_length)
{
    return base64_decode(out, in, out_length);
}

int Utils::B16Decode(unsigned char *out, const char *in, int out_length)
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

void Utils::BlowfishDecrypt(uint8_t *data, const uint8_t *key, int key_len)
{
    struct APEBlowfish ctx;

    uint32_t xl = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    uint32_t xr = data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7];

    APE_blowfish_init(&ctx, key, key_len);

    APE_blowfish_crypt_ecb(&ctx, &xl, &xr, 1);

    xl = ntohl(xl);
    xr = ntohl(xr);

    memcpy(data, &xl, sizeof(uint32_t));
    memcpy(data + sizeof(uint32_t), &xr, sizeof(uint32_t));
}

int Utils::FFT(int dir, int nn, double *x, double *y)
{
    long m, i, i1, j, i2, l, l2;
    double c1, c2, tx, ty, t1, t2, z;

    m = log2(nn);

    /* Do the bit reversal */
    i2 = nn >> 1;
    j = 0;
    for (i = 0; i < nn - 1; i++) {
        long k;

        if (i < j) {
            tx   = x[i];
            ty   = y[i];
            x[i] = x[j];
            y[i] = y[j];
            x[j] = tx;
            y[j] = ty;
        }
        k = i2;
        while (k <= j) {
            j -= k;
            k >>= 1;
        }
        j += k;
    }

    /* Compute the FFT */
    c1 = -1.0;
    c2 = 0.0;
    l2 = 1;
    for (l = 0; l < m; l++) {
        long l1;
        double u1, u2;

        l1 = l2;
        l2 <<= 1;
        u1 = 1.0;
        u2 = 0.0;
        for (j = 0; j < l1; j++) {
            for (i = j; i < nn; i += l2) {
                i1    = i + l1;
                t1    = u1 * x[i1] - u2 * y[i1];
                t2    = u1 * y[i1] + u2 * x[i1];
                x[i1] = x[i] - t1;
                y[i1] = y[i] - t2;
                x[i] += t1;
                y[i] += t2;
            }
            z  = u1 * c1 - u2 * c2;
            u2 = u1 * c2 + u2 * c1;
            u1 = z;
        }
        c2 = sqrt((1.0 - c1) / 2.0);
        if (dir == 1) c2 = -c2;
        c1 = sqrt((1.0 + c1) / 2.0);
    }

    /* Scaling for forward transform */
    if (dir == 1) {
        for (i = 0; i < nn; i++) {
            x[i] /= (double)nn;
            y[i] /= (double)nn;
        }
    }

    return true;
}

// }}}

} // namespace Core
} // namespace Nidium
