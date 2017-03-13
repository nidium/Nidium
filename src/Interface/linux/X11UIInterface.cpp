/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "X11UIInterface.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <ape_netlib.h>

#include <X11/cursorfont.h>
#include <../build/include/SDL_config.h>

#include <SDL.h>
#include <SDL_syswm.h>

#include "System.h"

#include <gtk/gtk.h>

namespace Nidium {
namespace Interface {


// {{{ Functions
#if 0
static Window *UIX11Interface(SDL_Window *m_Win)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(m_Win, &info);

    return static_cast<Window*>(info.info.x11.window);
}
#endif
// }}}

// {{{ UIX11Interface
UIX11Interface::UIX11Interface()
    : UIInterface(), m_Mainjs({ 0, 0, 0 }), m_Console(NULL)
{
}

void UIX11Interface::quitApplication()
{
    this->processGtkPendingEvents();

    exit(1);
}

void UIX11Interface::hitRefresh()
{
    this->processGtkPendingEvents();

    this->restartApplication();
}

void UIX11Interface::onWindowCreated()
{
    m_Console = new UIX11Console(this);
    static_cast<System *>(SystemInterface::_interface)->initSystemUI();
}


void UIX11Interface::openFileDialog(const char *files[],
                                    void (*cb)(void *nof,
                                               const char *lst[],
                                               uint32_t len),
                                    void *arg,
                                    int flags)
{
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new(
        "Open File", nullptr, GTK_FILE_CHOOSER_ACTION_OPEN, "_Cancel",
        GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, nullptr);

    if (files != NULL) {
        GtkFileFilter *filter;
        char names[256];
        memset(names, 0, 256);
        int nameLength = 0;

        filter = gtk_file_filter_new();

        for (int i = 0; files[i] != NULL; i++) {
            char *name = static_cast<char *>(
                calloc(sizeof(char), strlen(files[i]) + 3));
            if (!name) continue;

            strcat(name, "*.");
            strcat(name, files[i]);

            gtk_file_filter_add_pattern(filter, name);

            nameLength += strlen(files[i]) + 4;

            if (nameLength < 256) {
                strcat(names, name);
                strcat(names, " ");
            }

            free(name);
        }

        if (nameLength > 0) {
            gtk_file_filter_set_name(GTK_FILE_FILTER(filter),
                                     (const gchar *)names);
        }

        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                                    GTK_FILE_FILTER(filter));
    }

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    const char **lst = NULL;
    int i = 0;
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        GSList *filenames, *list;
        filenames = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
        guint len = g_slist_length(filenames);

        lst = static_cast<const char **>(malloc(sizeof(char *) * len));
        if (!lst) {
            return;
        }

        list = filenames;

        while (list) {
            if (list->data) {
                lst[i] = strdup(static_cast<const char *>(list->data));
                g_free(list->data);
                i++;
            }
            list = list->next;
        }

        g_slist_free(filenames);
    }


    gtk_widget_destroy(dialog);

    this->processGtkPendingEvents();

    if (i > 0) {
        cb(arg, lst, i);

        for (int j = 0; j < i; j++) {
            // TODO: new style cast
            free((void *)(lst[j]));
        }
    } else {
        cb(arg, NULL, 0);
    }

    free(lst);
}

static int ProcessSystemLoop(void *arg)
{
    /*
    SDL_PumpEvents();
    UIX11Interface *ui = static_cast<UIX11Interface *>(arg);

    if (ui->getNidiumContext()) {
        ui->makeMainGLCurrent();
    }
    */

    gtk_main_iteration_do(FALSE);
    return 4;
}

void UIX11Interface::runLoop()
{
    APE_timer_create(m_Gnet, 1, UIInterface::HandleEvents,
                     static_cast<void *>(this));
    APE_timer_create(m_Gnet, 1, ProcessSystemLoop, static_cast<void *>(this));
    APE_loop_run(m_Gnet);
}

void UIX11Interface::processGtkPendingEvents()
{
    while (gtk_events_pending()) {
        gtk_main_iteration();
    }
}

