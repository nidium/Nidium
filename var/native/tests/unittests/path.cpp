/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include <gtest/gtest.h>

#include <NativePath.h>
#include <NativeFileStream.h>
#include <NativeHTTPStream.h>
#include <NativeNFSStream.h>
#include <libgen.h>
#include <errno.h>

unsigned long _ape_seed = 31415961;

#define TEST_DIR "/tmp/"
#define TEST_DIR_OUTSIDE "/foo/bar/"
#define TEST_HOST "www.nidium.com"
#define TEST_URL "http://" TEST_HOST "/"
#define TEST_FILE "test_file"
#define TEST_PREFIX "nvfs://"
#define USER_STREAM_BASE_DIR "/tmp/"

#define TEST_ABS_FILE TEST_DIR TEST_FILE
#define TEST_ABS_OUTSIDE_FILE TEST_DIR_OUTSIDE TEST_FILE
#define TEST_REL_FILE TEST_FILE
#define TEST_REL_OUTSIDE_FILE "../" TEST_FILE
#define TEST_URL_FILE TEST_URL TEST_FILE
#define TEST_URL_DIR TEST_URL "tmp/"
#define TEST_PREFIX_FILE TEST_PREFIX TEST_FILE

/*
    Class for testing prefixed stream with a base dir different than "/"
*/
class NativeUserStream : public NativeFileStream
{
  public:
    explicit NativeUserStream(const char *location) :
        NativeFileStream(location)
    {
    }

    static NativeBaseStream *createStream(const char *location) {
        return new NativeUserStream(location);
    }

    static bool allowLocalFileStream() {
        return true;
    }

    static bool allowSyncStream() {
        return true;
    }

    static const char *getBaseDir() {
        return USER_STREAM_BASE_DIR;
    }
};

TEST(NativePath, RegisterScheme)
{
    NativePath::registerScheme(SCHEME_DEFINE("file://",    NativeFileStream,    false), true); // default
    NativePath::registerScheme(SCHEME_DEFINE("http://",    NativeHTTPStream,    true));
    NativePath::registerScheme(SCHEME_DEFINE("https://",   NativeHTTPStream,    true));
    NativePath::registerScheme(SCHEME_DEFINE("nvfs://",    NativeNFSStream,     false));
    NativePath::registerScheme(SCHEME_DEFINE("user://",    NativeUserStream,    false));
}

// {{{ Sanitize
TEST(NativePath, Sanitize)
{
    char *sanitized = NativePath::sanitize("dir/../");

    ASSERT_STREQ("", sanitized);

    free(sanitized);
}

TEST(NativePath, Sanitize2)
{
    char *sanitized = NativePath::sanitize("dir//./a/.././file");

    ASSERT_STREQ("dir/file", sanitized);

    free(sanitized);
}

TEST(NativePath, Sanitize3)
{
    char *sanitized = NativePath::sanitize("foo/bar/../file");

    ASSERT_STREQ("foo/file", sanitized);

    free(sanitized);
}

TEST(NativePath, SanitizeAbsoluteInvalid)
{
    char *sanitized = NativePath::sanitize(TEST_DIR "../../");

    ASSERT_STREQ(nullptr, sanitized);

    free(sanitized);
}

TEST(NativePath, SanitizeAbsolute)
{
    // Path is : /tmp/../tmp/file 
    char *sanitized = NativePath::sanitize(TEST_DIR ".." TEST_DIR TEST_FILE);

    ASSERT_STREQ(TEST_DIR TEST_FILE, sanitized);

    free(sanitized);
}

TEST(NativePath, SanitizeRelativeOutsideRoot)
{
    char *sanitized = NativePath::sanitize("file/../../");

    ASSERT_STREQ("../", sanitized);

    free(sanitized);
}

// }}}

