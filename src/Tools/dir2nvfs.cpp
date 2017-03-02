/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <prio.h>
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

void listdir(JSNFS *nfs, PRDir *dir, std::string fullpath, int strip)
{
    PRDirEntry *cur;

    if (dir == NULL) {
        fprintf(stderr, "null dir given\n");
        return;
    }

    while ((cur = PR_ReadDir(dir, PR_SKIP_BOTH)) != NULL) {
        std::string newpath = fullpath + "/" + cur->name;
        const char *vpath   = &newpath.c_str()[strip];

        if (cur->d_type & DT_DIR) {
            if (!nfs->mkdir(vpath, strlen(vpath))) {
                fprintf(stderr, "Failed to create dir %s\n", vpath);
                continue;
            }

            listdir(nfs, PR_OpenDir(newpath.c_str()), newpath, strip);
        } else if (cur->d_type & DT_REG) {

            // PtrAutoDelete<Stream *> stream(Stream::Create(newpath.c_str()));
            Stream *stream = Stream::Create(newpath.c_str());

            if (stream == NULL) {
                fprintf(stderr, "Could not create stream for file %s\n",
                        newpath.c_str());
                continue;
            }
            char *content;
            size_t len;

            if (!stream->getContentSync(&content, &len, true)) {
                fprintf(stderr, "Could not read stream for file %s\n",
                        newpath.c_str());
                continue;
            }

            if (!nfs->writeFile(vpath, strlen(vpath), content, len)) {
                fprintf(stderr, "Failed to write file %s\n", vpath);
                continue;
            }

            fprintf(stderr, "Saved file : %s\n", vpath);
        }
    }

    PR_CloseDir(dir);
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
        printf("$ %s <path> [prefix] [> out]\n", argv[0]);
        return 1;
    }

    PRDir *dir = PR_OpenDir(argv[1]);
    if (!dir) {
        fprintf(stderr, "Cant open dir %s\n", argv[1]);
    }

    JS_BeginRequest(cx);

    JSNFS *nfs = new JSNFS(cx);

    if (argc == 3) {
        std::string prefix = "/";
        prefix += argv[2];

        fprintf(stderr, "Create prefix %s...\n", prefix.c_str());

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
