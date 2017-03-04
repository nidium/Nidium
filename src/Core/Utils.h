/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_utils_h__
#define core_utils_h__

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>

#ifdef _MSC_VER
#include <io.h>
#include <time.h>
#include <prrng.h>
#else
#include <unistd.h>
#include <sys/syscall.h>
#endif

#include <array>

namespace Nidium {
namespace Core {

// {{{ NonCopyable
class NonCopyable
{
public:
    NonCopyable()
    {
    }

private:
    NonCopyable(const NonCopyable &);
    NonCopyable &operator=(const NonCopyable &);
};
// }}}

// {{{ UserAgentUtils
class UserAgentUtils
{
public:
    enum OS
    {
        kOS_Windows,
        kOS_Mac,
        kOS_Unix,
        kOS_Other
    };

    /* Fast OS detection */
    static OS GetOS(const char *ua)
    {
        const char *paddr = strchr(ua, '(');

        if (!paddr || !paddr[1] || !paddr[2]) {
            return UserAgentUtils::kOS_Other;
        }

        switch (paddr[1]) {
            case 'W': /* Windows */
            case 'w': /* windows */
            case 'c': /* compatible */
            case 'C': /* Compatible */
                return UserAgentUtils::kOS_Windows;
            case 'X': /* X11 */
                return UserAgentUtils::kOS_Unix;
            case 'M': {
                if (paddr[2] == 'S') { /* MSIE */
                    return UserAgentUtils::kOS_Windows;
                } else if (paddr[2] == 'a') { /* Macintosh */
                    return UserAgentUtils::kOS_Mac;
                }
            }
        }
        return UserAgentUtils::kOS_Other;
    }
};
// }}}

// {{{ Utils
class Utils
{
public:
    static uint64_t GetTick(bool ms = false);
    static void SHA1hmac(const unsigned char *key,
                         uint32_t keylen,
                         const unsigned char *buf,
                         uint32_t buflen,
                         unsigned char out[20]);
    static void
    SHA1(const unsigned char *buf, uint32_t buflen, unsigned char out[20]);
    static char *B64Encode(const unsigned char *buf, size_t len);
    static int B64Decode(unsigned char *out, const char *in, int out_length);
    static void BlowfishDecrypt(uint8_t *data, const uint8_t *key, int key_len);
    static int B16Decode(unsigned char *out, const char *in, int out_length);

    template <typename T>
    static T RandInt()
    {

        T ret = 0;
#if _MSC_VER
        int seed = 0;

        if (PR_GetRandomNoise(&seed, sizeof(seed)) == 0) {
            seed = time(NULL);
            //ndm_log(NDM_LOG_WARNING, "Utils", "Cannot find seed for random number.");
        }
        srand(seed);

        ret = rand();
#else
        int random;

        /* TODO: keep open */
        random = open("/dev/urandom", O_RDONLY);

        if (!random) {
            fprintf(stderr, "Cannot open /dev/urandom\n");
            return 0;
        }

        read(random, &ret, sizeof(T));
        close(random);
#endif

        return ret;
    }

    template <int size>
    static std::array<unsigned char, size> RandBytes()
    {
        int random;
        std::array<unsigned char, size> data;

        /* TODO: keep open from ape->urandom_fd */
        random = open("/dev/urandom", O_RDONLY);

        if (!random) {
            fprintf(stderr, "Cannot open /dev/urandom\n");
            return data;
        }

        read(random, &data[0], size);
        close(random);

        return data;
    }

    static void HTTPTime(char *buf);
    static int FFT(int dir, int nn, double *x, double *y);
};
// }}}

// {{{ PthreadLock
class PthreadAutoLock
{
public:
    PthreadAutoLock(pthread_mutex_t *mutex) : m_Lock(mutex)
    {

        pthread_mutex_lock(m_Lock);
    }

    ~PthreadAutoLock()
    {
        pthread_mutex_unlock(m_Lock);
    }

private:
    pthread_mutex_t *m_Lock;
};
// }}}

// {{{ PtrAutoDelete
template <typename T = void *>
class PtrAutoDelete
{
public:
    PtrAutoDelete(T ptr = NULL, void (*func)(void *) = NULL)
        : m_Ptr(ptr), m_Free(func)
    {
    }

    void set(T ptr)
    {
        m_Ptr = ptr;
    }

    T ptr() const
    {
        return m_Ptr;
    }

    operator T()
    {
        return m_Ptr;
    }

    void disable()
    {
        m_Ptr = NULL;
    }

    ~PtrAutoDelete()
    {
        if (!m_Ptr) return;

        if (!m_Free) {
            delete static_cast<T>(m_Ptr);
        } else {
            m_Free(m_Ptr);
        }
    }

private:
    T m_Ptr;
    void (*m_Free)(void *);
};
// }}}



// {{{ Macros
#define nidium_min(val1, val2) ((val1 > val2) ? (val2) : (val1))
#define nidium_max(val1, val2) ((val1 < val2) ? (val2) : (val1))
#define nidium_clamp(val, min, max) nidium_max(nidium_min(val, max), min)

#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1 : 0

#define APE_CTX(CX) ((ape_global *)JS_GetContextPrivate(CX))

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define NLOG(format, ...) \
    printf("[%s:%d] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)
// }}}

} // namespace Core
} // namespace Nidium

#endif
