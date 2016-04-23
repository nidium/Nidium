#include "X11UIInterface.h"

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
#include "SDL_keycode_translate.h"

#ifdef NATIVE_USE_GTK
#include <gtk/gtk.h>
#endif

#ifdef NATIVE_USE_QT
#include <QtGui>
#include <QFileDialog>
#include <QString>
#endif

#include "Binding/JSWindow.h"
#include "NML/App.h"
#include "NML/Context.h"
#include "NML/NML.h"

#include "System.h"

#define kNativeWidth 1280
#define kNativeHeight 600

#define kNativeTitleBarHeight 0

#define kNativeVSYNC 1

uint32_t ttfps = 0;

#if 0
static Window *NativeX11Window(SDL_Window *m_Win)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(m_Win, &info);

    return (Window*)info.info.x11.window;
}
#endif

void NativeX11UIInterface_onNMLLoaded(void *arg)
{
    NativeX11UIInterface *UI = static_cast<NativeX11UIInterface *>(arg);
    UI->onNMLLoaded();
}

int NativeEvents(NativeX11UIInterface *NUII)
{
    SDL_Event event;
    int nrefresh = 0;
    //while(1) {
    int nevents = 0;
        while(SDL_PollEvent(&event)) {
            Nidium::Binding::NativeJSwindow *window = NULL;
            if (NUII->m_NativeCtx) {
                window = Nidium::Binding::NativeJSwindow::GetObject(NUII->m_NativeCtx->getNJS());
            }
            nevents++;
            switch(event.type) {
                case SDL_WINDOWEVENT:
                    if (window) {
                        switch (event.window.event) {
                            case SDL_WINDOWEVENT_FOCUS_GAINED:
                                window->windowFocus();
                                break;
                            case SDL_WINDOWEVENT_FOCUS_LOST:
                                window->windowBlur();
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case SDL_TEXTINPUT:
                    if (window && strlen(event.text.text) > 0) {
                        window->textInput(event.text.text);
                    }
                    break;
                case SDL_USEREVENT:
                    break;
                case SDL_QUIT:
                    NUII->stopApplication();
                    SDL_Quit();
#ifdef NATIVE_USE_GTK
                    while (gtk_events_pending ()) {
                        gtk_main_iteration();
                    }
#endif
                    exit(1);
                    break;
                case SDL_MOUSEMOTION:
                    if (window) {
                        window->mouseMove(event.motion.x, event.motion.y - kNativeTitleBarHeight,
                                   event.motion.xrel, event.motion.yrel);
                    }
                    break;
                case SDL_MOUSEWHEEL:
                {
                    int cx, cy;
                    SDL_GetMouseState(&cx, &cy);
                    if (window) {
                        window->mouseWheel(event.wheel.x, event.wheel.y, cx, cy - kNativeTitleBarHeight);
                    }
                    break;
                }
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                    if (window) {
                        window->mouseClick(event.button.x, event.button.y - kNativeTitleBarHeight,
                                    event.button.state, event.button.button, event.button.clicks);
                    }
                break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    int keyCode = 0;

                    int mod = 0;
                    if (
                        (&event.key)->keysym.sym == SDLK_r &&
                        (event.key.keysym.mod & KMOD_CTRL) && event.type == SDL_KEYDOWN) {
                        if (++nrefresh > 1) {
                            break;
                        }
                        fprintf(stdout, "\n\n= = = = Refresh... = = = = =\n");
                        //[console clear];
#ifdef NATIVE_USE_GTK
                        while (gtk_events_pending ()) {
                            gtk_main_iteration();
                        }
#endif
                        //fprintf(stdout, "\n\n= = = = Restarting... = = = = =\n");
                        NUII->restartApplication();
                        break;
                    }
                    if (event.key.keysym.sym >= 97 && event.key.keysym.sym <= 122) {
                        keyCode = event.key.keysym.sym - 32;
                    } else {
                        keyCode = SDL_KEYCODE_TO_DOMCODE(event.key.keysym.sym);
                    }

                    if (event.key.keysym.mod & KMOD_SHIFT) {
                        mod |= Nidium::Binding::NIDIUM_KEY_SHIFT;
                    }
                    if (event.key.keysym.mod & KMOD_ALT) {
                        mod |= Nidium::Binding::NIDIUM_KEY_ALT;
                    }
                    if (event.key.keysym.mod & KMOD_CTRL) {
                        mod |= Nidium::Binding::NIDIUM_KEY_CTRL;
                    }

                    if (window) {
                        window->keyupdown(SDL_KEYCODE_GET_CODE(keyCode), mod,
                            event.key.state, event.key.repeat,
                            SDL_KEYCODE_GET_LOCATION(keyCode));
                    }
                    /*fprintf(stdout, "Mapped to %d\n", keyCode);
                    fprintf(stout, "Key : %d %d %d %d %d uni : %d\n", event.key.keysym.sym,
                           event.key.repeat,
                           event.key.state,
                           event.key.type,
                           event.key.keysym.scancode,
                           event.key.keysym.unicode);*/
                    //return;
                    break;
                }
            }
        }

        if (ttfps%20 == 0 && NUII->m_NativeCtx != NULL) {
            NUII->m_NativeCtx->getNJS()->gc();
        }

        if (NUII->m_CurrentCursor != NativeX11UIInterface::NOCHANGE) {
            int cursor;
            SDL_SysWMinfo info;

            switch(NUII->m_CurrentCursor) {
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

            if (SDL_GetWindowWMInfo(NUII->m_Win, &info)) {
                Cursor c = XCreateFontCursor(info.info.x11.display, cursor);
                Display *d = info.info.x11.display;

                XDefineCursor(d, info.info.x11.window, c);
                XFlush(d);
                XFreeCursor(d, c);
            }

            NUII->m_CurrentCursor = NativeX11UIInterface::NOCHANGE;
        }

        if (NUII->m_NativeCtx) {
            NUII->m_NativeCtx->frame();
        }

        //NUII->getConsole()->flush();
        if (NUII->getFBO() != 0 && NUII->m_NativeCtx) {
            //glFlush();
            //glFinish();
            glReadBuffer(GL_COLOR_ATTACHMENT0);

            glReadPixels(0, 0, NUII->getWidth(), NUII->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, NUII->getFrameBufferData());
            uint8_t *pdata = NUII->getFrameBufferData();

            NUII->m_NativeCtx->rendered(pdata, NUII->getWidth(), NUII->getHeight());
        } else {
            SDL_GL_SwapWindow(NUII->m_Win);
        }

    //}
    ttfps++;
    //NSLog(@"ret : %d for %d events", (tend - tstart), nevents);
    return 16;
}

static int NativeProcessUI(void *arg)
{
    return NativeEvents(static_cast<NativeX11UIInterface *>(arg));
}

#if 0
static bool NativeExtractMain(const char *buf, int len,
    size_t offset, size_t total, void *user)
{
    NativeX11UIInterface *UI = (NativeX11UIInterface *)user;

    memcpy(UI->mainjs.buf+UI->mainjs.offset, buf, len);
    UI->mainjs.offset += len;

    if (offset == total) {
        if (UI->NJS->LoadScriptContent(UI->mainjs.buf, total, "main.js")) {
            UI->NJS->Loaded();

        }
    }

    return true;
}

static void NativeDoneExtracting(void *closure, const char *fpath)
{
    NativeX11UIInterface *ui = (NativeX11UIInterface *)closure;
    chdir(fpath);
    fprintf(stdout, "Changing directory to : %s\n", fpath);
    ui->m_Nml = new Nidium::NML::NativeNML(ui->m_Gnet);
    ui->m_Nml->setNJS(ui->NJS);
    ui->m_Nml->loadFile("./index.nml");
}
#endif
static void NativeDoneExtracting(void *closure, const char *fpath)
{
    NativeX11UIInterface *ui = static_cast<NativeX11UIInterface*>(closure);
    if (chdir(fpath) != 0) {
        fprintf(stderr, "Cant enter cache directory (%d)\n", errno);
        return;
    }
    fprintf(stdout, "Changing directory to : %s\n", fpath);

    ui->m_Nml = new Nidium::NML::NativeNML(ui->m_Gnet);
    ui->m_Nml->loadFile("./index.nml", NativeX11UIInterface_onNMLLoaded, ui);
}

NativeX11UIInterface::NativeX11UIInterface()
{
    this->m_Width = 0;
    this->m_Height = 0;
    this->m_Initialized = false;
    this->m_Nml = NULL;
    this->m_FilePath = NULL;
    this->console = NULL;

    this->m_CurrentCursor = NOCHANGE;
    this->m_NativeCtx = NULL;

    m_Gnet = APE_init();
}

bool NativeX11UIInterface::createWindow(int width, int height)
{
    SDL_GLContext contexteOpenGL;

    if (!this->m_Initialized) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == -1)
        {
            NLOG("Can't init SDL:  %s\n", SDL_GetError());
            return false;
        }

        static_cast<NativeSystem *>(NativeSystemInterface::_interface)->initSystemUI();

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        this->m_Win = SDL_CreateWindow("Native - Running", 100, 100,
            width, height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL/* | SDL_WINDOW_FULLSCREEN*/);

        if (this->m_Win == NULL) {
            NLOG("Cant create window (SDL)\n");
            return false;
        }

        this->m_Width = width;
        this->m_Height = height;

        //window = NativeX11Window(win);

        /*[window setCollectionBehavior:
                 NSWindowCollectionBehaviorFullScreenPrimary];*/

        initControls();

        //[btn setFrame:CGRectMake(btn.frame.origin.x, btn.frame.origin.y-4, btn.frame.size.width, btn.frame.size.width)];

        //[window setBackgroundColor:[NSColor colorWithSRGBRed:0.0980 green:0.1019 blue:0.09411 alpha:1.]];

        //[window setFrameAutosaveName:@"nativeMainWindow"];
        if (kNativeTitleBarHeight != 0) {
            //[window setStyleMask:NSTexturedBackgroundWindowMask|NSTitledWindowMask];

            //[window setContentBorderThickness:32.0 forEdge:NSMinYEdge];
            //[window setOpaque:NO];
        }
        //[window setMovableByWindowBackground:NO];
        //[window setOpaque:NO]; // YES by default
        //[window setAlphaValue:0.5];

        contexteOpenGL = SDL_GL_CreateContext(m_Win);
        m_MainGLCtx = contexteOpenGL;
        if (contexteOpenGL == NULL) {
            NLOG("Failed to create OpenGL context : %s", SDL_GetError());
            return false;
        }
        SDL_StartTextInput();

        if (SDL_GL_SetSwapInterval(kNativeVSYNC) == -1) {
            fprintf(stdout, "Cant vsync\n");
        }

        glViewport(0, 0, width*2, height*2);
        NLOG("[DEBUG] OpenGL %s", glGetString(GL_VERSION));

        console = new NativeUIX11Console();

        this->m_Initialized = true;
    } else {
        this->setWindowSize(width, height);
    }

    Nidium::NML::NativeContext::CreateAndAssemble(this, m_Gnet);

    return true;
}

