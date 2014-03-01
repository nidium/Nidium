#include <jsoncpp.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <native_netlib.h>
#include <sys/stat.h>

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
            {
#define APP_READ_SIZE (1024L*1024L*2)
                struct zip_file *zfile;
                
                zfile = zip_fopen_index(app->fZip, app->action.u32,
                    ZIP_FL_UNCHANGED);

                if (zfile == NULL) {
                    break;
                }

                char *content = (char *)malloc(sizeof(char) * APP_READ_SIZE);
                size_t total = 0;
                int r = 0;
                
                while ((r = zip_fread(zfile, content, APP_READ_SIZE)) >= 0) {
                    total += r;
                    app->actionExtractRead(content, r, total, app->action.u64);
                    if (r != APP_READ_SIZE) {
                        break;
                    }
                }
                zip_fclose(zfile);
                free(content);
#undef APP_READ_SIZE
                break;
            }
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
#define MAX_MSG_IN_ROW 32
    NativeApp *app = (NativeApp *)arg;
    struct NativeApp::native_app_msg *ptr;
    int nread = 0;

    NativeSharedMessages::Message *msg;

    while (++nread < MAX_MSG_IN_ROW && (msg = app->messages->readMessage())) {
        switch (msg->event()) {
            case NativeApp::APP_MESSAGE_READ:
                ptr = static_cast<struct NativeApp::native_app_msg *>(msg->dataPtr());
                ptr->cb(ptr->data, ptr->len, ptr->offset, ptr->total, ptr->user);
                free(ptr->data);
                delete ptr;
                delete msg;
                break;
            default:break;
        }
    }

    return 1;
#undef MAX_MSG_IN_ROW
}

void NativeApp::actionExtractRead(const char *buf, int len,
    size_t offset, size_t total)
{
    struct native_app_msg *msg = new struct native_app_msg;
    msg->data = (char *)malloc(len);
    msg->len  = len;
    msg->total = total;
    msg->offset = offset;
    msg->cb = (NativeAppExtractCallback)action.cb;
    msg->user = action.user;

    memcpy(msg->data, buf, len);

    messages->postMessage(msg, APP_MESSAGE_READ);
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

    if ((numFiles = zip_get_num_entries(fZip, ZIP_FL_UNCHANGED)) == -1 ||
        numFiles == 0) {
        
        zip_close(fZip);
        fZip = NULL;

        return 0;
    }

    return this->loadManifest();
}

struct NativeExtractor_s
{
    NativeApp *app;
    uint64_t curIndex;
    const char *fName;
    char *fDir;
    void (*done)(void *, const char *path);
    void *closure;
    struct {
        size_t len;
        size_t offset;
        FILE *fp;
    } data;
};

static bool NativeExtractor(const char * buf,
    int len, size_t offset, size_t total, void *user)
{
    struct NativeExtractor_s *arg = (struct NativeExtractor_s *)user;

    /* First call (open the file) */
    if (arg->data.offset == 0) {

        char *fpath = (char *)malloc(sizeof(char) *
                        (strlen(arg->fDir) + strlen(arg->fName) + 1));

        sprintf(fpath, "%s%s", arg->fDir, arg->fName);

        if ((arg->data.fp = fopen(fpath, "w")) == NULL) {
            free(fpath);
            return false;
        }

        free(fpath);
    }

    if (fwrite(buf, 1, len, arg->data.fp) != len) {
        /* TODO: handle error */
    }
    arg->data.offset += len;

    /* File extracting finished */
    if (offset == total) {
        if (arg->data.fp) {
            fclose(arg->data.fp);
        }

        if (arg->curIndex < (arg->app->numFiles-1)) {
            arg->data.offset = 0;
            arg->data.fp = NULL;

            /* skip directories */
            while (++arg->curIndex) {
                arg->fName = zip_get_name(arg->app->fZip,
                    arg->curIndex, ZIP_FL_UNCHANGED);

                if (arg->fName[strlen(arg->fName)-1] != '/') {
                    break;
                }
            }

            /* Extract next file */
            arg->data.len = arg->app->extractFile(arg->fName,
                                NativeExtractor, arg);

            return true;
        }

        if (arg->done != NULL) {
            arg->done(arg->closure, arg->fDir);
        }
        /* Complete extracting finished */
        free(arg->fDir);
        delete arg;

        return false;
    }

    return true;
} 

