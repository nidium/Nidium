/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_systeminterface_h__
#define interface_systeminterface_h__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace Nidium {
namespace Interface {

class System;

class SystemInterface
{
public:
    enum AlertType
    {
        ALERT_WARNING = 0,
        ALERT_INFO    = 1,
        ALERT_CRITIC  = 2
    };

    virtual float backingStorePixelRatio() = 0;
    virtual void openURLInBrowser(const char *url)
    {
        return;
    }
    virtual const char *execute(const char *cmd)
    {
        return NULL;
    }
    virtual const char *getCacheDirectory() = 0;
    virtual const char *getEmbedDirectory() = 0;
    virtual const char *getUserDirectory()
    {
        return "~/";
    };
    virtual void alert(const char *message, AlertType type = ALERT_INFO) = 0;
    virtual const char *cwd() = 0;
    virtual const char *getLanguage() = 0;
    virtual void sendNotification(const char *title,
                                  const char *content,
                                  bool sound = false){};
    static SystemInterface *GetInstance()
    {
        return SystemInterface::_interface;
    }
    static SystemInterface *_interface;
    virtual ~SystemInterface(){};

    virtual void print(const char *buf)
    {
        fwrite(buf, 1, strlen(buf), stdout);
    }

private:
    void operator=(System const &);

protected:
    float m_fBackingStorePixelRatio;
};

} // namespace Interface
} // namespace Nidium

#endif
