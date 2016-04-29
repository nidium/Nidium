#include "X11UIInterface.h"
#include "System.h"

#include <ape_netlib.h>

#include <X11/cursorfont.h>
#include <../build/include/SDL_config.h>
#include <SDL.h>
#include <SDL_syswm.h>

#ifdef NIDIUM_USE_GTK
#include <gtk/gtk.h>
#endif

namespace Nidium {
namespace Interface {


// {{{ Functions
#if 0
static Window *UIX11Interface(SDL_Window *m_Win)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(m_Win, &info);

    return (Window*)info.info.x11.window;
}
#endif
// }}}

// {{{ UIX11Interface
UIX11Interface::UIX11Interface() : UIInterface(), console(NULL)
{

}

void UIX11Interface::quitApplication()
{
#ifdef NIDIUM_USE_GTK
    while (gtk_events_pending ()) {
        gtk_main_iteration();
    }
#endif
    exit(1);
}

void UIX11Interface::hitRefresh()
{
#ifdef NIDIUM_USE_GTK
    while (gtk_events_pending ()) {
        gtk_main_iteration();
    }
#endif
    this->restartApplication();
}

void UIX11Interface::onWindowCreated()
{
    console = new UIX11Console();
    static_cast<System *>(SystemInterface::_interface)->initSystemUI();
}


void UIX11Interface::openFileDialog(const char *files[],
    void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg, int flags)
{
#ifdef NIDIUM_USE_GTK
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
}

static int ProcessSystemLoop(void *arg)
{
    //SDL_PumpEvents();
    UIX11Interface *ui = (UIX11Interface *)arg;

    /*if (ui->m_NativeCtx) {
        ui->makeMainGLCurrent();
    }*/

    gtk_main_iteration_do(FALSE);
    return 4;
}

void UIX11Interface::runLoop()
{
    APE_timer_create(m_Gnet, 1, UIInterface::HandleEvents, (void *)this);
    APE_timer_create(m_Gnet, 1, ProcessSystemLoop, (void *)this);
    APE_loop_run(m_Gnet);
}


void UIX11Interface::log(const char *buf)
{
    fwrite(buf, sizeof(char), strlen(buf), stdout);
    fflush(stdout);
}

void UIX11Interface::logf(const char *format, ...)
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

void UIX11Interface::vlog(const char *format, va_list ap)
{
    char *buff;
    int len;

    len = vasprintf(&buff, format, ap);

    this->log(buff);

    free(buff);
}

void UIX11Interface::setSystemCursor(CURSOR_TYPE cursorvalue)
{
    int cursor;
    SDL_SysWMinfo info;

    switch(cursorvalue) {
        case UIX11Interface::ARROW:
            cursor = XC_left_ptr;
            break;
        case UIX11Interface::BEAM:
            cursor = XC_xterm;
            break;
        case UIX11Interface::CROSS:
            cursor = XC_crosshair;
            break;
        case UIX11Interface::POINTING:
            cursor = XC_hand2;
            break;
        case UIX11Interface::CLOSEDHAND:
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
void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data)
{
        printf("Clicked on tray icon\n");
}
void UIX11Interface::enableSysTray()
{
    SystemMenuItem *item = m_SystemMenu.items();
    if (!item) {
        return;
    }

    size_t icon_len, icon_width, icon_height;
    const uint8_t *icon_custom = m_SystemMenu.getIcon(&icon_len,
                                    &icon_width, &icon_height);

    if (icon_custom) {
        GBytes *bytes = g_bytes_new(icon_custom, icon_len);

        GdkPixbuf *gicon = gdk_pixbuf_new_from_bytes(bytes, GDK_COLORSPACE_RGB,
            TRUE, 8, icon_width, icon_height, 4*icon_width);

        GtkStatusIcon *statusicon = gtk_status_icon_new();
        g_signal_connect(G_OBJECT(statusicon), "activate",
                         G_CALLBACK(tray_icon_on_click), NULL);
        gtk_status_icon_set_from_pixbuf(statusicon, gicon);

        gtk_status_icon_set_visible(statusicon, TRUE);

    }
}

void UIX11Interface::renderSystemTray()
{

}

// }}}

// {{{ UIX11Console
void UIX11Console::log(const char *str)
{
    if (strcmp("\n", str) == 0) {
        fprintf(stdout, "\n");
    } else {
        fprintf(stdout, "[CONSOLE] %s", str);
    }
}
// }}}

} // namespace Interface
} // namespace Nidium

