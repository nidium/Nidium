#include "NativeApp.h"
#include "NativeJS.h"
#include <jsapi.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define NATIVE_MANIFEST "manifest.json"

NativeApp::NativeApp(const char *path) :
    fZip(NULL), numFiles(0)
{
    this->path = strdup(path);
}

int NativeApp::open(NativeJS *njs)
{
    int err = 0;
    fZip = zip_open(path, ZIP_CHECKCONS, &err);

    if (err != ZIP_ER_OK || fZip == NULL) {
        char buf_erreur[1024];
        zip_error_to_str(buf_erreur, sizeof buf_erreur, err, errno);
        printf("Failed to open zip file (%d) : %s\n", err, buf_erreur);
        return 0;
    }

    if ((numFiles = zip_get_num_entries(fZip, ZIP_FL_UNCHANGED)) == -1) {
        zip_close(fZip);
        fZip = NULL;

        return 0;
    }

    for (int i = 0; i < numFiles; i++) {
        printf("File : %s\n", zip_get_name(fZip, i, ZIP_FL_UNCHANGED));
    }

    return this->loadManifest(njs);
}

int NativeApp::loadManifest(NativeJS *njs)
{
#define MPROP(str, obj, type) \
if (JS_GetProperty(njs->cx, obj, str, &dval) == JS_FALSE || \
    JSVAL_IS_VOID(dval) || !dval.is ## type()) { \
    njs->unrootObject(obj); \
    printf("Manifest error : " str " property not found or wrong type\n"); \
    return 0; \
}
    if (fZip == NULL) return 0;

    int index;
    jsval dval;
    char *content;
    struct zip_file *manifest;
    struct zip_stat stat;

    if ((index = zip_name_locate(fZip, NATIVE_MANIFEST, ZIP_FL_NODIR)) == -1 ||
        strcmp(zip_get_name(fZip, index, ZIP_FL_UNCHANGED), NATIVE_MANIFEST) != 0 ||
        (manifest = zip_fopen_index(fZip, index, ZIP_FL_UNCHANGED)) == NULL) {

        printf("manifest.json not found\n");
        return 0;
    }

    zip_stat_init(&stat);

    if (zip_stat_index(fZip, index, ZIP_FL_UNCHANGED, &stat) == -1 ||
       !(stat.valid & (ZIP_STAT_SIZE|ZIP_STAT_COMP_SIZE))) {
        return 0;
    }

    if (stat.size > (1024L * 1024L)) {
        printf("Manifest file too big\n");
        return 0;
    }

    content = (char *)malloc(sizeof(char) * stat.size);
    int r = 0;
    if ((r = zip_fread(manifest, content, stat.size)) == -1) {
        free(content);
        return 0;
    }

    size_t len = 0;
    if (JS_DecodeBytes(njs->cx, content, r, 0, &len) == JS_FALSE) {
        free(content);
        return 0;
    }

    jschar *res = (jschar *)malloc(sizeof(jschar) * len);

    if (JS_DecodeBytes(njs->cx, content, r, res, &len) == JS_FALSE) {
        free(content);
        free(res);
        return 0;
    }

    jsval objManifest;
    if (JS_ParseJSON(njs->cx, res, len, &objManifest) == JS_FALSE) {
        printf("Cant parse JSON\n");
        free(content);
        free(res);
        return 0;
    }

    free(content);

    this->manifestObj = JSVAL_TO_OBJECT(objManifest);

    njs->rootObjectUntilShutdown(this->manifestObj);

    MPROP("info", this->manifestObj, Object);
    MPROP("title", JSVAL_TO_OBJECT(dval), String);
    JSAutoByteString title(njs->cx, JSVAL_TO_STRING(dval));
    this->appInfos.title = strdup(title.ptr());

    MPROP("info", this->manifestObj, Object);
    MPROP("uid", JSVAL_TO_OBJECT(dval), String);
    JSAutoByteString uid(njs->cx, JSVAL_TO_STRING(dval));
    this->appInfos.udid = strdup(uid.ptr());

    return 1;
}

NativeApp::~NativeApp()
{
    free(this->appInfos.title);
    free(this->appInfos.udid);
    free(path);
}
