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

#include "System.h"

namespace Nidium {
namespace Interface {

// {{{ Functions
static void get_dpi(int *x, int *y)
{
    double xres, yres;
    Display *dpy;
    char *displayname = NULL;
    int scr = 0; /* Screen number */

    if ((NULL == x) || (NULL == y)) { return ; }

    dpy = XOpenDisplay (displayname);
    if (dpy == nullptr) {
        *x = -1;
        *y = -1;

        fprintf(stderr, "Failed to open X display\n");
        return;
    }


    /*
     * There are 2.54 centimeters to an inch; so there are 25.4 millimeters.
     *
     *     dpi = N pixels / (M millimeters / (25.4 millimeters / 1 inch))
     *         = N pixels / (M inch / 25.4)
     *         = N * 25.4 pixels / M inch
     */
    xres = ((((double) DisplayWidth(dpy, scr)) * 25.4) /
            ((double) DisplayWidthMM(dpy, scr)));
    yres = ((((double) DisplayHeight(dpy, scr)) * 25.4) /
            ((double) DisplayHeightMM(dpy, scr)));

    *x = (int) (xres + 0.5);
    *y = (int) (yres + 0.5);

    XCloseDisplay (dpy);
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

    m_fBackingStorePixelRatio = float(x) / 96.f;
    m_fBackingStorePixelRatio = 1.0;
}

float System::backingStorePixelRatio()
{
    return m_fBackingStorePixelRatio;
}

const char *System::getPrivateDirectory()
{
    static char privatedir[MAXPATHLEN];

    strncpy(privatedir, this->pwd(), MAXPATHLEN - (sizeof("/private/") + 1));
    strcat(privatedir, "/private/");

    return privatedir;
}

const char *System::getCacheDirectory()
{
    const char *homedir = getUserDirectory();
    char nHome[2048];

    snprintf(nHome, 2048, "%s.nidium/", homedir);

    if (mkdir(nHome, 0755) == -1 && errno != EEXIST) {
        fprintf(stderr, "Cant create cache directory %s\n", nHome);
        return NULL;
    }

    return strdup(nHome);
}

const char *System::getUserDirectory()
{
    static char retHome[2048];

    char *homedir = getenv("HOME");
    if (!homedir) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    sprintf(retHome, "%s/", homedir);

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

    GtkWidget *dialog = gtk_message_dialog_new ((GtkWindow *)NULL,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            gtkType,
            GTK_BUTTONS_CLOSE,
            "%s", message);

    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
}

void System::initSystemUI()
{
    if (!m_SystemUIReady) {
#ifdef NIDIUM_USE_GTK
        gtk_init(0, NULL);
#endif
    }
}


const char *System::pwd()
{
    static char dir[MAXPATHLEN];

    getcwd(dir, MAXPATHLEN);

    return dir;
}
const char *System::getLanguage()
{

    const char * lang;

    lang = setlocale(LC_IDENTIFICATION, NULL);

    return lang;
}
// }}}

} // namespace Interface
} // namespace Nidium

