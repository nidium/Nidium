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

    /*
        Get the current application temporary directory
    */
    virtual const char *getTempPath() {
        return "/tmp/";
    }

    /*
        Get the nidium embeded path
    */
    virtual const char *getNidiumEmbedPath()
    {
        return nullptr;
    }

    /*
        Returns a filedescriptor of a new rw temporary file
    */
    virtual int getTempFd();

    static OS *GetInstance()
    {
        return OS::_instance;
    }
    static OS *_instance;
protected:
    OS(){};
};

}
}

#endif