void NativeX11UIInterface::setCursor(CURSOR_TYPE type)
{
    this->m_CurrentCursor = type;
}

void NativeX11UIInterface::setWindowTitle(const char *name)
{
    SDL_SetWindowTitle(m_Win, (name == NULL || *name == '\0' ? "nidium" : name));
}
const char *NativeX11UIInterface::getWindowTitle() const
{
    return SDL_GetWindowTitle(m_Win);
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
    /*
    NSWindow *window = NativeX11Window(win);
    NSUInteger mask = [window styleMask];

    fprintf(stdout, "setting titlebar color\n");

    if ((mask & NSTexturedBackgroundWindowMask) == 0) {
        [window setStyleMask:mask|NSTexturedBackgroundWindowMask];
        [window setMovableByWindowBackground:NO];
        [window setOpaque:NO];
    }

    [window setBackgroundColor:[NSColor
        colorWithSRGBRed:((double)r)/255.
                   green:((double)g)/255.
                    blue:((double)b)/255.
                   alpha:((double)a)/255]];
   */
}

void NativeX11UIInterface::initControls()
{
    /*
    NSWindow *window = NativeX11Window(win);
    NSButton *close = [window standardWindowButton:NSWindowCloseButton];
    NSButton *min = [window standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *max = [window standardWindowButton:NSWindowZoomButton];

    memset(&this->controls, 0, sizeof(CGRect));

    if (close) {
        this->controls.closeFrame = close.frame;
    }
    if (min) {
        this->controls.minFrame = min.frame;
    }
    if (max) {
        this->controls.zoomFrame = max.frame;
    }
    */
}

