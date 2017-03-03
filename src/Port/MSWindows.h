/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef port_mswindows_h__
#define port_mswindows_h__

#include <port/windows.h>

#define ftruncate _chsize
#define usleep Sleep

#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#define MAXPATHLEN _MAX_PATH 

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif


char* strsep(char** stringp, const char* delim);

namespace Nidium {
namespace Port {

}
}

#endif

