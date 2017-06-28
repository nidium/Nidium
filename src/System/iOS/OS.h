/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef system_ios_os_h__
#define system_ios_os_h__

#include "System/OS.h"

namespace Nidium {
namespace System {
namespace iOS {

class OS : public System::OS {
public:
	const char *getTempPath() override;
};

}
}
}

#endif