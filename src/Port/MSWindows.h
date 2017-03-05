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

//http://stackoverflow.com/questions/33696092/whats-the-correct-replacement-for-posix-memalign-in-windows
#define posix_memalign(p, a, s) (((*(p)) = _aligned_malloc((s), (a))), *(p) ?0 :errno)
#define posix_memalign_free _aligned_free

//http ://stackoverflow.com/questions/40159892/using-asprintf-on-windows
char* strsep(char** stringp, const char* delim);
//http://stackoverflow.com/questions/6062822/whats-wrong-with-strndup
char *strndup(char *str, size_t chars);
int vasprintf(char **strp, const char *fmt, va_list ap);
int asprintf(char **strp, const char *fmt, ...);

namespace Nidium {
namespace Port {

}
}

#endif

