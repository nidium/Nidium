#include "NativeSystem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <gtk/gtk.h>

NativeSystem::NativeSystem()
{
    fbackingStorePixelRatio = 1.0;
#ifdef NATIVE_USE_GTK
    gtk_init(0, NULL);
#endif
}

float NativeSystem::backingStorePixelRatio()
{
    return fbackingStorePixelRatio;
}

const char *NativeSystem::getPrivateDirectory()
{
    return "private/";
}

const char *NativeSystem::getCacheDirectory()
{
    char *homedir = getenv("HOME");
    char nHome[1024];

    if (!homedir) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    snprintf(nHome, 1024, "%s/.nidium/", homedir);

    if (mkdir(nHome, 0755) == -1 && errno != EEXIST) {
        printf("Cant create cache directory %s\n", nHome);
        return NULL;
    }

    return strdup(nHome);
}

void NativeSystem::alert(const char *message, AlertType type)
{
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
