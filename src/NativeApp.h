#ifndef nativeapp_h__
#define nativeapp_h__

#include <json/json.h>
#include <pthread.h>
#include "zip.h"

class NativeSharedMessages;
class NativeJS;

class NativeApp
{
public:
    char *path;
    
    NativeApp(const char *path);
    int open();
    void runWorker(struct _ape_global *net);
    uint64_t extractFile(const char *path);
    ~NativeApp();

    const char *getTitle() const {
        return this->appInfos.title.asCString();
    }
    const char *getUDID() const {
        return this->appInfos.udid.asCString();
    }

    enum ACTION_TYPE {
        APP_ACTION_EXTRACT
    };

    struct {
        uint64_t u64;
        void *ptr;
        enum ACTION_TYPE type;
        bool active;
        bool stop;
        uint32_t u32;
        uint8_t  u8;
    } action;

    pthread_t threadHandle;
    pthread_mutex_t threadMutex;
    pthread_cond_t threadCond;

    NativeSharedMessages *messages;
private:

    struct zip *fZip;
    Json::Reader reader;
    int numFiles;
    bool workerIsRunning;

    int loadManifest();
    

    struct {
        Json::Value title;
        Json::Value udid;
    } appInfos;

    struct _ape_timer *timer;
    struct _ape_global *net;
};

#endif
