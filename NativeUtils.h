/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativeutils_h__
#define nativeutils_h__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <jsapi.h>

class NativeNoncopyable {
public:
    NativeNoncopyable() {}

private:
    NativeNoncopyable(const NativeNoncopyable&);
    NativeNoncopyable& operator=(const NativeNoncopyable&);
};

class NativeUserAgentUtils
{
public:
    enum OS {
        WINDOWS,
        MAC,
        UNIX,
        OTHER
    };

    /* Fast OS detection */
    static OS getOS(const char *ua) {
        const char *paddr = strchr(ua, '(');

        if (!paddr || !paddr[1] || !paddr[2]) {
            return OTHER;
        }

        switch (paddr[1]) {
            case 'W': /* Windows */
            case 'w': /* windows */
            case 'c': /* compatible */
            case 'C': /* Compatible */
                return WINDOWS;
            case 'X': /* X11 */
                return UNIX;
            case 'M':
            {
                if (paddr[2] == 'S') { /* MSIE */
                    return WINDOWS;
                } else if (paddr[2] == 'a') { /* Macintosh */
                    return MAC;
                }
            }
        }
        return OTHER;
    }

};

class NativeUtils
{
public:
    static uint64_t getTick(bool ms = false);
    static char16_t *Utf8ToUtf16(JSContext *cx, const char *str, size_t len, size_t *outputlen);
    static bool isMainThread()
    {
        // TODO : Windows support and better implementation
        return getpid() == syscall(SYS_gettid);
    }

    static void sha1hmac(const unsigned char *key, uint32_t keylen,
        const unsigned char *buf, uint32_t buflen, unsigned char out[20]);
    static void sha1(const unsigned char *buf, uint32_t buflen, unsigned char out[20]);
    static char *b64Encode(const unsigned char *buf, size_t len);
    static int b64Decode(unsigned char *out, const char *in, int out_length);
    static void blowfishDecrypt(uint8_t *data, const uint8_t *key, int key_len);
    static int b16Decode(unsigned char *out, const char *in, int out_length);

template <typename T>
    static T randInt()
    {
        int random;
        T ret = 0;

        /* TODO: keep open */
        random = open("/dev/urandom", O_RDONLY);

        if (!random) {
            fprintf(stderr, "Cannot open /dev/urandom\n");
            return 0;
        }

        read(random, &ret, sizeof(T));
        close(random);

        return ret;
    }

    static void HTTPTime(char *buf);
};

class NativePthreadAutoLock {
  public:
    NativePthreadAutoLock(pthread_mutex_t *mutex)
      : lock(mutex) {

        pthread_mutex_lock(lock);
    }

    ~NativePthreadAutoLock() {
        pthread_mutex_unlock(lock);
    }
  private:
    pthread_mutex_t *lock;
};

template <typename T = void *>
class NativePtrAutoDelete {
  public:
    NativePtrAutoDelete(T ptr = NULL, void (*func)(void *) = NULL)
      : m_Ptr(ptr), m_Free(func) {
    }

    void set(T ptr) {
        m_Ptr = ptr;
    }

    T ptr() const {
        return m_Ptr;
    }

    operator T() {
        return m_Ptr;
    }

    void disable() {
        m_Ptr = NULL;
    }

    ~NativePtrAutoDelete() {
        if (!m_Ptr) return;

        if (!m_Free) {
            delete (T) m_Ptr;
        } else {
            m_Free(m_Ptr);
        }
    }
  private:
    T m_Ptr;
    void (*m_Free)(void *);
};

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define native_clamp(val, min, max) native_max(native_min(val, max), min)

#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1 : 0

#define APE_CTX(CX) ((ape_global *)JS_GetContextPrivate(CX))

#endif

