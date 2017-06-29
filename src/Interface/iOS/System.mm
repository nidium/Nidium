/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "System.h"
#import <Foundation/Foundation.h>
#include <string>

namespace Nidium {
namespace Interface {

System::System()
{
    m_EmbedPath = strdup((std::string([NSBundle mainBundle].resourcePath.UTF8String) + "/Embed/").c_str());

}

} // namespace Interface
} // namespace Nidium
