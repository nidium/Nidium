/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "System/iOS/OS.h"
#import <Foundation/Foundation.h>

namespace Nidium {
namespace System {

OS *OS::_instance = new iOS::OS();

namespace iOS {

const char *OS::getTempPath()
{
    return NSTemporaryDirectory().UTF8String;
}


}}}