void NativeX11UIInterface::setWindowControlsOffset(double x, double y)
{
    /*
    NSWindow *window = NativeX11Window(win);
    NSButton *close = [window standardWindowButton:NSWindowCloseButton];
    NSButton *min = [window standardWindowButton:NSWindowMiniaturizeButton];
    NSButton *max = [window standardWindowButton:NSWindowZoomButton];

    if (close) {
        [close setFrame:CGRectMake(
            this->controls.closeFrame.origin.x+x,
            this->controls.closeFrame.origin.y-y,
            this->controls.closeFrame.size.width,
            this->controls.closeFrame.size.height)];
    }
    if (min) {
        [min setFrame:CGRectMake(
            this->controls.minFrame.origin.x+x,
            this->controls.minFrame.origin.y-y,
            this->controls.minFrame.size.width,
            this->controls.minFrame.size.height)];
    }
    if (max) {
        [max setFrame:CGRectMake(
            this->controls.zoomFrame.origin.x+x,
            this->controls.zoomFrame.origin.y-y,
            this->controls.zoomFrame.size.width,
            this->controls.zoomFrame.size.height)];
    }
    */
}

void NativeX11UIInterface::runLoop()
{

    APE_timer_create(m_Gnet, 1, NativeProcessUI, (void *)this);

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

void NativeX11UIInterface::setClipboardText(const char *text)
{
    SDL_SetClipboardText(text);
}

char *NativeX11UIInterface::getClipboardText()
{
    return SDL_GetClipboardText();
}

NativeUIX11Console::~NativeUIX11Console()
{
}

void NativeX11UIInterface::restartApplication(const char *path)
{
    this->stopApplication();
    this->runApplication(path == NULL ? this->m_FilePath : path);
}

bool NativeX11UIInterface::runApplication(const char *path)
{
    Nidium::Core::Messages::initReader(m_Gnet);

    if (path != this->m_FilePath) {
        if (this->m_FilePath) {
            free(this->m_FilePath);
        }
        this->m_FilePath = strdup(path);
    }
    if (path == NULL || strlen(path) < 5) {
        return false;
    }
    //    FILE *main = fopen("index.nml", "r");
    const char *ext = &path[strlen(path)-4];

    if (strncasecmp(ext, ".npa", 4) == 0) {
        FILE *main = fopen(path, "r");
        if (main == NULL) {
            return false;
        }
        Nidium::NML::NativeApp *app = new Nidium::NML::NativeApp(path);
        if (app->open()) {
            if (!this->createWindow(app->getWidth()*2, 2*app->getHeight()+kNativeTitleBarHeight)) {
                return false;
            }
            this->setWindowTitle(app->getTitle());

            app->runWorker(this->m_Gnet);

            const char *cachePath = this->getCacheDirectory();
            char *uidpath = (char *)malloc(sizeof(char) *
                                (strlen(app->getUDID()) + strlen(cachePath) + 16));
            sprintf(uidpath, "%s%s.content/", cachePath, app->getUDID());

            app->extractApp(uidpath, NativeDoneExtracting, this);
            free(uidpath);
            /*this->mainjs.buf = (char *)malloc(fsize);
            this->mainjs.len = fsize;
            this->mainjs.offset = 0;

            fprintf(stdout, "Start looking for main.js of size : %ld\n", fsize);*/
            return true;
        } else {
            delete app;
        }
    } else {
        this->m_Nml = new Nidium::NML::NativeNML(this->m_Gnet);
        this->m_Nml->loadFile(path, NativeX11UIInterface_onNMLLoaded, this);

        return true;
    }
    return false;
}

void NativeX11UIInterface::stopApplication()
{
    if (this->m_Nml) delete this->m_Nml;
    if (this->m_NativeCtx) {
        delete this->m_NativeCtx;
        this->m_NativeCtx = NULL;
        Nidium::Core::Messages::destroyReader();
    }
    this->m_Nml = NULL;
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    /* Also clear the front buffer */
    SDL_GL_SwapWindow(this->m_Win);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void NativeX11UIInterface::onNMLLoaded()
{
    if (!this->createWindow(
        this->m_Nml->getMetaWidth(),
        this->m_Nml->getMetaHeight() + kNativeTitleBarHeight)) {
        exit(2);
    }

    this->setWindowTitle(this->m_Nml->getMetaTitle());
}

void NativeX11UIInterface::setWindowSize(int w, int h)
{
    SDL_SetWindowSize(m_Win, w, h);
    this->m_Width = w;
    this->m_Height = h;
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

