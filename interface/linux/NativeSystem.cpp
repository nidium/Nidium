#include "NativeSystem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <pwd.h>
#include <gtk/gtk.h>
#include <string>
#include <unistd.h>

NativeSystem::NativeSystem() : m_SystemUIReady(false)
{
    fbackingStorePixelRatio = 1.0;
}

float NativeSystem::backingStorePixelRatio()
{
    return fbackingStorePixelRatio;
}

const char *NativeSystem::getPrivateDirectory()
{
    static char privatedir[MAXPATHLEN];
    
    strncpy(privatedir, this->pwd(), MAXPATHLEN - (sizeof("/private/") + 1));
    strcat(privatedir, "/private/");

    return privatedir;
}

const char *NativeSystem::getCacheDirectory()
{
    char *homedir = getUserDirectory();
    char nHome[2048];

    snprintf(nHome, 2048, "%s.nidium/", homedir);

    if (mkdir(nHome, 0755) == -1 && errno != EEXIST) {
        printf("Cant create cache directory %s\n", nHome);
        return NULL;
    }

    return strdup(nHome);
}

const char *NativeSystem::getUserDirectory()
{
    static char retHome[2048];

    char *homedir = getenv("HOME");
    if (!homedir) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    sprintf(retHome, "%s/", homedir);

    return retHome;
}

void NativeSystem::alert(const char *message, AlertType type)
{
    // SystemUI (GTK/QT) must be initialized after SDL (or GTK crash)
    // Since NativeSystem::alert() can be called before SDL is initialized
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
    }

    GtkWidget *dialog = gtk_message_dialog_new ((GtkWindow *)NULL,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            gtkType,
            GTK_BUTTONS_CLOSE,
            message);

    gtk_dialog_run (GTK_DIALOG (dialog));

    gtk_widget_destroy (dialog);
}

void NativeSystem::initSystemUI()
{
    if (!m_SystemUIReady) {
#ifdef NATIVE_USE_GTK
        gtk_init(0, NULL);
#endif
    }
}


const char *NativeSystem::pwd()
{
    static char dir[MAXPATHLEN];

    getcwd(dir, MAXPATHLEN);

    return dir;
}
