/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "System/macOS/OS.h"
#include <string>
#import <Foundation/Foundation.h>

namespace Nidium {
namespace System {

OS *OS::_instance = new macOS::OS();

namespace macOS {

OS::OS() {
	m_EmbededPath = strdup((std::string([NSBundle mainBundle].resourcePath.UTF8String) + "/Embed/").c_str());
}

const char *OS::getTempPath()
{
    return NSTemporaryDirectory().UTF8String;
}

OS::~OS() {
	if (m_EmbededPath) {
		free(m_EmbededPath);
	}
}

}}}
