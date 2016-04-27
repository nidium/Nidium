#include "Frontend/App.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ape_netlib.h>

#include <Binding/NidiumJS.h>

namespace Nidium {
namespace Frontend {

#define NIDIUM_MANIFEST "manifest.json"

// {{{ Extractor
struct Extractor_s
{
    App *app;
    uint64_t curIndex;
    const char *fName;
    char *fDir;
    void (*done)(void *, const char *m_Path);
    void *closure;
    struct {
        size_t len;
        size_t offset;
        FILE *fp;
    } data;
};

static bool Extractor(const char * buf,
    int len, size_t offset, size_t total, void *user)
{
    struct Extractor_s *arg = (struct Extractor_s *)user;

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

        if (arg->curIndex < (arg->app->m_NumFiles-1)) {
            arg->data.offset = 0;
            arg->data.fp = NULL;

            /* skip directories */
            while (++arg->curIndex) {
                arg->fName = zip_get_name(arg->app->m_fZip,
                    arg->curIndex, ZIP_FL_UNCHANGED);

                if (arg->fName[strlen(arg->fName)-1] != '/') {
                    break;
                }
            }

            /* Extract next file */
            arg->data.len = arg->app->extractFile(arg->fName,
                                Extractor, arg);

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
// }}}

// {{{ App
App::App(const char *path) :
    m_Messages(NULL), m_fZip(NULL), m_NumFiles(0), m_WorkerIsRunning(false), m_Timer(NULL), m_Net(NULL)
{
    m_Path = strdup(path);
}

static void *native_appworker_thread(void *arg)
{
    App *app = static_cast<App *>(arg);

    printf("Starting App worker...\n");

    while (!app->m_Action.stop) {
        pthread_mutex_lock(&app->m_ThreadMutex);

        while (!app->m_Action.active && !app->m_Action.stop) {
            pthread_cond_wait(&app->m_ThreadCond, &app->m_ThreadMutex);
        }
        if (app->m_Action.stop) {
            pthread_mutex_unlock(&app->m_ThreadMutex);
            return NULL;
        }

        switch (app->m_Action.type) {
            case App::APP_ACTION_EXTRACT:
            {
#define APP_READ_SIZE (1024L*1024L*2)
                struct zip_file *zfile;

                zfile = zip_fopen_index(app->m_fZip, app->m_Action.u32,
                    ZIP_FL_UNCHANGED);

                if (zfile == NULL) {
                    break;
                }

                char *content = (char *)malloc(sizeof(char) * APP_READ_SIZE);
                size_t total = 0;
                int r = 0;

                while ((r = zip_fread(zfile, content, APP_READ_SIZE)) >= 0) {
                    total += r;
                    app->actionExtractRead(content, r, total, app->m_Action.u64);
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
        app->m_Action.active = false;
        pthread_mutex_unlock(&app->m_ThreadMutex);
    }
    printf("Thread ended 2\n");
    return NULL;
}

static int Nidium_handle_app_messages(void *arg)
{
#define MAX_MSG_IN_ROW 32
    App *app = static_cast<App *>(arg);
    struct App::native_app_msg *ptr;
    int nread = 0;

    Nidium::Core::SharedMessages::Message *msg;

    while (++nread < MAX_MSG_IN_ROW && (msg = app->m_Messages->readMessage())) {
        switch (msg->event()) {
            case App::APP_MESSAGE_READ:
                ptr = static_cast<struct App::native_app_msg *>(msg->dataPtr());
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

void App::actionExtractRead(const char *buf, int len,
    size_t offset, size_t total)
{
    struct native_app_msg *msg = new struct native_app_msg;
    msg->data = (char *)malloc(len);
    msg->len  = len;
    msg->total = total;
    msg->offset = offset;
    msg->cb = (AppExtractCallback)m_Action.cb;
    msg->user = m_Action.user;

    memcpy(msg->data, buf, len);

    m_Messages->postMessage(msg, APP_MESSAGE_READ);
}

void App::runWorker(ape_global *net)
{
    m_Messages = new Nidium::Core::SharedMessages();

    m_Net = net;

    m_Action.active = false;
    m_Action.stop = false;

    m_Timer = APE_timer_create(net, 1,
        Nidium_handle_app_messages, this);

    pthread_mutex_init(&m_ThreadMutex, NULL);
    pthread_cond_init(&m_ThreadCond, NULL);

    m_WorkerIsRunning = true;

    pthread_create(&m_ThreadHandle, NULL, native_appworker_thread, this);

    pthread_mutex_lock(&m_ThreadMutex);
        pthread_cond_signal(&m_ThreadCond);
    pthread_mutex_unlock(&m_ThreadMutex);
}

int App::open()
{
    int err = 0;
    m_fZip = zip_open(m_Path, ZIP_CHECKCONS, &err);

    if (err != ZIP_ER_OK || m_fZip == NULL) {
        char buf_erreur[1024];
        zip_error_to_str(buf_erreur, sizeof buf_erreur, err, errno);
        printf("Failed to open zip file (%d) : %s\n", err, buf_erreur);
        return 0;
    }

    if ((m_NumFiles = zip_get_num_entries(m_fZip, ZIP_FL_UNCHANGED)) == -1 ||
        m_NumFiles == 0) {

        zip_close(m_fZip);
        m_fZip = NULL;

        return 0;
    }

    return this->loadManifest();
}

int App::extractApp(const char *path,
        void (*done)(void *, const char *), void *closure)
{
    char *fullpath = strdup(path);

    if (path[strlen(path)-1] != '/') {
        printf("extractApp : invalid path (non / terminated)\n");
        return 0;
    }

    if (m_fZip == NULL || !m_WorkerIsRunning) {
        printf("extractApp : you need to call open() and runWorker() before\n");
        return 0;
    }
#if 0
#define NIDIUM_CACHE_DIR "./cache/"
    if (mkdir(NIDIUM_CACHE_DIR, 0777) == -1 && errno != EEXIST) {
        printf("Cant create cache directory\n");
        return 0;
    }
    fullpath = (char *)malloc(sizeof(char) *
                (strlen(m_Path) + sizeof(NIDIUM_CACHE_DIR) + 16));

    sprintf(fullpath, NIDIUM_CACHE_DIR "%s", m_Path);

#undef NIDIUM_CACHE_DIR
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
    for (i = 0; i < m_NumFiles; i++) {
        const char *fname = zip_get_name(m_fZip, i, ZIP_FL_UNCHANGED);

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
    struct Extractor_s *arg = new Extractor_s;
    arg->app = this;
    arg->curIndex = first;
    arg->fName = zip_get_name(m_fZip, first, ZIP_FL_UNCHANGED);
    arg->data.offset = 0;
    arg->data.fp = NULL;
    arg->fDir = fullpath;
    arg->done = done;
    arg->closure = closure;
    arg->data.len = this->extractFile(arg->fName, Extractor, arg);

    return (arg->data.len != 0);
}

uint64_t App::extractFile(const char *file, AppExtractCallback cb, void *user)
{
    if (m_fZip == NULL || !m_WorkerIsRunning) {
        printf("extractFile : you need to call open() and runWorker() before\n");
        return 0;
    }

    uint64_t index;
    struct zip_file *zfile;
    struct zip_stat stat;

    if ((index = zip_name_locate(m_fZip, file, 0)) == -1 ||
        strcmp(zip_get_name(m_fZip, index, ZIP_FL_UNCHANGED), file) != 0 ||
        (zfile = zip_fopen_index(m_fZip, index, ZIP_FL_UNCHANGED)) == NULL) {

        printf("extractFile: Failed to open %s\n", file);
        return 0;
    }

    zip_stat_init(&stat);

    if (zip_stat_index(m_fZip, index, ZIP_FL_UNCHANGED, &stat) == -1 ||
       !(stat.valid & (ZIP_STAT_SIZE|ZIP_STAT_COMP_SIZE))) {
        zip_fclose(zfile);
        return 0;
    }

    pthread_mutex_lock(&m_ThreadMutex);
    if (m_Action.active) {
        pthread_mutex_unlock(&m_ThreadMutex);
        printf("extractFile: Worker already working...\n");
        zip_fclose(zfile);
        return 0;
    }

    m_Action.active = true;
    m_Action.type = APP_ACTION_EXTRACT;
    m_Action.ptr  = strdup(file);
    m_Action.u32  = index;
    m_Action.u64  = stat.size;
    m_Action.cb = (void *)cb;
    m_Action.user = user;
    pthread_cond_signal(&m_ThreadCond);
    pthread_mutex_unlock(&m_ThreadMutex);
    zip_fclose(zfile);

    return stat.size;
}

int App::loadManifest()
{
#define MPROP(root, str, type, out) \
Json::Value out; \
if (!root.isMember(str) || !(out = root[str]) || !out.is ## type()) { \
    printf("Manifest error : " str " property not found or wrong type (required : " #type ")\n"); \
    return 0; \
}
    if (m_fZip == NULL) return 0;

    int index;
    char *content;
    struct zip_file *manifest;
    struct zip_stat stat;

    if ((index = zip_name_locate(m_fZip, NIDIUM_MANIFEST, ZIP_FL_NODIR)) == -1 ||
        strcmp(zip_get_name(m_fZip, index, ZIP_FL_UNCHANGED), NIDIUM_MANIFEST) != 0 ||
        (manifest = zip_fopen_index(m_fZip, index, ZIP_FL_UNCHANGED)) == NULL) {

        printf(NIDIUM_MANIFEST " not found\n");
        return 0;
    }

    zip_stat_init(&stat);

    if (zip_stat_index(m_fZip, index, ZIP_FL_UNCHANGED, &stat) == -1 ||
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

    if (!m_Reader.parse(content, content+stat.size, root)) {
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

App::~App()
{
    if (m_WorkerIsRunning) {
        m_Action.stop = true;

        pthread_mutex_lock(&m_ThreadMutex);
        pthread_cond_signal(&m_ThreadCond);
        pthread_mutex_unlock(&m_ThreadMutex);

        pthread_join(m_ThreadHandle, NULL);
        APE_timer_destroy(m_Net, m_Timer);
    }
    if (m_fZip) {
        zip_close(m_fZip);
    }

    free(m_Path);
}
// }}}

} // namespace Frontend
} // namespace Nidium

