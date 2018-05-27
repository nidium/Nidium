/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef io_file_h__
#define io_file_h__

#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace Nidium {
namespace IO {
namespace FileSystem {

    static void mkdirp(const char *path)
    {

        char tmp[PATH_MAX];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp), "%s", path);

        len = strlen(tmp);

        if (tmp[len - 1] == '/') {
            tmp[len - 1] = 0;
        }

        for (p = tmp + 1; *p; p++) {

            if (*p == '/') {
                *p = 0;
                mkdir(tmp, S_IRWXU);
                *p = '/';
            }

        }

        mkdir(tmp, S_IRWXU);

    }


} // namespace FileSystem
} // namespace IO
} // namespace Nidium

#endif
