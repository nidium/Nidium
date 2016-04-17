/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdlib.h>
#include <string.h>

#include "unittest.h"

#include <Core/NativeEvents.h>

TEST(NativeEvents, Simple)
{
    NativeEvents *e;

    e = new NativeEvents();

    delete e;
}

