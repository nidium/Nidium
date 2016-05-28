/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "unittest.h"

#include <jsapi.h>

#include "System.h"

unsigned long _ape_seed = 31415961;

namespace Nidium {
    namespace Interface {
        class SystemInterface;

        SystemInterface *SystemInterface::_interface = new System();
    }
}

