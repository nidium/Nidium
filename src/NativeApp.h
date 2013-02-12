#ifndef nativeapp_h__
#define nativeapp_h__

#include "zip.h"

class NativeJS;
class JSObject;

class NativeApp
{
public:
    char *path;
    
    NativeApp(const char *path);
    int open(NativeJS *njs);
    ~NativeApp();

    const char *getTitle() const {
        return this->appInfos.title;
    }
    const char *getUDID() const {
        return this->appInfos.udid;
    }

private:
    struct zip *fZip;
    int numFiles;
    JSObject *manifestObj;

    int loadManifest(NativeJS *njs);

    struct {
        char *title;
        char *udid;
    } appInfos;
};

#endif
