#include "NativeX11UIInterface.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <X11/cursorfont.h>
#include <../build/include/SDL_config.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>


#ifdef NATIVE_USE_GTK
#include <gtk/gtk.h>
#endif

#ifdef NATIVE_USE_QT
#include <QtGui>
#include <QFileDialog>
#include <QString>
#endif

#include "NativeJSWindow.h"
#include "NativeApp.h"
#include "NativeContext.h"
#include "NativeSystem.h"
#include "NativeNML.h"

#define kNativeWidth 1280
#define kNativeHeight 600

#define kNativeVSYNC 1
#define kNativeTitleBarHeight 0

#if 0
static Window *NativeX11Window(SDL_Window *m_Win)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(m_Win, &info);

    return (Window*)info.info.x11.window;
}
#endif


NativeX11UIInterface::NativeX11UIInterface() :
    NativeUIInterface(), console(NULL)
{

}

void NativeX11UIInterface::quitApplication()
{
#ifdef NATIVE_USE_GTK
    while (gtk_events_pending ()) {
        gtk_main_iteration();
    }
#endif
    exit(1);
}

void NativeX11UIInterface::hitRefresh()
{
#ifdef NATIVE_USE_GTK
    while (gtk_events_pending ()) {
        gtk_main_iteration();
    }
#endif
    this->restartApplication();
}

void NativeX11UIInterface::onWindowCreated()
{
    console = new NativeUIX11Console();
    static_cast<NativeSystem *>(NativeSystemInterface::_interface)->initSystemUI();
}


void NativeX11UIInterface::openFileDialog(const char *files[],
    void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg, int flags)
{
#ifdef NATIVE_USE_GTK
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new ("Open File",
            (GtkWindow *)NULL,
            GTK_FILE_CHOOSER_ACTION_OPEN,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
            (gchar *)NULL);

    if (files != NULL) {
        GtkFileFilter *filter;
        char names[256];
        memset(names, 0, 256);
        int nameLength = 0;

        filter = gtk_file_filter_new();

        for (int i = 0; files[i] != NULL; i++) {
            char *name = (char *)calloc(sizeof(char), strlen(files[i]) + 3);
            if (!name) continue;

            strcat(name, "*.");
            strcat(name, files[i]);

            gtk_file_filter_add_pattern(filter, name);

            nameLength += strlen(files[i])+4;

            if (nameLength < 256) {
                strcat(names, name);
                strcat(names, " ");
            }

            free(name);
        }

        if (nameLength > 0) {
            gtk_file_filter_set_name(GTK_FILE_FILTER(filter), (const gchar *)names);
        }

        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(filter));
    }

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    const char **lst = NULL;
    int i = 0;
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        GSList *filenames, *list;
        filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER(dialog));
        guint len = g_slist_length(filenames);

        lst = (const char **)malloc(sizeof(char *) * len);
        if (!lst) {
            return;
        }

        list = filenames;

        while (list) {
            if (list->data) {
                lst[i] = strdup((const char *)list->data);
                g_free (list->data);
                i++;
            }
            list = list->next;
        }

        g_slist_free(filenames);
    }


    gtk_widget_destroy(dialog);

    while (gtk_events_pending ()) {
        gtk_main_iteration();
    }

    if (i > 0) {
        cb(arg, lst, i);

        for (int j = 0; j < i; j++) {
            free((void *)lst[j]);
        }
    } else {
        cb(arg, NULL, 0);
    }

    free(lst);
#endif
#ifdef NATIVE_USE_QT
    QApplication app(0, NULL);
    QString fileName = QFileDialog::getOpenFileName(NULL,
               "Open file", NULL, "Image Files (*.png *.jpg *.bmp)");
#endif
}

void NativeX11UIInterface::setTitleBarRGBAColor(uint8_t r, uint8_t g,
    uint8_t b, uint8_t a)
{

}

void NativeX11UIInterface::initControls()
{

}

void NativeX11UIInterface::setWindowControlsOffset(double x, double y)
{

}

void NativeX11UIInterface::runLoop()
{

    APE_timer_create(m_Gnet, 1, NativeUIInterface::HandleEvents, (void *)this);

    APE_loop_run(m_Gnet);
}

NativeUIX11Console::NativeUIX11Console ()
{
}

void NativeUIX11Console::log(const char *str)
{
    if (strcmp("\n", str) == 0) {
        fprintf(stdout, "\n");
    } else {
        fprintf(stdout, "[CONSOLE] %s", str);
    }
}

void NativeUIX11Console::show()
{
}

void NativeUIX11Console::hide()
{
}

bool NativeUIX11Console::hidden()
{
    return true;
}

void NativeUIX11Console::clear()
{
}

void NativeUIX11Console::flush()
{
}

NativeUIX11Console::~NativeUIX11Console()
{
}

void NativeX11UIInterface::stopApplication()
{
    if (this->m_Nml) delete this->m_Nml;
    if (this->m_NativeCtx) {
        delete this->m_NativeCtx;
        this->m_NativeCtx = NULL;
        NativeMessages::destroyReader();
    }
    this->m_Nml = NULL;
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    /* Also clear the front buffer */
    SDL_GL_SwapWindow(this->m_Win);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}


void NativeX11UIInterface::log(const char *buf)
{
    fwrite(buf, sizeof(char), strlen(buf), stdout);
    fflush(stdout);
}

void NativeX11UIInterface::logf(const char *format, ...)
{
    char *buff;
    int len;
    va_list val;

    va_start(val, format);
    len = vasprintf(&buff, format, val);
    va_end(val);

    this->log(buff);

    free(buff);
}

void NativeX11UIInterface::vlog(const char *format, va_list ap)
{
    char *buff;
    int len;

    len = vasprintf(&buff, format, ap);

    this->log(buff);

    free(buff);
}

void NativeX11UIInterface::setSystemCursor(CURSOR_TYPE cursorvalue)
{
    int cursor;
    SDL_SysWMinfo info;

    switch(cursorvalue) {
        case NativeX11UIInterface::ARROW:
            cursor = XC_left_ptr;
            break;
        case NativeX11UIInterface::BEAM:
            cursor = XC_xterm;
            break;
        case NativeX11UIInterface::CROSS:
            cursor = XC_crosshair;
            break;
        case NativeX11UIInterface::POINTING:
            cursor = XC_hand2;
            break;
        case NativeX11UIInterface::CLOSEDHAND:
            cursor = XC_hand1;
            break;
        default:
            cursor = XC_left_ptr;
            break;
    }

    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(m_Win, &info)) {
        Cursor c = XCreateFontCursor(info.info.x11.display, cursor);
        Display *d = info.info.x11.display;

        XDefineCursor(d, info.info.x11.window, c);
        XFlush(d);
        XFreeCursor(d, c);
    }
}
