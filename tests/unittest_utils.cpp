#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_sha1.h>
#include <ape_base64.h>

#include <NativeUtils.h>

TEST(NativeUtils, NativeNoncopyable)
{
    class NativeNoncopyable *nc = new NativeNoncopyable();

    delete nc;
}

TEST(NativeUtils, NativeTick)
{
    uint64_t tick;
    NativeUtils nu;

    tick = nu.getTick(true);
    EXPECT_TRUE(tick > 10);
}

TEST(NativeUtils, Utf8)
{
    uint16_t * jstr;
    const char * cstr;
    size_t len;

    len = 0;
    cstr = "',.pyfgcrl/=aOEUIDHTNS-;QJKXBMWVZ`1234567890[]\t\n";
    jstr = NativeUtils::Utf8ToUtf16(cstr, strlen(cstr), &len);
    EXPECT_EQ(len, 48);
    free(jstr);

    jstr = NativeUtils::Utf8ToUtf16(cstr, 1, &len);
    EXPECT_TRUE(jstr[0] == cstr[0]);
    EXPECT_EQ(len, 1);
    free(jstr);
}

TEST(NativeUtils, Mainthread)
{
    EXPECT_TRUE(NativeUtils::isMainThread());
}

TEST(NativeUtils, sha1hmac)
{
    char * key = strdup("dvorak");
    char * buf = strdup("querty");
    unsigned char out1[20], out2[20];

    sha1_hmac((unsigned char *)key, strlen(key), (unsigned char *)buf, strlen(buf), out1);
    NativeUtils::sha1hmac((unsigned char*)key, strlen(key), (unsigned char*)buf, strlen(buf), out2);
    EXPECT_TRUE(strncmp((char*)&out1[0], (char*)&out2[0], 20) == 0);

    free(key);
    free(buf);
}

TEST(NativeUtils, sha1)
{
    char * buf = strdup("superman");
    unsigned char out1[20], out2[20];

    sha1_csum((unsigned char *)buf, strlen(buf), out1);
    NativeUtils::sha1((unsigned char*)buf, strlen(buf), out2);
    EXPECT_TRUE(strncmp((char*)&out1[0], (char*)&out2[0], 20) == 0);

    free(buf);
}

TEST(NativeUtils, b64)
{
#define B64_OUT_LENGTH 32
    char * buf = strdup("spiderman");
    char *out1, *out2;
    unsigned char out12[B64_OUT_LENGTH], out22[B64_OUT_LENGTH];
    memset(out12, '\0', B64_OUT_LENGTH);
    memset(out22, '\0', B64_OUT_LENGTH);

    out1 = out2 = NULL;
    out1 = base64_encode((unsigned char *)buf, strlen(buf));
    out2 = NativeUtils::b64Encode((unsigned char*)buf, strlen(buf));
    EXPECT_TRUE(out1 != NULL);
    EXPECT_TRUE(strcmp(out1, out2) == 0);

    base64_decode(&out12[0], out1, B64_OUT_LENGTH);
    NativeUtils::b64Decode(&out22[0], out2, B64_OUT_LENGTH);
    EXPECT_TRUE(strcmp((char*)&out12[0], (char*)&out22[0]) == 0);
    EXPECT_TRUE(strcmp(buf, (char*)&out22[0]) == 0);

#undef B64_OUT_LENGTH
    free(out1);
    free(out2);
    free(buf);
}

TEST(NativeUtils, HttpTime)
{
    char buf[35];
    memset(&buf[0], '\0', 35);

    NativeUtils::HTTPTime(buf);
    EXPECT_TRUE(strlen(buf) > 0);
}

TEST(NativeUtils, MinMax)
{
    size_t mini, maxi;
    long minil, maxil;

    mini = 16;
    maxi = 129;
    minil = native_min(mini, maxi);
    maxil = native_max(mini, maxi);
    EXPECT_EQ(minil, mini);
    EXPECT_EQ(maxil, maxi);
    EXPECT_EQ(native_clamp(1, mini, maxi), 16);
    EXPECT_EQ(native_clamp(17, mini, maxi), 17);
    EXPECT_EQ(native_clamp(177, mini, maxi), 129);
}

struct constStrMacro{
        const char * str;
        const int len;
};

#if 0
TEST(NativeUtils, TreadLock)
{
    pthread_mutex_t mutex;

    class NativePthreadAutoLock * pt = new NativePthreadAutoLock(&mutex);

    delete pt;
}
#endif

TEST(NativeUtils, NativePtrAuteDelete)
{
    int i;

    i = 0;
    NativePtrAutoDelete<int*> * adp = new NativePtrAutoDelete<int*>(&i, NULL);
    EXPECT_TRUE(adp != NULL);

    i++;
    int *ip = adp->ptr();
    EXPECT_TRUE(ip == &i);
    EXPECT_EQ(*ip, 1);

    adp->disable();
    ip = adp->ptr();
    EXPECT_TRUE(ip == NULL);

    delete adp;
}

TEST(NativeUtils, ConstStrLenMacro)
{
        {
#define MYSTR NULL
        struct constStrMacro test = { CONST_STR_LEN(MYSTR)};
        EXPECT_EQ(test.len, 0);
        EXPECT_TRUE(test.str == NULL);
#undef MYSTR
        }
        {
#define MYSTR "HELLO"
        struct constStrMacro test = { CONST_STR_LEN(MYSTR)};
        EXPECT_EQ(test.len, 5);
        EXPECT_TRUE(strcmp(test.str, MYSTR) == 0);
#undef MYSTR
        }
        {
#define MYSTR "H\0ELLO"
        struct constStrMacro test = { CONST_STR_LEN(MYSTR)};
        EXPECT_EQ(test.len, 6);
        EXPECT_TRUE(strcmp(test.str, "H") == 0);
#undef MYSTR
        }

}

