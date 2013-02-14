#include <json/json.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <native_netlib.h>

#include "NativeApp.h"
#include "NativeJS.h"
#include "NativeSharedMessages.h"

#define NATIVE_MANIFEST "manifest.json"

NativeApp::NativeApp(const char *path) :
    messages(NULL), fZip(NULL), numFiles(0), workerIsRunning(false)
{
    this->path = strdup(path);
}

static void *native_appworker_thread(void *arg)
{
    NativeApp *app = (NativeApp *)arg;

    printf("Starting NativeApp worker...\n");
    
    while (!app->action.stop) {
        pthread_mutex_lock(&app->threadMutex);

        while (!app->action.active && !app->action.stop) {
            pthread_cond_wait(&app->threadCond, &app->threadMutex);
        }
        if (app->action.stop) {
            pthread_mutex_unlock(&app->threadMutex);
            return NULL;
        }

        switch (app->action.type) {
            case NativeApp::APP_ACTION_EXTRACT:
                printf("Request extracting file : %s at index %d\n", app->action.ptr, app->action.u32);
                break;
            default:
                printf("unknown action\n");
                break;
        }
        app->action.active = false;
        pthread_mutex_unlock(&app->threadMutex);
    }
    printf("Thread ended 2\n");
    return NULL;
}

static int Native_handle_app_messages(void *arg)
{
    NativeApp *app = (NativeApp *)arg;

    NativeSharedMessages::Message msg;

    while (app->messages->readMessage(&msg)) {
        switch (msg.event()) {

            default:break;
        }
    }

    return 1;
}

void NativeApp::runWorker(ape_global *net)
{
    messages = new NativeSharedMessages();

    this->net = net;

    this->action.active = false;
    this->action.stop = false;

    timer = add_timer(&net->timersng, 1,
        Native_handle_app_messages, this);

    pthread_mutex_init(&threadMutex, NULL);
    pthread_cond_init(&threadCond, NULL);

    this->workerIsRunning = true;

    pthread_create(&threadHandle, NULL, native_appworker_thread, this);

    pthread_mutex_lock(&threadMutex);
        pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);    
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

uint64_t NativeApp::extractFile(const char *file)
{
    if (fZip == NULL || !workerIsRunning) {
        printf("extractFile : you need to call open() and runWorker() before\n");
        return 0;
    }

    int index;
    struct zip_file *zfile;
    struct zip_stat stat;

    if ((index = zip_name_locate(fZip, file, ZIP_FL_NODIR)) == -1 ||
        strcmp(zip_get_name(fZip, index, ZIP_FL_UNCHANGED), file) != 0 ||
        (zfile = zip_fopen_index(fZip, index, ZIP_FL_UNCHANGED)) == NULL) {

        printf("extractFile: Failed to open %s\n", file);
        return 0;
    }

    zip_stat_init(&stat);

    if (zip_stat_index(fZip, index, ZIP_FL_UNCHANGED, &stat) == -1 ||
       !(stat.valid & (ZIP_STAT_SIZE|ZIP_STAT_COMP_SIZE))) {
        return 0;
    }

    pthread_mutex_lock(&threadMutex);
    if (action.active) {
        pthread_mutex_unlock(&threadMutex);
        printf("extractFile: Worker already working...\n");
        return 0;
    }

    action.active = true;
    action.type = APP_ACTION_EXTRACT;
    action.ptr  = strdup(file);
    action.u32  = index;
    pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);

    return stat.size;
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

        printf(NATIVE_MANIFEST " not found\n");
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

        return 0;
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
    if (workerIsRunning) {
        action.stop = true;

        pthread_mutex_lock(&threadMutex);
        pthread_cond_signal(&threadCond);
        pthread_mutex_unlock(&threadMutex);

        pthread_join(threadHandle, NULL);
        del_timer(&this->net->timersng, this->timer);
    }

    free(path);
}
