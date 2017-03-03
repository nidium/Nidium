/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Port/MSWindows.h"

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
namespace Nidium {
namespace Port {

} // namespace Port
} // namespace Nidium
