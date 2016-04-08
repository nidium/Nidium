/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NativeServer.h"

int main(int argc, char **argv)
{
    return NativeServer::Start(argc, argv);
}

