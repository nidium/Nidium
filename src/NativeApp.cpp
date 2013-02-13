#include "NativeApp.h"
#include "NativeJS.h"

#include <json/json.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define NATIVE_MANIFEST "manifest.json"

NativeApp::NativeApp(const char *path) :
    fZip(NULL), numFiles(0)
{
    this->path = strdup(path);
}

int NativeApp::open()
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

    return this->loadManifest();
}

int NativeApp::loadManifest()
{
#define MPROP(root, str, type, out) \
Json::Value out; \
if (!root.isMember(str) || !(out = root[str]) || !out.is ## type()) { \
    printf("Manifest error : " str " property not found or wrong type (required : " #type ")\n"); \
    return 0; \
}
    if (fZip == NULL) return 0;

    int index;
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

    Json::Value root;

    if (!reader.parse(content, content+stat.size, root)) {
        printf("Cant parse JSON\n");
    }

    MPROP(root, "info", Object, info);
    MPROP(info, "title", String, title);
    MPROP(info, "uid", String, uid);

    this->appInfos.title = title;
    this->appInfos.udid = uid;

    return 1;
}

NativeApp::~NativeApp()
{
    free(path);
}
