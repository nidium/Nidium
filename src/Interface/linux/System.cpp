/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <libnotify/notify.h>

#include <ape_log.h>

#include "System.h"
#include <libgen.h>
#include <string>

namespace Nidium {
namespace Interface {

// {{{ Functions
static void get_dpi(int *x, int *y)
{
    double xres, yres;
    Display *dpy;
    char *displayname = NULL;
    int scr           = 0; /* Screen number */

    if ((NULL == x) || (NULL == y)) {
        return;
    }

    dpy = XOpenDisplay(displayname);
    if (dpy == nullptr) {
        *x = -1;
        *y = -1;

        ndm_log(NDM_LOG_ERROR, "System", "Failed to open X display");
        return;
    }

    /*
     * There are 2.54 centimeters to an inch; so there are 25.4 millimeters.
     *
     *     dpi = N pixels / (M millimeters / (25.4 millimeters / 1 inch))
     *         = N pixels / (M inch / 25.4)
     *         = N * 25.4 pixels / M inch
     */
    xres = (((static_cast<double>(DisplayWidth(dpy, scr))) * 25.4)
            / (static_cast<double>(DisplayWidthMM(dpy, scr))));
    yres = (((static_cast<double>(DisplayHeight(dpy, scr))) * 25.4)
            / (static_cast<double>(DisplayHeightMM(dpy, scr))));

    *x = static_cast<int>(xres + 0.5);
    *y = static_cast<int>(yres + 0.5);

    XCloseDisplay(dpy);
}
// }}}

// {{{ System
System::System() : m_SystemUIReady(false)
{
    int x, y;
    get_dpi(&x, &y);
    if (x == -1 && y == -1) {
        exit(3);
    }

    m_fBackingStorePixelRatio = static_cast<float>(x) / 96.f;
    m_fBackingStorePixelRatio = 1.0;

    char procPath[PATH_MAX];
    char nidiumPath[PATH_MAX];
    pid_t pid = getpid();

    sprintf(procPath, "/proc/%d/exe", pid);

    if (readlink(procPath, nidiumPath, PATH_MAX) == -1)
        m_EmbedPath = nullptr;
    else {
        const char *embed = "src/Embed/";
        char *dir         = dirname(dirname(nidiumPath));
        size_t len        = strlen(dir) + strlen(embed) + 2;

        m_EmbedPath = static_cast<char *>(malloc(sizeof(char) * len));

        snprintf(m_EmbedPath, len, "%s/%s", dir, embed);
    }

    notify_init("Nidium");
}

System::~System()
{
    notify_uninit();
}

float System::backingStorePixelRatio()
{
    return m_fBackingStorePixelRatio;
}

const char *System::getEmbedDirectory()
{
    return m_EmbedPath;
}

const char *System::getCacheDirectory()
{
    const char *homedir = getUserDirectory();
    char nHome[4096];

    snprintf(nHome, 4096, "%s.config/nidium/", homedir);

    if (mkdir(nHome, 0755) == -1 && errno != EEXIST) {
        ndm_logf(NDM_LOG_ERROR, "System", "Can not create cache directory %s", nHome);
        return NULL;
    }

    return strdup(nHome);
}

const char *System::getUserDirectory()
{
    static char retHome[4096];

    char *homedir = getenv("HOME");
    if (!homedir) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    snprintf(retHome, 4096, "%s/", homedir);

    return retHome;
}

void System::alert(const char *message, AlertType type)
{
    // SystemUI (GTK/QT) must be initialized after SDL (or GTK crash)
    // Since System::alert() can be called before SDL is initialized
    // we initialize system UI on the fly if needed
    this->initSystemUI();

    GtkMessageType gtkType = GTK_MESSAGE_INFO;

    switch (type) {
        case ALERT_WARNING:
            gtkType = GTK_MESSAGE_WARNING;
            break;
        case ALERT_CRITIC:
            gtkType = GTK_MESSAGE_ERROR;
            break;
        case ALERT_INFO:
            break;
        default:
            break;
    }

    GtkWidget *dialog = gtk_message_dialog_new(
        (GtkWindow *)NULL, GTK_DIALOG_DESTROY_WITH_PARENT, gtkType,
        GTK_BUTTONS_CLOSE, "%s", message);

    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
}

void System::initSystemUI()
{
    if (!m_SystemUIReady) {
#ifdef NIDIUM_USE_GTK
        gtk_init(0, NULL);
#endif
    }
}


const char *System::cwd()
{
    static char dir[MAXPATHLEN];

    getcwd(dir, MAXPATHLEN);

    return dir;
}
const char *System::getLanguage()
{

    const char *lang;

    lang = setlocale(LC_IDENTIFICATION, NULL);

    return lang;
}

void System::sendNotification(const char *title,
                              const char *content,
                              bool sound)
{
    NotifyNotification *notification
        = notify_notification_new(title, content, nullptr);

    notify_notification_show(notification, nullptr /* error reporter */);

    g_object_unref(G_OBJECT(notification));
}

const char *System::execute(const char *cmd)
{
    char buffer[128];
    FILE *fp;
    std::string *result = new std::string();

    fp = popen(cmd, "r");
    if (fp == nullptr) {
        return nullptr;
    }

    while (!feof(fp)) {
        if (fgets(buffer, 128, fp) != nullptr) {
            result->append(buffer);
        }
    }

    pclose(fp);

    // FIXME : Memory leak, caller should have to free the
    // memory but osx implementation is different from linux
    return result->c_str();
}
// }}}


} // namespace Interface
} // namespace Nidium
