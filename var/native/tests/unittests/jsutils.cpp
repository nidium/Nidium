/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <native_netlib.h>
#include <JS/NativeJS.h>
#include <JS/NativeJSUtils.h>

TEST(NativeJSUtils, String)
{
    char * cstr;
    const char *text = " .c89032 pg98u 2.dhu982 89";
    const char * te = text; //@TODO: encode as utf8;
    char target[32];
    bool success;

    memset(&target[0], '\0', sizeof(target));
    ape_global *g_ape = APE_init();
    NativeJS njs(g_ape);

    JS::RootedValue rval(njs.cx, JSVAL_VOID);
    success = NativeJSUtils::strToJsval(njs.cx, text, strlen(text), &rval, "utf8");
    EXPECT_TRUE(success == true);
    JS::RootedString jstr(njs.cx,  rval.toString());
    cstr = JS_EncodeString(njs.cx, jstr);
    EXPECT_TRUE(strcmp(te, cstr) == 0);
    free(cstr);

    success = NativeJSUtils::strToJsval(njs.cx, text, strlen(text), &rval, "asciiiiii");
    EXPECT_TRUE(success == true);
    jstr = rval.toString();
    cstr = JS_EncodeString(njs.cx, jstr);
    EXPECT_TRUE(strcmp(te, cstr) == 0);
    free(cstr);

    jstr = NativeJSUtils::newStringWithEncoding(njs.cx, text, strlen(text), "utf8");
    cstr = JS_EncodeString(njs.cx, jstr);
    EXPECT_TRUE(strcmp(te, cstr) == 0);
    free(cstr);

    jstr = NativeJSUtils::newStringWithEncoding(njs.cx, text, strlen(text), "asciiiiii");
    cstr = JS_EncodeString(njs.cx, jstr);
    EXPECT_TRUE(strcmp(te, cstr) == 0);
    free(cstr);

    APE_destroy(g_ape);
}