#if 1
void UIX11Interface::setSystemCursor(CURSOR_TYPE cursorvalue)
{
    int cursor;
    SDL_SysWMinfo info;

    if (cursorvalue != UIInterface::HIDDEN) {
        this->hideCursor(false);
    }

    switch (cursorvalue) {
        case UIX11Interface::ARROW:
            cursor = XC_left_ptr;
            ndm_log(NDM_LOG_DEBUG, "X11UI", "Normal cursor");
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
        case UIX11Interface::HIDDEN:
            ndm_log(NDM_LOG_DEBUG, "X11UI", "Hide cursor");
            this->hideCursor(true);
            return;
        default:
            cursor = XC_left_ptr;
            break;
    }

    SDL_VERSION(&info.version);

    if (SDL_GetWindowWMInfo(m_Win, &info)) {
        Cursor c   = XCreateFontCursor(info.info.x11.display, cursor);
        Display *d = info.info.x11.display;

        XDefineCursor(d, info.info.x11.window, c);
        XFlush(d);
        XFreeCursor(d, c);
    }
}
#endif

void tray_icon_on_click(GtkStatusIcon *status_icon, gpointer user_data)
{
    ndm_log(NDM_LOG_DEBUG, "X11UI", "Clicked on tray icon");
}
void UIX11Interface::enableSysTray()
{
#if 0
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
#endif
}

void UIX11Interface::renderSystemTray()
{
}

// }}}

// {{{ UIX11Console
void UIX11Console::log(const char *str)
{
    if (m_IsOpen) {
        GtkScrolledWindow *scroll = GTK_SCROLLED_WINDOW(m_Scroll);
        GtkTextMark *mark         = gtk_text_buffer_get_insert(m_Buffer);
        GtkTextIter iter;

        // Add the text
        gtk_text_buffer_get_iter_at_mark(m_Buffer, &iter, mark);
        gtk_text_buffer_insert(m_Buffer, &iter, str, -1);

        // Scroll the window
        GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(scroll);

        gtk_adjustment_set_value(adjustment,
                                 gtk_adjustment_get_upper(adjustment));

        m_Interface->processGtkPendingEvents();
    } else {
        fprintf(stdout, "%s", str);
        fflush(stdout);
    }
}

static void consoleHidden(GtkWidget *widget, GdkEvent *ev, gpointer priv)
{
    UIX11Console *console = static_cast<UIX11Console *>(priv);
    console->hide();
}

void UIX11Console::show()
{
    if (m_IsOpen) return;

    if (!m_Window) {
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        m_TextView = gtk_text_view_new();
        m_Window   = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        m_Buffer   = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_TextView));
        m_Scroll   = gtk_scrolled_window_new(NULL, NULL);

        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m_Scroll),
                                       GTK_POLICY_AUTOMATIC,
                                       GTK_POLICY_AUTOMATIC);

        gtk_text_view_set_editable(GTK_TEXT_VIEW(m_TextView), FALSE);
        gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(m_TextView), FALSE);

        g_object_set(m_TextView, "monospace", TRUE, NULL);

        gtk_window_set_default_size(GTK_WINDOW(m_Window), 300, 200);
        gtk_window_set_title(GTK_WINDOW(m_Window), "Nidium Console");

        gtk_container_add(GTK_CONTAINER(m_Scroll), m_TextView);

        gtk_box_pack_start(GTK_BOX(vbox), m_Scroll, TRUE, TRUE, 0);

        gtk_container_add(GTK_CONTAINER(m_Window), vbox);

        g_signal_connect(m_Window, "delete-event", G_CALLBACK(consoleHidden),
                         this);
    }

    gtk_widget_show_all(m_Window);

    m_IsOpen = true;
}

void UIX11Console::hide()
{
    if (!m_Window || !m_IsOpen) return;

    ndm_log(NDM_LOG_DEBUG, "X11UI", "Hide");

    gtk_widget_hide(m_Window);
}
// }}}

} // namespace Interface
} // namespace Nidium
