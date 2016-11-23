#ifndef macros_h__
#define macros_h__

/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include <string.h>

#include "Interface/UIInterface.h"
#include "Macros.h"

namespace Nidium {
namespace Interface {
extern UIInterface *__NidiumUI;
}
}

#define NUI_LOG(format, ...)                                 \
    Nidium::Interface::__NidiumUI->getNidiumContext()->vlog( \
        "[%s:%d] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)
#endif