int NativeApp::extractApp(const char *path,
        void (*done)(void *, const char *), void *closure)
{
    char *fullpath = strdup(path);

    if (path[strlen(path)-1] != '/') {
        printf("extractApp : invalid path (non / terminated)\n");
        return 0;
    }

    if (fZip == NULL || !workerIsRunning) {
        printf("extractApp : you need to call open() and runWorker() before\n");
        return 0;
    }
#if 0
#define NATIVE_CACHE_DIR "./cache/"
    if (mkdir(NATIVE_CACHE_DIR, 0777) == -1 && errno != EEXIST) {
        printf("Cant create cache directory\n");
        return 0;
    }
    fullpath = (char *)malloc(sizeof(char) *
                (strlen(path) + sizeof(NATIVE_CACHE_DIR) + 16));

    sprintf(fullpath, NATIVE_CACHE_DIR "%s", path);

#undef NATIVE_CACHE_DIR
#endif
    int ret = 0;
    if ((ret = mkdir(fullpath, 0777)) == -1 && errno != EEXIST) {
        printf("Cant create Application directory\n");
        free(fullpath);
        return 0;
    } else if (ret == -1 && errno == EEXIST) {
        printf("cache for this app already exists\n");
        done(closure, fullpath);
        free(fullpath);
        return 0;
    }

    int i, first = -1;

    /* Create all directories first */
    for (i = 0; i < numFiles; i++) {
        const char *fname = zip_get_name(fZip, i, ZIP_FL_UNCHANGED);

        if (fname[strlen(fname)-1] == '/') {
            char *create = (char *)malloc(sizeof(char) *
                (strlen(fullpath) + strlen(fname) + 8));
            sprintf(create, "%s%s", fullpath, fname);

            mkdir(create, 0777);
            free(create);
            continue;
        }
        /* extractFile doesnt accept directory -- check for the first file */
        if (first == -1) {
            first = i;
        }
    }
    if (first == -1) {
        return 0;
    }
    struct NativeExtractor_s *arg = new NativeExtractor_s;
    arg->app = this;
    arg->curIndex = first;
    arg->fName = zip_get_name(fZip, first, ZIP_FL_UNCHANGED);
    arg->data.offset = 0;
    arg->data.fp = NULL;
    arg->fDir = fullpath;
    arg->done = done;
    arg->closure = closure;
    arg->data.len = this->extractFile(arg->fName, NativeExtractor, arg);

    return (arg->data.len != 0);
}

uint64_t NativeApp::extractFile(const char *file, NativeAppExtractCallback cb, void *user)
{
    if (fZip == NULL || !workerIsRunning) {
        printf("extractFile : you need to call open() and runWorker() before\n");
        return 0;
    }

    uint64_t index;
    struct zip_file *zfile;
    struct zip_stat stat;

    if ((index = zip_name_locate(fZip, file, 0)) == -1 ||
        strcmp(zip_get_name(fZip, index, ZIP_FL_UNCHANGED), file) != 0 ||
        (zfile = zip_fopen_index(fZip, index, ZIP_FL_UNCHANGED)) == NULL) {

        printf("extractFile: Failed to open %s\n", file);
        return 0;
    }

    zip_stat_init(&stat);

    if (zip_stat_index(fZip, index, ZIP_FL_UNCHANGED, &stat) == -1 ||
       !(stat.valid & (ZIP_STAT_SIZE|ZIP_STAT_COMP_SIZE))) {
        zip_fclose(zfile);
        return 0;
    }

    pthread_mutex_lock(&threadMutex);
    if (action.active) {
        pthread_mutex_unlock(&threadMutex);
        printf("extractFile: Worker already working...\n");
        zip_fclose(zfile);
        return 0;
    }

    action.active = true;
    action.type = APP_ACTION_EXTRACT;
    action.ptr  = strdup(file);
    action.u32  = index;
    action.u64  = stat.size;
    action.cb = (void *)cb;
    action.user = user;
    pthread_cond_signal(&threadCond);
    pthread_mutex_unlock(&threadMutex);
    zip_fclose(zfile);

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
        zip_fclose(manifest);
        return 0;
    }

    if (stat.size > (1024L * 1024L)) {
        printf("Manifest file too big\n");
        zip_fclose(manifest);
        return 0;
    }

    content = (char *)malloc(sizeof(char) * stat.size);
    int r = 0;
    if ((r = zip_fread(manifest, content, stat.size)) == -1) {
        zip_fclose(manifest);
        free(content);
        return 0;
    }
    zip_fclose(manifest);

    Json::Value root;

    if (!reader.parse(content, content+stat.size, root)) {
        printf("Cant parse JSON\n");

        return 0;
    }
    MPROP(root, "info", Object, info);
    MPROP(info, "title", String, title);
    MPROP(info, "uid", String, uid);
    MPROP(info, "width", Int, width);
    MPROP(info, "height", Int, height);

    this->appInfos.title = title;
    this->appInfos.udid = uid;
    this->appInfos.width = width.asInt();
    this->appInfos.height = height.asInt();

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
    if (fZip) {
        zip_close(this->fZip);
    }

    free(path);
}
