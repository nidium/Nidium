/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

#include <gtest/gtest.h>

#include <Core/Path.h>
#include <IO/NFSStream.h>
#include <IO/FileStream.h>
#include <Net/HTTPStream.h>

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

using Nidium::Core::Path;

/*
    Class for testing prefixed stream with a base dir different than "/"
*/
class UserStream : public Nidium::IO::FileStream
{
  public:
    explicit UserStream(const char *location) :
        Nidium::IO::FileStream(location)
    {
    }

    static Nidium::IO::Stream *createStream(const char *location) {
        return new UserStream(location);
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

TEST(Path, RegisterScheme)
{
    Path::registerScheme(SCHEME_DEFINE("file://",    Nidium::IO::FileStream,    false), true); // default
    Path::registerScheme(SCHEME_DEFINE("http://",    Nidium::Net::HTTPStream,    true));
    Path::registerScheme(SCHEME_DEFINE("https://",   Nidium::Net::HTTPStream,    true));
    Path::registerScheme(SCHEME_DEFINE("nvfs://",    Nidium::IO::NFSStream,     false));
    Path::registerScheme(SCHEME_DEFINE("user://",    UserStream,    false));
}

// {{{ Sanitize
TEST(Path, Sanitize)
{
    char *sanitized = Path::sanitize("dir/../");

    ASSERT_STREQ("", sanitized);

    free(sanitized);
}

TEST(Path, Sanitize2)
{
    char *sanitized = Path::sanitize("dir//./a/.././file");

    ASSERT_STREQ("dir/file", sanitized);

    free(sanitized);
}

TEST(Path, Sanitize3)
{
    char *sanitized = Path::sanitize("foo/bar/../file");

    ASSERT_STREQ("foo/file", sanitized);

    free(sanitized);
}

TEST(Path, Sanitize4)
{
    char *sanitized = Path::sanitize("dir/..");

    ASSERT_STREQ("", sanitized);

    free(sanitized);
}

TEST(Path, Sanitize5)
{
    bool external = false;
    char *sanitized = Path::sanitize("dir/../..", &external);

    ASSERT_STREQ("../", sanitized);
    ASSERT_TRUE(external);

    free(sanitized);
}

TEST(Path, SanitizeEmpty)
{
    char *sanitized = Path::sanitize("");

    ASSERT_STREQ("", sanitized);

    free(sanitized);
}

TEST(Path, SanitizeDot)
{
    char *sanitized = Path::sanitize(".");

    ASSERT_STREQ("", sanitized);

    free(sanitized);
}

TEST(Path, SanitizeDotSlash)
{
    char *sanitized = Path::sanitize("./");

    ASSERT_STREQ("", sanitized);

    free(sanitized);
}

TEST(Path, SanitizeNull)
{
    char *sanitized = Path::sanitize(nullptr);

    ASSERT_STREQ(nullptr, sanitized);

    free(sanitized);
}

TEST(Path, SanitizeAbsoluteInvalid)
{
    char *sanitized = Path::sanitize(TEST_DIR "../../");

    ASSERT_STREQ(nullptr, sanitized);

    free(sanitized);
}

TEST(Path, SanitizeAbsolute)
{
    // Path is : /tmp/../tmp/file 
    char *sanitized = Path::sanitize(TEST_DIR ".." TEST_DIR TEST_FILE);

    ASSERT_STREQ(TEST_DIR TEST_FILE, sanitized);

    free(sanitized);
}

TEST(Path, SanitizeRelativeOutsideRoot)
{
    char *sanitized = Path::sanitize("file/../../");

    ASSERT_STREQ("../", sanitized);

    free(sanitized);
}

// }}}

// {{{ Path without chroot
TEST(Path, InvalidAbsoluteFileNoChroot)
{
    Path path("/absolute/path/file", false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(Path, AbsoluteFileNoChroot)
{
    int fd = open(TEST_ABS_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    close(fd);

    Path path(TEST_ABS_FILE, false, false);

    unlink(TEST_ABS_FILE);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_ABS_FILE, path.path());
}

TEST(Path, InvalidRelativeFileNoChroot)
{
    Path path("file", false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(Path, RelativeFileNoChroot)
{
    int fd = open(TEST_REL_FILE, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    char testFilePath [PATH_MAX];
    char *testDirPath = nullptr;

    Path path(TEST_REL_FILE, false, false);

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

TEST(Path, HttpNoChroot)
{
    Path path(TEST_URL_FILE, false, false);

    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL_FILE, path.path());
}

// }}}

// {{{ cd & chroot (local)
TEST(Path, CdLocal)
{
    Path::cd(TEST_DIR);

    ASSERT_STREQ(TEST_DIR, Path::getPwd());
}

TEST(Path, ChrootLocal)
{
    Path::chroot(TEST_DIR);

    ASSERT_STREQ(TEST_DIR, Path::getRoot());
}
// }}}

// {{{ Relative paths
TEST(Path, RelativeFile)
{
    Path path(TEST_REL_FILE, false, false);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_DIR TEST_REL_FILE, path.path());
}

TEST(Path, RelativeFileUTF8)
{
#define UTF8_FILENAME "♥ Nidium"
    Path path(UTF8_FILENAME, false, false);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_DIR UTF8_FILENAME, path.path());
#undef UTF8_FILENAME
}

TEST(Path, RelativeFileDot)
{
    Path path("./" TEST_REL_FILE, false, false);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_DIR TEST_REL_FILE, path.path());
}

TEST(Path, RelativeFileOutChroot)
{
    Path path(TEST_REL_OUTSIDE_FILE, false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(Path, RelativeFileOutChrootAllowAll)
{
    Path path(TEST_REL_OUTSIDE_FILE, true, false);

    ASSERT_STREQ("/", path.dir());
    ASSERT_STREQ("/" TEST_FILE, path.path());
}
// }}}

// {{{ Absolute paths
TEST(Path, AbsoluteFile)
{
    Path path(TEST_ABS_FILE, false, false);

    ASSERT_STREQ(TEST_DIR, path.dir());
    ASSERT_STREQ(TEST_DIR TEST_FILE, path.path());
}

TEST(Path, AbsoluteFileOutChroot)
{
    Path path(TEST_ABS_OUTSIDE_FILE, false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(Path, AbsoluteFileOutChrootAllowAll)
{
    Path path(TEST_ABS_OUTSIDE_FILE, true, false);

    ASSERT_STREQ(TEST_DIR_OUTSIDE, path.dir());
    ASSERT_STREQ(TEST_DIR_OUTSIDE TEST_FILE, path.path());
}
// }}}

// {{{ Http Paths
TEST(Path, WithHttp)
{
    Path path(TEST_URL_FILE);

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL_FILE, path.path());
}

TEST(Path, WithHttpNoFile)
{
    Path path(TEST_URL);

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL, path.path());
}

TEST(Path, WithHttpComplex)
{
    Path path(TEST_URL "foo/../");

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL, path.path());
}

TEST(Path, WithHttpComplex2)
{
    Path path(TEST_URL "/foo/bar/../file");

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL "foo/", path.dir());
    ASSERT_STREQ(TEST_URL "foo/file", path.path());
}

TEST(Path, WithHttpProtoTooMuchSlashes)
{
    Path path("http:///www.nidium.com/");

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL, path.path());
}

TEST(Path, WithHttpHostWithoutTrailingSlash)
{
    Path path("http://www.nidium.com");

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL, path.path());
}

TEST(Path, WithInvalidHttp)
{
    Path path(TEST_URL "foo/../../");

    ASSERT_STREQ(nullptr, path.host());
    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());

}
// }}}

// {{{ Prefixed paths
TEST(Path, SanitizePrefixFile)
{
    Path path("nvfs://" TEST_FILE);

    ASSERT_STREQ("/" TEST_FILE, path.path());
    ASSERT_STREQ("/", path.dir());
}

TEST(Path, SanitizePrefixDir)
{
    Path path("nvfs://" TEST_DIR);

    ASSERT_STREQ(TEST_DIR, path.path());
    ASSERT_STREQ(TEST_DIR, path.dir());
}

TEST(Path, SanitizePrefixUser)
{
    Path path("user://foo/bar");

    ASSERT_STREQ(USER_STREAM_BASE_DIR "foo/", path.dir());
    ASSERT_STREQ(USER_STREAM_BASE_DIR "foo/bar", path.path());
}

TEST(Path, SanitizePrefixUserOutside)
{
    Path path("user://foo/../../");

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}
// }}}

// {{{ cd subdirectory of chroot
TEST(Path, CdLocalSubdir)
{
    Path::cd(TEST_DIR "/bar/");

    ASSERT_STREQ(TEST_DIR "/bar/", Path::getPwd());
}

TEST(Path, RelativeFileDifferentCwd)
{
    Path path(TEST_REL_FILE, false, false);

    ASSERT_STREQ(TEST_DIR "bar/", path.dir());
    ASSERT_STREQ(TEST_DIR "bar/" TEST_REL_FILE, path.path());
}
// }}}

// {{{ cd & chroot (remote with directory)
TEST(Path, CdRemote)
{
    Path::cd(TEST_URL_DIR);

    ASSERT_STREQ(TEST_URL_DIR, Path::getPwd());
}

TEST(Path, ChrootRemote)
{
    Path::chroot(TEST_URL_DIR);

    ASSERT_STREQ(TEST_URL_DIR, Path::getRoot());
}
// }}}

// {{{ Path with remote chroot in a directory
TEST(Path, RelativeRemote)
{
    Path path(TEST_FILE, false, false);

    ASSERT_STREQ(TEST_URL_DIR, path.dir());
    ASSERT_STREQ(TEST_URL_DIR TEST_FILE, path.path());
}

TEST(Path, FilePrefixWithRemoteRoot)
{
    Path path("file://foo/bar", false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}

TEST(Path, AbsolutePathRemoteRoot)
{
    Path path("/foo/bar", false, false);

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL "foo/", path.dir());
    ASSERT_STREQ(TEST_URL "foo/bar", path.path());
}

TEST(Path, AbsoluteOtherHost)
{
    Path path("http://www.foo.com/dir/file", false, false);

    ASSERT_STREQ("www.foo.com", path.host());
    ASSERT_STREQ("http://www.foo.com/dir/", path.dir());
    ASSERT_STREQ("http://www.foo.com/dir/file", path.path());
}

TEST(Path, RelativeRemoteOutsideChroot)
{
    Path path("../../", false, false);

    ASSERT_STREQ(nullptr, path.dir());
    ASSERT_STREQ(nullptr, path.path());
}
// }}}

// {{{ cd & chroot (remote "/")
TEST(Path, CdRemoteSlash)
{
    Path::cd(TEST_URL);

    ASSERT_STREQ(TEST_URL, Path::getPwd());
}

TEST(Path, ChrootRemoteSlash)
{
    Path::chroot(TEST_URL);

    ASSERT_STREQ(TEST_URL, Path::getRoot());
}
// }}}

// {{{ Path with remote chroot on /
TEST(Path, RelativeRemoteSlash)
{
    Path path(TEST_FILE, false, false);

    ASSERT_STREQ(TEST_URL, path.dir());
    ASSERT_STREQ(TEST_URL TEST_FILE, path.path());
}

TEST(Path, AbsolutePathRemoteRootSlash)
{
    Path path("/foo/bar", false, false);

    ASSERT_STREQ(TEST_HOST, path.host());
    ASSERT_STREQ(TEST_URL "foo/", path.dir());
    ASSERT_STREQ(TEST_URL "foo/bar", path.path());
}
// }}}
