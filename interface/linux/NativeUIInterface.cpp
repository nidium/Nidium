
#include "NativeX11UIInterface.h"
#include <NativeJS.h>
#include <NativeSkia.h>
#include <NativeApp.h>
#ifdef NATIVE_USE_GTK
#include <gtk/gtk.h>
#endif
#ifdef NATIVE_USE_QT
#include <QtGui>
#include <QFileDialog>
#include <QString>
#endif
#include <../build/include/SDL_config.h> 
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include <native_netlib.h>
#include <NativeNML.h>
#include <string.h>
#include <stdio.h>

#include <X11/Xlib.h>

#define kNativeWidth 1280
#define kNativeHeight 600

#define kNativeTitleBarHeight 0

#define kNativeVSYNC 1

uint32_t ttfps = 0;


static Window *NativeX11Window(SDL_Window *win)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(win, &info);

    return (Window*)info.info.x11.window;
}

int NativeEvents(NativeX11UIInterface *NUII)
{   
    SDL_Event event;
    int nrefresh = 0;
    //while(1) {
    int nevents = 0;
        while(SDL_PollEvent(&event)) {
            nevents++;
            switch(event.type) {
                case SDL_TEXTINPUT:
                    NUII->NJS->textInput(event.text.text);
                    break;
                case SDL_USEREVENT:
                    break;
                case SDL_QUIT:
                    delete NUII->NJS;
                    SDL_Quit();
#ifdef NATIVE_USE_GTK
                    while (gtk_events_pending ()) {
                        gtk_main_iteration();
                    }
#endif
                    exit(1);
                    break;
                case SDL_MOUSEMOTION:
                    NUII->NJS->mouseMove(event.motion.x, event.motion.y - kNativeTitleBarHeight,
                                   event.motion.xrel, event.motion.yrel);
                    break;
                case SDL_MOUSEWHEEL:
                {
                    int cx, cy;
                    SDL_GetMouseState(&cx, &cy);
                    NUII->NJS->mouseWheel(event.wheel.x, event.wheel.y, cx, cy - kNativeTitleBarHeight);
                    break;
                }
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                    NUII->NJS->mouseClick(event.button.x, event.button.y - kNativeTitleBarHeight,
                                    event.button.state, event.button.button);
                break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    int keyCode = 0;
                    
                    int mod = 0;
                    if (
                        (&event.key)->keysym.sym == SDLK_r &&
                        event.key.keysym.mod & KMOD_CTRL && event.type == SDL_KEYDOWN) {
                        if (++nrefresh > 1) {
                            break;
                        }
                        printf("\n\n=======Refresh...=======\n");
                        //[console clear];
                        delete NUII->NJS;
#ifdef NATIVE_USE_GTK
                        while (gtk_events_pending ()) {
                            gtk_main_iteration();
                        }
#endif
                        //printf("\n\n=======Restarting...=====\n");
                        glClearColor(1, 1, 1, 0);
                        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                        
                        NUII->NJS = new NativeJS(NUII->getWidth(),
                            NUII->getHeight(), NUII, NUII->gnet);

                        NativeNML *nml = new NativeNML(NUII->gnet);
                        nml->setNJS(NUII->NJS);
                        nml->loadFile("index.nml");
                        //SDL_GL_SwapBuffers();
                        break;
                    }
                    if (event.key.keysym.sym >= 97 && event.key.keysym.sym <= 122) {
                        keyCode = event.key.keysym.sym - 32;
                    } else {
                        keyCode = event.key.keysym.sym;
                    }
                    
                    if (event.key.keysym.mod & KMOD_SHIFT) {
                        mod |= NATIVE_KEY_SHIFT;
                    }
                    if (event.key.keysym.mod & KMOD_ALT) {
                        mod |= NATIVE_KEY_ALT;
                    }
                    if (event.key.keysym.mod & KMOD_CTRL) {
                        mod |= NATIVE_KEY_CTRL;
                    }
                    
                    NUII->NJS->keyupdown(keyCode, mod, event.key.state, event.key.repeat);
                    /*printf("Mapped to %d\n", keyCode);
                    printf("Key : %d %d %d %d %d uni : %d\n", event.key.keysym.sym,
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
        if (ttfps%20 == 0) {
            NUII->NJS->gc();
        }
        /*
        if (NUII->currentCursor != NativeX11UIInterface::NOCHANGE) {
            switch(NUII->currentCursor) {
                case NativeX11UIInterface::ARROW:
                    [[NSCursor arrowCursor] set];
                    break;
                case NativeX11UIInterface::BEAM:
                    [[NSCursor IBeamCursor] set];
                    break;
                case NativeX11UIInterface::CROSS:
                    [[NSCursor crosshairCursor] set];
                    break;
                case NativeX11UIInterface::POINTING:
                    [[NSCursor pointingHandCursor] set];
                    break;
                case NativeX11UIInterface::CLOSEDHAND:
                    [[NSCursor closedHandCursor] set];
                    break;
                default:
                    break;
            }
            NUII->currentCursor = NativeX11UIInterface::NOCHANGE;
        }
        */

        NUII->NJS->callFrame();
        NUII->NJS->rootHandler->layerize(NULL, 0, 0, 1.0, NULL);
        NUII->NJS->postDraw();

        //NUII->getConsole()->flush();
        SDL_GL_SwapWindow(NUII->win);

    //}
    ttfps++;
    //NSLog(@"ret : %d for %d events", (tend - tstart), nevents);
    return 16;
}

static int NativeProcessUI(void *arg)
{
    return NativeEvents((NativeX11UIInterface *)arg);
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
#endif

static void NativeDoneExtracting(void *closure, const char *fpath)
{
    NativeX11UIInterface *ui = (NativeX11UIInterface *)closure;
    chdir(fpath);
    printf("Changing directory to : %s\n", fpath);
    NativeNML *nml = new NativeNML(ui->gnet);
    nml->setNJS(ui->NJS);
    nml->loadFile("./index.nml");
}

bool NativeX11UIInterface::runApplication(const char *path)
{
    FILE *main = fopen("main.js", "r");
    if (main != NULL) {
        fclose(main);
        if (!this->createWindow(kNativeWidth, kNativeHeight+kNativeTitleBarHeight)) {
            return false;
        }

        NativeNML *nml = new NativeNML(this->gnet);
        nml->setNJS(this->NJS);
        nml->loadFile("index.nml");

        return true;
    } else {
        NativeApp *app = new NativeApp(path);
        if (app->open()) {
            if (!this->createWindow(app->getWidth(), app->getHeight()+kNativeTitleBarHeight)) {
                return false;
            }
            this->setWindowTitle(app->getTitle());

            app->runWorker(this->gnet);
            /*if (!(fsize = app->extractFile("main.js", NativeExtractMain, this)) ||
                fsize > 1024L*1024L*5) {

                return false;
            }*/

            char *uidpath = (char *)malloc(sizeof(char) *
                                (strlen(app->getUDID()) + 16));
            sprintf(uidpath, "%s.content/", app->getUDID());
            
            app->extractApp(uidpath, NativeDoneExtracting, this);
            free(uidpath);
            /*this->mainjs.buf = (char *)malloc(fsize);
            this->mainjs.len = fsize;
            this->mainjs.offset = 0;

            printf("Start looking for main.js of size : %ld\n", fsize);*/
            return true;
        }
    }
    return false;
}

NativeX11UIInterface::NativeX11UIInterface()
{
    this->width = 0;
    this->height = 0;

    this->currentCursor = NOCHANGE;
    this->NJS = NULL;
}

bool NativeX11UIInterface::createWindow(int width, int height)
{
    SDL_GLContext contexteOpenGL;
    Window *window;

    if (SDL_Init( SDL_INIT_EVERYTHING | SDL_INIT_TIMER) == -1)
    {
        printf( "Can't init SDL:  %s\n", SDL_GetError( ));
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0 );
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    win = SDL_CreateWindow("Native - Running", 100, 100,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL/* | SDL_WINDOW_FULLSCREEN*/);

    if (win == NULL) {
        printf("Cant create window (SDL)\n");
        return false;
    }

    this->width = width;
    this->height = height;

    window = NativeX11Window(win);

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

    contexteOpenGL = SDL_GL_CreateContext(win);
    SDL_StartTextInput();

    if (SDL_GL_SetSwapInterval(kNativeVSYNC) == -1) {
        printf("Cant vsync\n");
    }

    glViewport(0, 0, width, height);

#ifdef NATIVE_USE_GTK
    gtk_init(0, NULL);
#endif

    //NJS = new NativeJS(kNativeWidth, kNativeHeight);
    console = new NativeUIX11Console();
    gnet = native_netlib_init();
    NJS = new NativeJS(width, height, this, gnet);

    //NJS->LoadApplication("./demo.npa");

    return true;
}

void NativeX11UIInterface::setCursor(CURSOR_TYPE type)
{
    this->currentCursor = type;
}

void NativeX11UIInterface::setWindowTitle(const char *name)
{
    SDL_SetWindowTitle(win, (*name == '\0' ? "-" : name));
}

void NativeX11UIInterface::openFileDialog(const char const *files[],
    void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg)
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
        char name[256];
        memset(name, 0, 256);
        int nameLength = 0;
        filter = gtk_file_filter_new();
        for (int i = 0; files[i] != NULL; i++) {
            gtk_file_filter_add_pattern(filter, files[i]);
            nameLength += strlen(files[i])+1;
            if (nameLength < 256) {
                strcat(name, files[i]);
                strcat(name, " ");
            }
        }
        if (nameLength > 0) {
            gtk_file_filter_set_name(GTK_FILE_FILTER(filter), (const gchar *)name); 
        }

        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), GTK_FILE_FILTER(filter));
    }

    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        GSList *filenames, *list;
        filenames = gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER(dialog));      
        guint len = g_slist_length(filenames);

        const char **lst = (const char **)malloc(sizeof(char **) * len);

        if (!lst) {
            return;
        }

        list = filenames;
        int i = 0;
        while (list) {
            if (list->data) {
                lst[i] = strdup((const char *)list->data);
                g_free (list->data);
                i++;
            } 
            list = list->next;
        }

        g_slist_free(filenames);

        gtk_widget_destroy(dialog);

        while (gtk_events_pending ()) {
            gtk_main_iteration();
        }

        cb(arg, lst, i);

        free(lst);
    }
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

    printf("setting titlebar color\n");

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
    add_timer(&gnet->timersng, 1, NativeProcessUI, (void *)this);
    
    events_loop(gnet);  
}

NativeUIX11Console::NativeUIX11Console () 
    : isHidden(false), needFlush(false)
{
}

void NativeUIX11Console::log(const char *str)
{
    if (strcmp("\n", str) == 0) {
        printf("\n");
    } else {
        printf("[CONSOLE] %s", str);
    }
}

void NativeUIX11Console::show()
{
}

void NativeUIX11Console::hide()
{
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