// {{{ NativePath without chroot
TEST(NativePath, InvalidAbsoluteFileNoChroot)
{
    NativePath path("/absolute/path/file", false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(NativePath, AbsoluteFileNoChroot)
{
    int fd = open(TEST_ABS_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    close(fd);

    NativePath path(TEST_ABS_FILE, false, false);

    unlink(TEST_ABS_FILE);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_ABS_FILE, path.path());
}

TEST(NativePath, InvalidRelativeFileNoChroot)
{
    NativePath path("file", false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(NativePath, RelativeFileNoChroot)
{
    int fd = open(TEST_REL_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    char testFilePath [PATH_MAX];
    char *testDirPath = nullptr;

    NativePath path(TEST_REL_FILE, false, false);

    close(fd);

    if (realpath(TEST_REL_FILE, testFilePath) == nullptr) {
        ASSERT_FALSE("Failed to get realpath of test file : ## TEST_REL_FILE ## \n");
    }

    unlink(TEST_REL_FILE);

    char *tmp = dirname(strdup(testFilePath));
    int len = strlen(tmp);

    testDirPath = (char *)malloc((len + 2) * sizeof(char));
    strcpy(testDirPath, tmp);
    testDirPath[len] = '/';
    testDirPath[len + 1] = '\0';

    ASSERT_STREQ(testDirPath, path.dir());
    ASSERT_STREQ(testFilePath, path.path());

    free(tmp);
}

TEST(NativePath, HttpNoChroot)
{
    NativePath path(TEST_URL_FILE, false, false);

    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL_FILE, path.path());
}

// }}}

// {{{ cd & chroot (local)
TEST(NativePath, CdLocal)
{
    NativePath::cd(TEST_DIR);

    ASSERT_STREQ(TEST_DIR, NativePath::getPwd());
}

TEST(NativePath, ChrootLocal)
{
    NativePath::chroot(TEST_DIR);

    ASSERT_STREQ(TEST_DIR, NativePath::getRoot());
}
// }}}

// {{{ Relative paths
TEST(NativePath, RelativeFile)
{
    NativePath path(TEST_REL_FILE, false, false);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_DIR TEST_REL_FILE, path.path());
}

TEST(NativePath, RelativeFileUTF8)
{
#define UTF8_FILENAME "♥ Nidium"
    NativePath path(UTF8_FILENAME, false, false);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_DIR UTF8_FILENAME, path.path());
#undef UTF8_FILENAME
}

TEST(NativePath, RelativeFileDot)
{
    NativePath path("./" TEST_REL_FILE, false, false);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_DIR TEST_REL_FILE, path.path());
}

TEST(NativePath, RelativeFileOutChroot)
{
    NativePath path(TEST_REL_OUTSIDE_FILE, false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(NativePath, RelativeFileOutChrootAllowAll)
{
    NativePath path(TEST_REL_OUTSIDE_FILE, true, false);

    ASSERT_STREQ("/", path.dir());
    ASSERT_STREQ("/" TEST_FILE, path.path());
}
// }}}

// {{{ Absolute paths
TEST(NativePath, AbsoluteFile)
{
    NativePath path(TEST_ABS_FILE, false, false);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_DIR TEST_FILE, path.path());
}

TEST(NativePath, AbsoluteFileOutChroot)
{
    NativePath path(TEST_ABS_OUTSIDE_FILE, false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(NativePath, AbsoluteFileOutChrootAllowAll)
{
    NativePath path(TEST_ABS_OUTSIDE_FILE, true, false);

    ASSERT_STREQ(TEST_DIR_OUTSIDE, path.dir());
    ASSERT_STREQ(TEST_DIR_OUTSIDE TEST_FILE, path.path());
}
// }}}

// {{{ Http Paths
TEST(NativePath, WithHttp)
{
    NativePath path(TEST_URL_FILE);

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL_FILE, path.path());
}

TEST(NativePath, WithHttpNoFile)
{
    NativePath path(TEST_URL);

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL, path.path());
}

TEST(NativePath, WithHttpComplex)
{
    NativePath path(TEST_URL "foo/../");

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL, path.path());
}

TEST(NativePath, WithHttpComplex2)
{
    NativePath path(TEST_URL "/foo/bar/../file");

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL "foo/", path.dir());
    ASSERT_STREQ(TEST_URL "foo/file", path.path());
}

