/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef system_macos_os_h__
#define system_macos_os_h__

#include "System/OS.h"

namespace Nidium {
namespace System {
namespace macOS {

class OS : public System::OS {
public:
	OS();
	~OS();
	const char *getTempPath() override;
	const char *getNidiumEmbedPath() override;
};

}
}
}

#endif