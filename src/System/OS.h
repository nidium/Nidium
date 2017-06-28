/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef system_os_h__
#define system_os_h__

namespace Nidium {
namespace System {

class OS {
public:
	virtual const char *getTempPath() {
		return "/tmp/";
	}

    virtual int getTempFd();

    static OS *GetInstance()
    {
        return OS::_instance;
    }
    static OS *_instance;
};

}
}

#endif
