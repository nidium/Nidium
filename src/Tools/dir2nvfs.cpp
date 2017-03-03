/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <jsapi.h>
#include <ape_netlib.h>

#include "Core/Utils.h"
#include "Core/Context.h"
#include "IO/FileStream.h"
#include "Binding/JSNFS.h"

#ifndef DIR2NFS_OUTPUT
#define DIR2NFS_OUTPUT stdout
#endif

using Nidium::Core::Path;
using Nidium::Core::Messages;
using Nidium::Core::TaskManager;
using Nidium::IO::Stream;
using Nidium::IO::FileStream;
using Nidium::Binding::JSNFS;

unsigned long _ape_seed;

namespace Nidium {
namespace Tools {

void listdir(JSNFS *nfs, DIR *dir, std::string fullpath, int strip)
{
    dirent *cur;

    if (dir == NULL) {
        ndm_log(NDM_LOG_ERROR, "Tools", "null dir given");
        return;
    }

    while ((cur = readdir(dir)) != NULL) {
        std::string newpath = fullpath + "/" + cur->d_name;
        const char *vpath   = &newpath.c_str()[strip];

        if (cur->d_type & DT_DIR) {
            if (strcmp(cur->d_name, ".") == 0
                || strcmp(cur->d_name, "..") == 0) {
                continue;
            }

            if (!nfs->mkdir(vpath, strlen(vpath))) {
                ndm_logf(NDM_LOG_ERROR, "Tools", "Failed to create dir %s", vpath);
                continue;
            }

            listdir(nfs, opendir(newpath.c_str()), newpath, strip);
        } else if (cur->d_type & DT_REG) {

            // PtrAutoDelete<Stream *> stream(Stream::Create(newpath.c_str()));
            Stream *stream = Stream::Create(newpath.c_str());

            if (stream == NULL) {
                ndm_logf(NDM_LOG_ERROR, "Tools", "Could not create stream for file %s",
                        newpath.c_str());
                continue;
            }
            char *content;
            size_t len;

            if (!stream->getContentSync(&content, &len, true)) {
                ndm_logf(NDM_LOG_ERROR, "Tools", "Could not read stream for file %s",
                        newpath.c_str());
                continue;
            }

            if (!nfs->writeFile(vpath, strlen(vpath), content, len)) {
                ndm_logf(NDM_LOG_ERROR, "Tools", "Failed to write file %s", vpath);
                continue;
            }

            ndm_logf(NDM_LOG_ERROR, "Tools", "Saved file : %s", vpath);
        }
    }

    closedir(dir);
}

static Core::Context *initNidiumJS()
{
    _ape_seed       = time(NULL) ^ (getpid() << 16);
    ape_global *net = APE_init();
    return new Core::Context(net);
}

static int Embed(int argc, char **argv)
{
    Core::Context *ncx = initNidiumJS();
    JSContext *cx      = ncx->getNJS()->getJSContext();

    if (argc <= 1) {
        ndm_logf(NDM_LOG_ERROR, "Tools", "%s <path> [prefix] [> out]", argv[0]);
        return 1;
    }

    DIR *dir = opendir(argv[1]);
    if (!dir) {
        ndm_logf(NDM_LOG_ERROR, "Tools", "Cannot open dir %s", argv[1]);
    }

    JS_BeginRequest(cx);

    JSNFS *nfs = new JSNFS(cx);

    if (argc == 3) {
        std::string prefix = "/";
        prefix += argv[2];

        ndm_logf(NDM_LOG_DEBUG, "Tools", "Create prefix %s...", prefix.c_str());

        nfs->mkdir(prefix.c_str(), strlen(prefix.c_str()));

        listdir(nfs, dir, argv[1], strlen(argv[1]));
    } else {
        listdir(nfs, dir, argv[1], strlen(argv[1]));
    }

    nfs->save(DIR2NFS_OUTPUT);

    JS_EndRequest(cx);

    delete ncx;

    return 0;
}

} // namespace Tools
} // namespace Nidium

int main(int argc, char **argv)
{
    return Nidium::Tools::Embed(argc, argv);
}
