#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <NativeNFS.h>
#include <NativeStreamInterface.h>
#include <NativeFileStream.h>
#include <NativeUtils.h>
#include <string>
#include <native_netlib.h>
//clang++  main.cpp ../../nativejscore/external/json/jsoncpp.cpp ../../nativejscore/network/gyp/build/Release/libnativenetwork.a ../../nativejscore/gyp/build/Release/libnativejscore.a ../../out/third-party-libs/.libs/libhttp_parser.a ../../out/third-party-libs/.libs/libnspr4.a ../../out/third-party-libs/.libs/libcares.a ../../out/third-party-libs/release/libjs_static.a ../../out/third-party-libs/release/libzip.a -I../../nativejscore/ -I../../nativejscore/network/ -I../../nativejscore/external/json/  -lz -lssl -lcrypto

unsigned long _ape_seed;

void listdir(NativeNFS *nfs, DIR *dir, std::string fullpath, int strip)
{
    dirent *cur;

    if (dir == NULL) {
        fprintf(stderr, "null dir given\n");
        return;
    }

    while ((cur = readdir(dir)) != NULL) {
        std::string newpath = fullpath + "/" + cur->d_name;
        const char *vpath = &newpath.c_str()[strip];

        if (cur->d_type & DT_DIR) {
            if (strcmp(cur->d_name, ".") == 0 || strcmp(cur->d_name, "..") == 0) {
                continue;
            }

            if (!nfs->mkdir(vpath, strlen(vpath))) {
                fprintf(stderr, "Failed to create dir %s\n", vpath);
                continue;
            }

            listdir(nfs, opendir(newpath.c_str()), newpath, strip);
        } else if (cur->d_type & DT_REG) {

            NativePtrAutoDelete<NativeBaseStream *> stream(NativeBaseStream::create(newpath.c_str()));

            if (stream.ptr() == NULL) {
                fprintf(stderr, "Could not create stream for file %s\n", newpath.c_str());
                continue;
            }
            char *content;
            size_t len;

            if (!stream.ptr()->getContentSync(&content, &len, true)) {
                fprintf(stderr, "Could not read stream for file %s\n", newpath.c_str());
                continue;
            }

            if (!nfs->writeFile(vpath, strlen(vpath), content, len)) {
                fprintf(stderr, "Failed to write file %s\n", vpath);
                continue;
            }

            fprintf(stderr, "Saved file : %s\n", vpath);
        }
    }

    closedir(dir);
}

void initNativeJSCore()
{
    _ape_seed = time(NULL) ^ (getpid() << 16);

    /*
        This is required to create a stream (file is the default)
    */
    NativePath::registerScheme(SCHEME_DEFINE("file://", NativeFileStream, false), true);
    NativeTaskManager::createManager();
    ape_global *gnet = native_netlib_init();
    NativeMessages::initReader(gnet);

}

int main(int argc, char **argv)
{
    initNativeJSCore();

    DIR *dir = opendir(argv[1]);
    if (!dir) {
        fprintf(stderr, "Cant open dir %s\n", argv[1]);
    }
    NativeNFS *nfs = new NativeNFS();

    listdir(nfs, dir, argv[1], strlen(argv[1]));

    nfs->save(stdout);

    return 0;
}