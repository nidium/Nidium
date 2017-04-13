/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <ape_sha1.h>
#include <ape_base64.h>

#include <Core/Utils.h>

TEST(Utils, NonCopyable)
{
    class Nidium::Core::NonCopyable *nc = new Nidium::Core::NonCopyable();

    delete nc;
}

TEST(Utils, NativeTick)
{
    uint64_t tick;
    Nidium::Core::Utils nu;

    tick = nu.GetTick(true);
    EXPECT_TRUE(tick > 10);
}

TEST(Utils, SHA1hmac)
{
    char * key = strdup("dvorak");
    char * buf = strdup("querty");
    unsigned char out1[20], out2[20];

    sha1_hmac((unsigned char *)key, strlen(key), (unsigned char *)buf, strlen(buf), out1);
    Nidium::Core::Utils::SHA1hmac((unsigned char*)key, strlen(key), (unsigned char*)buf, strlen(buf), out2);
    EXPECT_TRUE(strncmp((char*)&out1[0], (char*)&out2[0], 20) == 0);

    free(key);
    free(buf);
}

TEST(Utils, SHA1)
{
    char * buf = strdup("superman");
    unsigned char out1[20], out2[20];

    sha1_csum((unsigned char *)buf, strlen(buf), out1);
    Nidium::Core::Utils::SHA1((unsigned char*)buf, strlen(buf), out2);
    EXPECT_TRUE(strncmp((char*)&out1[0], (char*)&out2[0], 20) == 0);

    free(buf);
}

TEST(Utils, B64)
{
#define B64_OUT_LENGTH 32
    char * buf = strdup("spiderman");
    char *out1, *out2;
    unsigned char out12[B64_OUT_LENGTH], out22[B64_OUT_LENGTH];
    memset(out12, '\0', B64_OUT_LENGTH);
    memset(out22, '\0', B64_OUT_LENGTH);

    out1 = out2 = NULL;
    out1 = base64_encode((unsigned char *)buf, strlen(buf));
    out2 = Nidium::Core::Utils::B64Encode((unsigned char*)buf, strlen(buf));
    EXPECT_TRUE(out1 != NULL);
    EXPECT_TRUE(strcmp(out1, out2) == 0);

    base64_decode(&out12[0], out1, B64_OUT_LENGTH);
    Nidium::Core::Utils::B64Decode(&out22[0], out2, B64_OUT_LENGTH);
    EXPECT_TRUE(strcmp((char*)&out12[0], (char*)&out22[0]) == 0);
    EXPECT_TRUE(strcmp(buf, (char*)&out22[0]) == 0);

#undef B64_OUT_LENGTH
    free(out1);
    free(out2);
    free(buf);
}

TEST(Utils, HttpTime)
{
    char buf[35];
    memset(&buf[0], '\0', 35);

    Nidium::Core::Utils::HTTPTime(buf);
    EXPECT_TRUE(strlen(buf) > 0);
}

TEST(Utils, MinMax)
{
    size_t mini, maxi;
    long minil, maxil;

    mini = 16;
    maxi = 129;
    minil = nidium_min(mini, maxi);
    maxil = nidium_max(mini, maxi);
    EXPECT_EQ(minil, mini);
    EXPECT_EQ(maxil, maxi);
    EXPECT_EQ(nidium_clamp(1, mini, maxi), 16);
    EXPECT_EQ(nidium_clamp(17, mini, maxi), 17);
    EXPECT_EQ(nidium_clamp(177, mini, maxi), 129);
}

struct constStrMacro{
        const char * str;
        const int len;
};

#if 0
// This is locking oO
TEST(Utils, TreadLock)
{
    pthread_mutex_t mutex;

    class Nidium::Core::PthreadAutoLock * pt = new Nidium::Core::PthreadAutoLock(&mutex);

    delete pt;
}
#endif

TEST(Utils, PtrAuteDelete)
{
    int i;

    i = 0;
    Nidium::Core::PtrAutoDelete<int*> * adp = new Nidium::Core::PtrAutoDelete<int*>(&i, NULL);
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

TEST(Utils, UserAgentUtils_GetOS)
{
    EXPECT_TRUE(Nidium::Core::UserAgentUtils::kOS_Windows == Nidium::Core::UserAgentUtils::GetOS("Mozilla/5.0 (compatible; U; ABrowse 0.6; Syllable) AppleWebKit/420+ (KHTML, like Gecko)"));
    EXPECT_TRUE(Nidium::Core::UserAgentUtils::kOS_Windows == Nidium::Core::UserAgentUtils::GetOS("Mozilla/4.0 (compatible; Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.0; Trident/4.0; Acoo Browser 1.98.744; .NET CLR 3.5.30729); Windows NT 5.1; Trident/4.0)"));
    EXPECT_TRUE(Nidium::Core::UserAgentUtils::kOS_Windows == Nidium::Core::UserAgentUtils::GetOS("Mozilla/4.0 (compatible; MSIE 7.0; AOL 9.1; AOLBuild 4334.5000; Windows NT 5.1; Trident/4.0)"));
    EXPECT_TRUE(Nidium::Core::UserAgentUtils::kOS_Other == Nidium::Core::UserAgentUtils::GetOS("wget"));
}

TEST(Utils, ConstStrLenMacro)
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

TEST(Utils, filename)
{
    EXPECT_EQ(strcmp(__FILENAME__, "utils.cpp" ), 0);

}
