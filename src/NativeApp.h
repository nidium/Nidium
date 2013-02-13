#ifndef nativeapp_h__
#define nativeapp_h__

#include "zip.h"
#include <json/json.h>

class NativeJS;
class JSObject;

class NativeApp
{
public:
    char *path;
    
    NativeApp(const char *path);
    int open();
    ~NativeApp();

    const char *getTitle() const {
        return this->appInfos.title.asCString();
    }
    const char *getUDID() const {
        return this->appInfos.udid.asCString();
    }

private:
    struct zip *fZip;
    Json::Reader reader;
    int numFiles;
    JSObject *manifestObj;

    int loadManifest();

    struct {
        Json::Value title;
        Json::Value udid;
    } appInfos;
};

#endif
