#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "unittest.h"

#include <NativeFileStream.h>
#include <NativeHTTPStream.h>
#include <NativePath.h>


#define MAX_PATH_LENGTH 1024

TEST(NativePath, Nulling)
{
    g_m_Root = NULL;
    g_m_Pwd = NULL;
    NativePath path(NULL);

    EXPECT_TRUE(path.path() == NULL);
    EXPECT_EQ(NativePath::g_m_SchemesCount, 0);
    NativePath::registerScheme(SCHEME_DEFINE("file://", NativeFileStream, false), true);
    NativePath::registerScheme(SCHEME_DEFINE("http://",    NativeHTTPStream,    true));
    EXPECT_EQ(NativePath::g_m_SchemesCount, 2);
    EXPECT_TRUE(strcmp(path.g_m_DefaultScheme->str, "file://") == 0);

#if 0
    EXPECT_TRUE(strcmp(path.getDir("/tmp/t.txt"), "/tmp") == 0);
    EXPECT_TRUE(strcmp(path.getDir("/tmp/t.txt/"), "/tmp/t.txt") == 0);
    EXPECT_TRUE(strcmp(path.getDir("file:///tmp/t.txt"), "file:///tmp") == 0);
    EXPECT_TRUE(strcmp(path.getDir("file:///tmp/t.txt/"), "file:///tmp/t.txt") == 0);
#endif
}

TEST(NativePath, Relative)
{
    g_m_Root = NULL;
    g_m_Pwd = NULL;
    NativePath path(NULL);

    EXPECT_TRUE(path.isRelative("file:///tmp/t") == false);
    EXPECT_TRUE(path.isRelative("/tmp/t") == false);
    EXPECT_TRUE(path.isRelative("./tmp/t") == true);
    EXPECT_TRUE(path.isRelative("../tmp/t") == true);
}

TEST(NativePath, Simple)
{
    g_m_Root = NULL;
    g_m_Pwd = NULL;
    const char *p ="/tmp/t.txt";
    NativePath path(p);

    EXPECT_TRUE(strcmp(path.path(), p) == 0);
    EXPECT_TRUE(path.getRoot() == NULL);
    EXPECT_TRUE(path.getPwd() == NULL);
    EXPECT_TRUE(strncmp(path.dir(), p, 4) == 0);
    EXPECT_TRUE(strcmp(path.getScheme()->str, "file://") == 0);

}

TEST(NativePath, InvallidScheme)
{
    g_m_Root = NULL;
    g_m_Pwd = NULL;
    const char *p ="doggystyle:///tmp/t.txt";
    NativePath path(p);

    EXPECT_TRUE(path.getRoot() == NULL);
    EXPECT_TRUE(path.getPwd() == NULL);
    EXPECT_TRUE(strcmp(path.getScheme()->str, "file://") == 0);
}

TEST(NativePath, SimpleScheme)
{
    g_m_Root = NULL;
    g_m_Pwd = NULL;
    const char *p ="file:///tmp/t.txt";
    char pwd_buf[MAX_PATH_LENGTH];
    NativePath path(p);
    char * pwd = getcwd(&pwd_buf[0], MAX_PATH_LENGTH);
    size_t len = strlen(pwd);

    pwd[len] = '/';
    pwd[len + 1] = '\0';

    //EXPECT_TRUE(strcmp(path.path(), pwd) == 0);
    EXPECT_TRUE(path.getRoot() == NULL);
    EXPECT_TRUE(path.getPwd() == NULL);
    EXPECT_TRUE(strcmp(path.dir(), pwd) == 0);
    EXPECT_TRUE(strcmp(path.getScheme()->str, "file://") == 0);
}


TEST(NativePath, RelativeFile)
{
    g_m_Root = NULL;
    g_m_Pwd = NULL;
    const char *p ="./tmp/t.txt";
    char pwd_buf[MAX_PATH_LENGTH];
    NativePath path(p);
    char * pwd = getcwd(&pwd_buf[0], MAX_PATH_LENGTH);
    size_t len = strlen(pwd);

    pwd[len] = '/';
    pwd[len + 1] = '\0';
#if 0
printf("path: %s\n", path.path());
printf("root: %s\n", path.getRoot());
printf("pwd:  %s\n", path.getPwd());
printf("dir:  %s\n", path.dir());
//printf("sch:  %s\n", path.getScheme()->str);
printf("\n");
#endif
    //EXPECT_TRUE(strcmp(path.path(), pwd) == 0);
    EXPECT_TRUE(path.getRoot() == NULL);
    EXPECT_TRUE(path.getPwd() == NULL);
    EXPECT_TRUE(strcmp(path.dir(), pwd) == 0);
    EXPECT_TRUE(strcmp(path.getScheme()->str, "file://") == 0);
}

TEST(NativePath, Changes)
{
    g_m_Root = NULL;
    g_m_Pwd = NULL;
    const char * p ="/tmp/t.txt";
    NativePath path(p);

    EXPECT_TRUE(strcmp(path.path(), p) == 0);
    EXPECT_TRUE(path.getRoot() == NULL);
    EXPECT_TRUE(path.getPwd() == NULL);
    EXPECT_TRUE(strcmp(path.dir(), "/tmp/") == 0);
    path.cd("/dummy/");
    EXPECT_TRUE(strcmp(path.path(), p) == 0);
    EXPECT_TRUE(path.getRoot() == NULL);
    EXPECT_TRUE(strcmp(path.getPwd(), "/dummy/") == 0);
    EXPECT_TRUE(strcmp(path.dir(), "/tmp/") == 0);
    path.chroot("/happy/");
    EXPECT_TRUE(strcmp(path.path(), p) == 0);
    EXPECT_TRUE(strcmp(path.getRoot(), "/happy/") == 0);
    EXPECT_TRUE(strcmp(path.getPwd(), "/dummy/") == 0);
    EXPECT_TRUE(strcmp(path.dir(), "/tmp/") == 0);
}

#if 0
TEST(NativePath, Cleanup)
{
    NativePath::unRegisterSchemes();
    EXPECT_EQ(NativePath::g_m_SchemesCount, 0);
}
#endif

//@TODO: const char * NativePath::currentJSCaller(JSContext *cx)
//@TODO: static char *sanitize(const char *path, bool *external = NULL, bool relative = true);
//@TODO: NativeBaseStream *createStream(bool onlySync = false) const {
//@TODO: Constructor with allowAll and noFilter variants

#if 0
//FIXME: do not register already existing path
TEST(NativePath, Reregister)
{
    EXPECT_EQ(NativePath::g_m_SchemesCount, 2);
    NativePath::registerScheme(SCHEME_DEFINE("http://",    NativeHTTPStream,    true));
    EXPECT_EQ(NativePath::g_m_SchemesCount, 2);
}
#endif

#undef MAX_PATH_LENGTH

