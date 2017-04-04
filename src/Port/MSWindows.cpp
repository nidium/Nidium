/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Port/MSWindows.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <varargs.h>
#include <cstring>

char* strsep(char** stringp, const char* delim)
{
    char* result;

    if ((stringp == NULL) || (*stringp == NULL)) {
       return NULL;
    }
    result = *stringp;
    while (**stringp && ! std::strchr(delim, **stringp)) {
       ++*stringp;
    }

    if (**stringp) {
       *(*stringp)++ = '\0';
    } else {
       *stringp = NULL;
    }

    return result;
}


char *strndup(char *str, size_t maxlen)
{
    char *buffer;
    int n;

    buffer = (char *)malloc(maxlen + 1);
    if (buffer) {
        for (n = 0; ((n < maxlen) && (str[n] != 0)); n++) buffer[n] = str[n];
        buffer[n] = 0;
    }

    return buffer;
}

int asprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int r = vasprintf(strp, fmt, ap);
    va_end(ap);
    return r;
}

namespace Nidium {
namespace Port {

} // namespace Port
} // namespace Nidium