TEST(NativePath, WithHttpProtoTooMuchSlashes)
{
    NativePath path("http:///www.nidium.com/");

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL, path.path());
}

TEST(NativePath, WithHttpHostWithoutTrailingSlash)
{
    NativePath path("http://www.nidium.com");

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL, path.path());
}

TEST(NativePath, WithInvalidHttp)
{
    NativePath path(TEST_URL "foo/../../");

    ASSERT_STREQ(nullptr, path.host());
    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());

}
// }}}

// {{{ Prefixed paths
TEST(NativePath, SanitizePrefixFile)
{
    NativePath path("nvfs://" TEST_FILE);

    ASSERT_STREQ("/" TEST_FILE, path.path());
    ASSERT_STREQ("/", path.dir());
}

TEST(NativePath, SanitizePrefixDir)
{
    NativePath path("nvfs://" TEST_DIR);

    ASSERT_STREQ(TEST_DIR, path.path());
    ASSERT_STREQ(TEST_DIR, path.dir());
}

TEST(NativePath, SanitizePrefixUser)
{
    NativePath path("user://foo/bar");

    ASSERT_STREQ(USER_STREAM_BASE_DIR "foo/", path.dir());
    ASSERT_STREQ(USER_STREAM_BASE_DIR "foo/bar", path.path());
}

TEST(NativePath, SanitizePrefixUserOutside)
{
    NativePath path("user://foo/../../");

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}
// }}}

// {{{ cd & chroot (remote with directory)
TEST(NativePath, CdRemote)
{
    NativePath::cd(TEST_URL_DIR);

    ASSERT_STREQ(TEST_URL_DIR, NativePath::getPwd());
}

TEST(NativePath, ChrootRemote)
{
    NativePath::chroot(TEST_URL_DIR);

    ASSERT_STREQ(TEST_URL_DIR, NativePath::getRoot());
}
// }}}

// {{{ NativePath with remote chroot in a directory
TEST(NativePath, RelativeRemote)
{
    NativePath path(TEST_FILE, false, false);

    ASSERT_STREQ(TEST_URL_DIR, path.dir());
    ASSERT_STREQ(TEST_URL_DIR TEST_FILE, path.path());
}

TEST(NativePath, FilePrefixWithRemoteRoot)
{
    NativePath path("file://foo/bar", false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(NativePath, AbsolutePathRemoteRoot)
{
    NativePath path("/foo/bar", false, false);

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL "foo/", path.dir());
    ASSERT_STREQ(TEST_URL "foo/bar", path.path());
}

TEST(NativePath, AbsoluteOtherHost)
{
    NativePath path("http://www.foo.com/dir/file", false, false);

    ASSERT_STREQ("www.foo.com", path.host());
    ASSERT_STREQ("http://www.foo.com/dir/", path.dir());
    ASSERT_STREQ("http://www.foo.com/dir/file", path.path());
}

TEST(NativePath, RelativeRemoteOutsideChroot)
{
    NativePath path("../../", false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}
// }}}

// {{{ cd & chroot (remote "/")
TEST(NativePath, CdRemoteSlash)
{
    NativePath::cd(TEST_URL);

    ASSERT_STREQ(TEST_URL, NativePath::getPwd());
}

TEST(NativePath, ChrootRemoteSlash)
{
    NativePath::chroot(TEST_URL);

    ASSERT_STREQ(TEST_URL, NativePath::getRoot());
}
// }}}

// {{{ NativePath with remote chroot on /
TEST(NativePath, RelativeRemoteSlash)
{
    NativePath path(TEST_FILE, false, false);

    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL TEST_FILE, path.path());
}

TEST(NativePath, AbsolutePathRemoteRootSlash)
{
    NativePath path("/foo/bar", false, false);

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL "foo/", path.dir());
    ASSERT_STREQ(TEST_URL "foo/bar", path.path());
}
// }}}
