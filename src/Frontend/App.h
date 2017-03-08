/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef frontend_app_h__
#define frontend_app_h__

#include <zip.h>
#include <json/json.h>

#include <ape_timers_next.h>

#include "Binding/NidiumJS.h"

namespace Nidium {
namespace Frontend {

typedef bool (*AppExtractCallback)(
    const char *buf, int len, size_t offset, size_t total, void *user);

class App
{
public:
    char *m_Path;

    App(const char *path);
    int open();
    void runWorker(ape_global *net);
    uint64_t extractFile(const char *path, AppExtractCallback cb, void *user);
    int extractApp(const char *path,
                   void (*done)(void *, const char *),
                   void *closure);
    ~App();

    const char *getTitle() const
    {
        return m_AppInfos.title.asCString();
    }
    const char *getUDID() const
    {
        return m_AppInfos.udid.asCString();
    }
    int getWidth() const
    {
        return m_AppInfos.width;
    }
    int getHeight() const
    {
        return m_AppInfos.height;
    }
    enum ACTION_TYPE
    {
        APP_ACTION_EXTRACT
    };

    struct
    {
        uint64_t u64;
        void *ptr;
        void *cb;
        void *user;
        enum ACTION_TYPE type;
        bool active;
        bool stop;
        uint32_t u32;
        uint8_t u8;
    } m_Action;

    pthread_t m_ThreadHandle;
    pthread_mutex_t m_ThreadMutex;
    pthread_cond_t m_ThreadCond;

    Core::SharedMessages *m_Messages;
    struct zip *m_fZip;

    void
    actionExtractRead(const char *buf, int len, size_t offset, size_t total);

    struct app_msg
    {
        size_t total;
        size_t offset;
        char *data;
        AppExtractCallback cb;
        void *user;
        int len;
    };

    enum APP_MESSAGE
    {
        APP_MESSAGE_READ
    };
    int m_NumFiles;

private:
    Json::Reader m_Reader;
    bool m_WorkerIsRunning;

    int loadManifest();

    struct
    {
        Json::Value title;
        Json::Value udid;

        int width;
        int height;
    } m_AppInfos;

    ape_timer_t *m_Timer;
    ape_global *m_Net;
};

} // namespace Frontend
} // namespace Nidium

#endif
