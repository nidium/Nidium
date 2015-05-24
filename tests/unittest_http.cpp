#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <NativeHTTP.h>

TEST(NativeHttp, Request)
{
    char * found = NULL;
    const char * cache = "no-cache";
    const char * key = "Pragma";
    const char * dat = "test=true";
    NativeHTTPRequest na("http://nidium.com/new.html");

    na.setHeader(key, cache);
    found = (char*) na.getHeader(key);
    EXPECT_TRUE(strcmp(found, cache) == 0);

    found = NULL;
    na.recycle();
    found = (char*)na.getHeader(key);
    EXPECT_TRUE(found == NULL);

    EXPECT_EQ(na.getPort(), 80);
    EXPECT_TRUE(na.isSSL() == false);
    EXPECT_TRUE(na.isValid() == true);
    EXPECT_TRUE(strcmp(na.getHost(), "nidium.com") == 0);
    EXPECT_TRUE(strcmp(na.getPath(), "/new.html") == 0);

    na.setPath("/newest.html");
    EXPECT_TRUE(strcmp(na.getPath(), "/newest.html") == 0);

    na.setData((char*)dat, strlen(dat));
    EXPECT_TRUE(strcmp(na.getData(), dat) == 0);
    EXPECT_EQ(na.getDataLength(), strlen(dat));

    na.resetURL("https://nidium.org:8080");
    EXPECT_TRUE(strcmp(na.getHost(), "nidium.org") == 0);
    EXPECT_EQ(na.getPort(), 8080);
    EXPECT_TRUE(na.isSSL() == true);
    EXPECT_TRUE(na.isValid() == true);
    EXPECT_TRUE(strcmp(na.getPath(), "/") == 0);
}

TEST(NativeHttp, Http)
{
    NativeHTTPRequest *request;
    ape_global *g_ape = native_netlib_init();
    NativeHTTP nm(g_ape);
    nm.http.headers.list = ape_array_new(1); // this should happen in the constructor....

    EXPECT_TRUE(nm.net == g_ape);
    EXPECT_TRUE(nm.m_CurrentSock == NULL);
    EXPECT_EQ(nm.err, 0);
    //@TODO: nm.http.*
    EXPECT_EQ(nm.m_Timeout, HTTP_DEFAULT_TIMEOUT);
    EXPECT_EQ(nm.m_TimeoutTimer, 0);
    EXPECT_TRUE(nm.delegate == NULL);
    EXPECT_TRUE(nm.native_http_data_type == NativeHTTP::DATA_NULL);

    nm.parsing(true);
    EXPECT_TRUE(nm.isParsing() == true);
    nm.parsing(false);
    EXPECT_TRUE(nm.isParsing() == false);

    EXPECT_EQ(nm.getFileSize(), 0);
    EXPECT_EQ(nm.getStatusCode(), 0);

    EXPECT_TRUE(nm.hasPendingError() == false);

    nm.setMaxRedirect(2);

    request = nm.getRequest();
    EXPECT_TRUE(request == NULL);

    nm.setPendingError(NativeHTTP::ERROR_RESPONSE);

    EXPECT_TRUE(nm.hasPendingError() == true);
    EXPECT_TRUE(nm.canDoRequest() == false);
    nm.clearPendingError();
    EXPECT_TRUE(nm.hasPendingError() == false);
    nm.canDoRequest(false);
    EXPECT_TRUE(nm.canDoRequest() == false);
    nm.canDoRequest(true);
    EXPECT_TRUE(nm.canDoRequest() == true);

    //keepalive
    EXPECT_TRUE(nm.isKeepAlive() == true);

#if 0
    //@TODO: ParseURI
    //@TODO: getPath
 @TODO:
    ape_array_add_camelkey_n(nm.http.headers.list, "connection", strlen("connection"), "close", strlen("close"));
    request->setHeader("connection", "close");
    EXPECT_TRUE(nm.isKeepAlive() == false);

    ape_array_add_camelkey_n(nm.http.headers.list, "connection", strlen("connection"), "close", strlen("close"));
    request->setHeader("connection", "keep-alive");
    EXPECT_TRUE(nm.isKeepAlive() == false);

    ape_array_add_camelkey_n(nm.http.headers.list, "connection", strlen("connection"), "keep-alive", strlen("keep-alive"));
    request->setHeader("connection", "close");
    EXPECT_TRUE(nm.isKeepAlive() == false);

    ape_array_add_camelkey_n(nm.http.headers.list, "connection", strlen("connection"), "keep-alive", strlen("keep-alive"));
    request->setHeader("connection", "keep-alive");
    EXPECT_TRUE(nm.isKeepAlive() == true);

    //@TODO: requestEnded
    //@TODO: headerEnded
    //@TODO: stopRequest
    //@TODO: clearTimeout
    //@TODO: onData
    //@TODO: setPrivate
    //@TODO: getPrivate
    //@TODO: resetData
    //@TODO: getFileSize
    //@TODO: getStatusCode
    //@TODO: close
    //@TODO: clearState
    native_netlib_destroy(g_ape);
#endif
}

/*
TEST(NativeHttp, Delegate)
{

}
*/

