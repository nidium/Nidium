#import "CocoaUIInterface.h"

#import <sys/stat.h>
#import <objc/runtime.h>

#import <Cocoa/Cocoa.h>

#import <native_netlib.h>

#import <SDL.h>
#import <SDL_opengl.h>
#import <SDL_syswm.h>
#import "SDL_keycode_translate.h"

#import <Core/Messages.h>
#import <Core/Path.h>
#import <Binding/NidiumJS.h>

#import <Context.h>
#import <Skia.h>
#import <NML.h>
#import <App.h>
#import <JSWindow.h>

#import "Macros.h"
#import "System.h"
#import "UIConsole.h"
#import "DragNSView.h"

#define kNativeWidth 1024
#define kNativeHeight 768

#define kNativeTitleBarHeight 0

#define kNativeVSYNC 1

@interface NativeCocoaUIInterfaceWrapper: NSObject {
    NativeCocoaUIInterface *base;
}

- (NSMenu *) renderSystemTray;
- (void) menuClicked:(id)ev;
- (id) initWithUI:(NativeCocoaUIInterface *)ui;

@end

uint64_t ttfps = 0;

static __inline__ void ConvertNSRect(NSRect *r)
{
    r->origin.y = CGDisplayPixelsHigh(kCGDirectMainDisplay) - r->origin.y - r->size.height;
}

static NSWindow *NativeCocoaWindow(SDL_Window *win)
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(win, &info);

    return (NSWindow *)info.info.cocoa.window;
}

int NativeEvents(NativeCocoaUIInterface *NUII)
{
    SDL_Event event;
    int nrefresh = 0;
    //while(1) {
    int nevents = 0;
        while(SDL_PollEvent(&event)) {
            if (NUII->m_NativeCtx) {
                NUII->makeMainGLCurrent();
            }
            NativeJSwindow *window = NULL;
            if (NUII->m_NativeCtx) {
                window = NativeJSwindow::GetObject(NUII->m_NativeCtx->getNJS());
            }
            nevents++;
            switch(event.type) {
                case SDL_WINDOWEVENT:
                    if (window) {
                        switch (event.window.event) {
                            case SDL_WINDOWEVENT_SIZE_CHANGED:
                                //fprintf(stout, "Size changed ??? %d %d\n", event.window.data1, event.window.data2);
                                //window->resized(event.window.data1, event.window.data2);
                                break;
                            case SDL_WINDOWEVENT_RESIZED:
                                //fprintf(stdout, "Resized\n");
                                //window->resized(event.window.data1, event.window.data2);
                                break;
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
                    if (window && event.text.text) {
                        window->textInput(event.text.text);
                    }
                    break;
                case SDL_USEREVENT:
                    break;
                case SDL_QUIT:
                    if (window && !window->onClose()) {
                        break;
                    }
                    NUII->stopApplication();
                    SDL_Quit();
                    [[NSApplication sharedApplication] terminate:nil];
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

                    if (event.key.keysym.sym >= 97 && event.key.keysym.sym <= 122) {
                        keyCode = event.key.keysym.sym - 32;
                    } else {
                        keyCode = SDL_KEYCODE_TO_DOMCODE(event.key.keysym.sym);
                    }

                    if (event.key.keysym.mod & KMOD_SHIFT || SDL_KEYCODE_GET_CODE(keyCode) == 16) {
                        mod |= Nidium::Binding::NIDIUM_KEY_SHIFT;
                    }
                    if (event.key.keysym.mod & KMOD_ALT || SDL_KEYCODE_GET_CODE(keyCode) == 18) {
                        mod |= Nidium::Binding::NIDIUM_KEY_ALT;
                    }
                    if (event.key.keysym.mod & KMOD_CTRL || SDL_KEYCODE_GET_CODE(keyCode) == 17) {
                        mod |= Nidium::Binding::NIDIUM_KEY_CTRL;
                    }
                    if (event.key.keysym.mod & KMOD_GUI || SDL_KEYCODE_GET_CODE(keyCode) == 91) {
                        mod |= Nidium::Binding::NIDIUM_KEY_META;
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
        if (ttfps%300 == 0 && NUII->m_NativeCtx != NULL) {
            NUII->m_NativeCtx->getNJS()->gc();
        }
        if (NUII->m_CurrentCursor != NativeCocoaUIInterface::NOCHANGE) {
            switch(NUII->m_CurrentCursor) {
                case NativeCocoaUIInterface::ARROW:
                    [[NSCursor arrowCursor] set];
                    break;
                case NativeCocoaUIInterface::BEAM:
                    [[NSCursor IBeamCursor] set];
                    break;
                case NativeCocoaUIInterface::CROSS:
                    [[NSCursor crosshairCursor] set];
                    break;
                case NativeCocoaUIInterface::POINTING:
                    [[NSCursor pointingHandCursor] set];
                    break;
                case NativeCocoaUIInterface::CLOSEDHAND:
                    [[NSCursor closedHandCursor] set];
                    break;
                case NativeCocoaUIInterface::RESIZELEFTRIGHT:
                    [[NSCursor resizeLeftRightCursor] set];
                    break;
                case NativeCocoaUIInterface::HIDDEN:
                    SDL_ShowCursor(0);
                    break;
                default:
                    break;
            }
            NUII->m_CurrentCursor = NativeCocoaUIInterface::NOCHANGE;
        }

        //glUseProgram(0);
        if (NUII->m_NativeCtx) {
            NUII->makeMainGLCurrent();
            NUII->m_NativeCtx->frame(true);
        }
        if (NUII->getConsole()) {
            NUII->getConsole()->flush();
        }

        if (NUII->getFBO() != 0 && NUII->m_NativeCtx) {
            //glFlush();
            //glFinish();
            glReadBuffer(GL_COLOR_ATTACHMENT0);

            glReadPixels(0, 0, NUII->getWidth(), NUII->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, NUII->getFrameBufferData());
            uint8_t *pdata = NUII->getFrameBufferData();

            NUII->m_NativeCtx->rendered(pdata, NUII->getWidth(), NUII->getHeight());
        } else {
            NUII->makeMainGLCurrent();
            SDL_GL_SwapWindow(NUII->m_Win);
        }

        //NSLog(@"Swap : %d\n", SDL_GetTicks()-s);

    //}
    ttfps++;
    //NSLog(@"ret : %d for %d events", (tend - tstart), nevents);
    return 16;
}

/*
    TODO: useless indirection?
*/
static int NativeProcessUI(void *arg)
{
    return NativeEvents((NativeCocoaUIInterface *)arg);
}

static int NativeProcessSystemLoop(void *arg)
{
    SDL_PumpEvents();
    NativeCocoaUIInterface *ui = (NativeCocoaUIInterface *)arg;

    if (ui->m_NativeCtx) {
        ui->makeMainGLCurrent();
    }
    return 4;
}

void NativeCocoaUIInterface_onNMLLoaded(void *arg)
{
    NativeCocoaUIInterface *UI = (NativeCocoaUIInterface *)arg;
    UI->onNMLLoaded();
}

#if 0
static bool NativeExtractMain(const char *buf, int len,
    size_t offset, size_t total, void *user)
{
    NativeCocoaUIInterface *UI = (NativeCocoaUIInterface *)user;

    memcpy(UI->m_m_Mainjs.buf+UI->m_m_Mainjs.offset, buf, len);
    UI->m_m_Mainjs.offset += len;

    if (offset == total) {
        if (UI->NJS->LoadScriptContent(UI->m_m_Mainjs.buf, total, "main.js")) {
            UI->NJS->Loaded();

        }
    }

    return true;
}
#endif

static void NativeDoneExtracting(void *closure, const char *fpath)
{
    NativeCocoaUIInterface *ui = (NativeCocoaUIInterface *)closure;
    if (chdir(fpath) != 0) {
        fprintf(stderr, "Cant enter cache directory (%d)\n", errno);
        return;
    }
    fprintf(stdout, "Changing directory to : %s\n", fpath);

    ui->m_Nml = new Nidium::NML::NML(ui->m_Gnet);
    ui->m_Nml->loadFile("./index.nml", NativeCocoaUIInterface_onNMLLoaded, ui);
}

void NativeCocoaUIInterface::log(const char *buf)
{
    if (this->m_Console && !this->m_Console->m_IsHidden) {
        this->m_Console->log(buf);
    } else {
        fwrite(buf, sizeof(char), strlen(buf), stdout);
        fflush(stdout);
    }
}

void NativeCocoaUIInterface::logf(const char *format, ...)
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

void NativeCocoaUIInterface::vlog(const char *format, va_list ap)
{
    char *buff;
    int len;

    len = vasprintf(&buff, format, ap);

    this->log(buff);

    free(buff);
}

void NativeCocoaUIInterface::logclear()
{
    if (this->m_Console && !this->m_Console->m_IsHidden) {
        this->m_Console->clear();
    }
}

void NativeCocoaUIInterface::onNMLLoaded()
{
    if (!this->createWindow(
        this->m_Nml->getMetaWidth(),
        this->m_Nml->getMetaHeight()+kNativeTitleBarHeight)) {

        return;
    }

    this->setWindowTitle(this->m_Nml->getMetaTitle());
}

void NativeCocoaUIInterface::stopApplication()
{
    [this->m_DragNSView setResponder:nil];
    this->disableSysTray();
    m_SystemMenu.deleteItems();

    if (this->m_Nml) {
        delete this->m_Nml;
        this->m_Nml = NULL;
    }
    if (this->m_NativeCtx) {
        delete this->m_NativeCtx;
        this->m_NativeCtx = NULL;
        Nidium::Core::Messages::DestroyReader();
    }

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    /* Also clear the front buffer */
    SDL_GL_SwapWindow(this->m_Win);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void NativeCocoaUIInterface::restartApplication(const char *path)
{
    this->stopApplication();
    this->runApplication(path == NULL ? this->m_FilePath : path);
}

bool NativeCocoaUIInterface::runJSWithoutNML(const char *path, int width, int height)
{
    Nidium::Core::Messages::InitReader(m_Gnet);
    if (path != this->m_FilePath) {
        if (this->m_FilePath) {
            free(this->m_FilePath);
        }
        this->m_FilePath = strdup(path);
    }
    if (strlen(path) < 5) {
        return false;
    }

    if (!this->createWindow(
        width, height+kNativeTitleBarHeight)) {
        return false;
    }

    this->setWindowTitle("Nidium");

    NativeJS::InitNet(m_Gnet);

    Nidium::Core::Path jspath(path);

    Nidium::Core::Path::CD(jspath.dir());
    Nidium::Core::Path::Chroot(jspath.dir());

    m_NativeCtx->getNJS()->LoadScript(path);

    return true;
}

bool NativeCocoaUIInterface::runApplication(const char *path)
{
    Nidium::Core::Messages::InitReader(m_Gnet);

    if (path != this->m_FilePath) {
        if (this->m_FilePath) {
            free(this->m_FilePath);
        }
        this->m_FilePath = strdup(path);
    }
    if (strlen(path) < 5) {
        return false;
    }
    //    FILE *main = fopen("index.nml", "r");
    const char *ext = &path[strlen(path)-4];

    if (strncasecmp(ext, ".npa", 4) == 0) {
        FILE *main = fopen(path, "r");
        if (main == NULL) {
            return false;
        }
        Nidium::NML::App *app = new Nidium::NML::App(path);
        if (app->open()) {
            if (!this->createWindow(app->getWidth(), app->getHeight()+kNativeTitleBarHeight)) {
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
            /*this->m_Mainjs.buf = (char *)malloc(fsize);
            this->m_Mainjs.len = fsize;
            this->m_Mainjs.offset = 0;

            fprintf(stdout, "Start looking for main.js of size : %ld\n", fsize);*/
            return true;
        } else {
            delete app;
        }
    } else {
        this->m_Nml = new Nidium::NML::NML(this->m_Gnet);
        this->m_Nml->loadFile(path, NativeCocoaUIInterface_onNMLLoaded, this);

        return true;
    }
    return false;
}

NativeCocoaUIInterface::NativeCocoaUIInterface() :
    m_StatusItem(NULL)
{
    this->m_Width = 0;
    this->m_Height = 0;
    this->m_Initialized = false;
    this->m_Nml = NULL;
    this->m_FilePath = NULL;
    this->m_Console = NULL;

    this->m_DragNSView = nil;

    this->m_CurrentCursor = NOCHANGE;
    this->m_NativeCtx = NULL;

    m_Wrapper = [[NativeCocoaUIInterfaceWrapper alloc] initWithUI:this];

    m_Gnet = native_netlib_init();
}

NativeUICocoaConsole *NativeCocoaUIInterface::getConsole(bool create, bool *created) {
    if (created) *created = false;
    if (this->m_Console == NULL && create) {
        this->m_Console = new NativeUICocoaConsole;
        if (created) *created = true;
    }
    return this->m_Console;
}

bool NativeCocoaUIInterface::createWindow(int width, int height)
{
    if (!this->m_Initialized) {
        NSWindow *window;
        SDL_GLContext contexteOpenGL;

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == -1)
        {
            fprintf(stderr, "Can't init SDL:  %s\n", SDL_GetError( ));
            return false;
        }
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, true);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0 );
        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

        //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        m_Win = SDL_CreateWindow("nidium", 100, 100,
            width, height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN*/);

        if (m_Win == NULL) {
            fprintf(stderr, "Cant create window (SDL)\n");
            return false;
        }

        this->m_Width = width;
        this->m_Height = height;

        window = NativeCocoaWindow(m_Win);

        this->m_DragNSView = [[NativeDragNSView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
        [[window contentView] addSubview:this->m_DragNSView];

        this->patchSDLView([window contentView]);

        [window setCollectionBehavior:
                 NSWindowCollectionBehaviorFullScreenPrimary];

        initControls();

        [window setFrameAutosaveName:@"nativeMainWindow"];
        if (kNativeTitleBarHeight != 0) {
            [window setStyleMask:NSTexturedBackgroundWindowMask|NSTitledWindowMask];
        }

#if NIDIUM_ENABLE_HIDPI
        NSView *openglview = [window contentView];
        [openglview setWantsBestResolutionOpenGLSurface:YES];
#endif
        contexteOpenGL = SDL_GL_CreateContext(m_Win);
        m_MainGLCtx = contexteOpenGL;

        if (contexteOpenGL == NULL) {
            NLOG("Failed to create OpenGL context : %s", SDL_GetError());
        }
        SDL_StartTextInput();

        if (SDL_GL_SetSwapInterval(kNativeVSYNC) == -1) {
            NLOG("Cant vsync");
        }

        this->m_Initialized = true;
        glViewport(0, 0, width, height);

        int depth;
        glGetIntegerv(GL_DEPTH_BITS, &depth);

        NLOG("[DEBUG] OpenGL Context at %p", contexteOpenGL);
        NLOG("[DEBUG] OpenGL %s", glGetString(GL_VERSION));
        NLOG("[DEBUG] GLSL Version : %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
        NLOG("[DEBUG] Deph buffer %i", depth);

    } else {
        //this->patchSDLView([NativeCocoaWindow(m_Win) contentView]);
    }

    this->setWindowFrame(NATIVE_WINDOWPOS_UNDEFINED_MASK,
        NATIVE_WINDOWPOS_UNDEFINED_MASK, width, height);

    Nidium::NML::NativeContext::CreateAndAssemble(this, m_Gnet);

    [this->m_DragNSView setResponder:NativeJSwindow::GetObject(m_NativeCtx->getNJS())];

    return true;
}

const char *NativeCocoaUIInterface::getCacheDirectory() const
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString* cacheDir = [paths objectAtIndex:0];

    if (cacheDir) {
        NSString *path = [NSString stringWithFormat:@"%@/nidium/",cacheDir];
        const char *cpath = [path cStringUsingEncoding:NSASCIIStringEncoding];
        if (mkdir(cpath, 0777) == -1 && errno != EEXIST) {
            NLOG("Cant create cache directory %s", cpath);
            return NULL;
        }
        return cpath;
    }
    return NULL;
}

void NativeCocoaUIInterface::setCursor(CURSOR_TYPE type)
{
    this->m_CurrentCursor = type;
}

void NativeCocoaUIInterface::setWindowTitle(const char *name)
{
    SDL_SetWindowTitle(m_Win, (name == NULL || *name == '\0' ? "nidium" : name));
}

const char *NativeCocoaUIInterface::getWindowTitle() const
{
    return SDL_GetWindowTitle(m_Win);
}

void NativeCocoaUIInterface::setTitleBarRGBAColor(uint8_t r, uint8_t g,
    uint8_t b, uint8_t a)
{
    NSWindow *window = NativeCocoaWindow(m_Win);
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
}

void NativeCocoaUIInterface::initControls()
{
    NSWindow *window = NativeCocoaWindow(m_Win);
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
}

void NativeCocoaUIInterface::setWindowControlsOffset(double x, double y)
{
    NSWindow *window = NativeCocoaWindow(m_Win);
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
}

void NativeCocoaUIInterface::openFileDialog(const char *files[],
    void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg, int flags)
{
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

    if (files != NULL) {
        NSMutableArray *fileTypesArray = [NSMutableArray arrayWithCapacity:0];

        for (int i = 0; files[i] != NULL; i++) {
            [fileTypesArray addObject:[NSString stringWithCString:files[i] encoding:NSASCIIStringEncoding]];
        }
        [openDlg setAllowedFileTypes:fileTypesArray];
    }

    [openDlg setCanChooseFiles:(flags & kOpenFile_CanChooseFile ? YES : NO)];
    [openDlg setCanChooseDirectories:(flags & kOpenFile_CanChooseDir ? YES : NO)];
    [openDlg setAllowsMultipleSelection:(flags & kOpenFile_AlloMultipleSelection ? YES : NO)];

    //[openDlg runModal];

#if 1
    // TODO: set a flag so that nidium can't be refreshed and unset after the block is called

    [openDlg beginSheetModalForWindow:NativeCocoaWindow(m_Win) completionHandler:^(NSInteger result) {
        if (result == NSFileHandlingPanelOKButton) {
            uint32_t len = [openDlg.URLs count];
            const char **lst = (const char **)malloc(sizeof(char **) * len);

            int i = 0;
            for (NSURL *url in openDlg.URLs) {
                lst[i] = [[url relativePath] UTF8String];
                i++;
            }

            cb(arg, lst, i);

            free(lst);
        } else {
            cb(arg, NULL, 0);
        }
    }];

    this->makeMainGLCurrent();
#endif
}

void NativeCocoaUIInterface::runLoop()
{
    APE_timer_create(m_Gnet, 1, NativeProcessUI, (void *)this);
    APE_timer_create(m_Gnet, 1, NativeProcessSystemLoop, (void *)this);

    APE_loop_run(m_Gnet);
}

void NativeCocoaUIInterface::setClipboardText(const char *text)
{
    SDL_SetClipboardText(text);
}

char *NativeCocoaUIInterface::getClipboardText()
{
    return SDL_GetClipboardText();
}

void NativeCocoaUIInterface::setWindowFrame(int x, int y, int w, int h)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSWindow *nswindow = NativeCocoaWindow(this->m_Win);

    this->m_Width = w;
    this->m_Height = h;
    CGRect oldframe = [nswindow frame];

    int screen_width, screen_height;
    if (x == NATIVE_WINDOWPOS_CENTER_MASK || y == NATIVE_WINDOWPOS_CENTER_MASK) {
        this->getScreenSize(&screen_width, &screen_height);

        if (x == NATIVE_WINDOWPOS_CENTER_MASK) {
            x = (screen_width - w) / 2;
        }
        if (y == NATIVE_WINDOWPOS_CENTER_MASK) {
            y = (screen_height - h) / 2;
        }
    }

    if (x == NATIVE_WINDOWPOS_UNDEFINED_MASK) {
        x = oldframe.origin.x;
    }
    if (y == NATIVE_WINDOWPOS_UNDEFINED_MASK) {
        y = oldframe.origin.y;
    }

    CGRect newframe = [nswindow frameRectForContentRect:CGRectMake(x, y, w, h)];
    [nswindow setFrame:newframe display:YES];

    [pool drain];
}

void NativeCocoaUIInterface::setWindowSize(int w, int h)
{
    NSLog(@"set window size");
    NSWindow *nswindow = NativeCocoaWindow(this->m_Win);

    NSSize size;
    size.width = w;
    size.height = h;

    this->m_Width = w;
    this->m_Height = h;

    NSRect frame = [nswindow frame];
    frame.origin.y += frame.size.height;
    frame.origin.y -= h;

    frame.size = size;

    //[nswindow setFrame:frame display:YES];

    SDL_SetWindowSize(this->m_Win, w, h);
    //[(NSOpenGLContext *)this->getGLContext() update];
    //[nswindow setContentSize:size];
}

void NativeCocoaUIInterface::alert(const char *message)
{

}
#if 0
bool NativeCocoaUIInterface::makeMainGLCurrent()
{
    [((NSOpenGLContext *)m_mainGLCtx) makeCurrentContext];

    return true;
}

SDL_GLContext NativeCocoaUIInterface::getCurrentGLContext()
{
    return (SDL_GLContext)[NSOpenGLContext currentContext];
}
#endif

#if 0
bool NativeCocoaUIInterface::makeGLCurrent(SDL_GLContext ctx)
{
    [((NSOpenGLContext *)ctx) makeCurrentContext];

    return true;
}
#endif

static const char *drawRect_Associated_obj = "_NativeUIInterface";

@interface NSPointer : NSObject
{
@public
    void *m_Ptr;
}

- (id) initWithPtr:(void *)ptr;
@end

@implementation NSPointer
- (id) initWithPtr:(void *)ptr
{
    self = [super init];
    if (!self) return nil;

    self->m_Ptr = ptr;

    return self;
}
@end
@interface NativeDrawRectResponder : NSView
    - (void) drawRect:(NSRect)dirtyRect;
@end

@implementation NativeDrawRectResponder
- (void) drawRect:(NSRect)dirtyRect
{
    NSPointer *idthis = objc_getAssociatedObject(self, drawRect_Associated_obj);
    NativeCocoaUIInterface *UI = (NativeCocoaUIInterface *)idthis->m_Ptr;
    Nidium::NML::NativeContext *ctx = UI->getNativeContext();

    if (ctx && ctx->isSizeDirty()) {
        [(NSOpenGLContext *)UI->getGLContext() update];
        ctx->sizeChanged(UI->getWidth(), UI->getHeight());
    }
}
@end

void NativeCocoaUIInterface::patchSDLView(NSView *sdlview)
{
    Class SDLView = NSClassFromString(@"SDLView");
    Method drawRectNewMeth = class_getInstanceMethod([NativeDrawRectResponder class], @selector(drawRect:));
    IMP drawRectNewImp = method_getImplementation(drawRectNewMeth);
    const char *types = method_getTypeEncoding(drawRectNewMeth);

    class_replaceMethod(SDLView, @selector(drawRect:), drawRectNewImp, types);

    NSPointer *idthis = [[NSPointer alloc] initWithPtr:this];

    objc_setAssociatedObject(sdlview, drawRect_Associated_obj, idthis, OBJC_ASSOCIATION_RETAIN_NONATOMIC);

    [idthis release];
}

void NativeCocoaUIInterface::enableSysTray(const void *imgData,
    size_t imageDataSize)
{
#if 0
    m_StatusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain];


    NSImage *icon = [NSApp applicationIconImage];
    [icon setScalesWhenResized:YES];
    [icon setSize: NSMakeSize(20.0, 20.0)];

    m_StatusItem.image = icon;
    m_StatusItem.highlightMode = YES;

    NSMenu *stackMenu = [[NSMenu alloc] initWithTitle:@""];

    NSMenuItem *menuOpen =
        [[NSMenuItem alloc] initWithTitle:@"Open" action:nil keyEquivalent:@""];
    NSMenuItem *menuClose =
        [[NSMenuItem alloc] initWithTitle:@"Close" action:nil keyEquivalent:@""];

    [stackMenu addItem:menuOpen];
    [stackMenu addItem:menuClose];

    [menuOpen setEnabled:YES];
    [menuClose setEnabled:YES];

    [m_StatusItem setMenu:stackMenu];
    m_StatusItem.title = @"";
#endif

    this->renderSystemTray();
}

void NativeCocoaUIInterface::disableSysTray()
{
    if (!m_StatusItem) {
        return;
    }
    [[NSStatusBar systemStatusBar] removeStatusItem:m_StatusItem];
    [m_StatusItem release];

    m_StatusItem = NULL;
}

void NativeCocoaUIInterface::quit()
{
    this->stopApplication();
    SDL_Quit();
    [[NSApplication sharedApplication] terminate:nil];
}

void NativeCocoaUIInterface::hideWindow()
{
    if (!m_Hidden) {
        m_Hidden = true;
        SDL_HideWindow(m_Win);

        /* Hide the Application (Dock, etc...) */
        [NSApp setActivationPolicy: NSApplicationActivationPolicyAccessory];

        set_timer_to_low_resolution(&this->m_Gnet->timersng, 1);
    }
}

void NativeCocoaUIInterface::showWindow()
{
    if (m_Hidden) {
        m_Hidden = false;
        SDL_ShowWindow(m_Win);

        [NSApp setActivationPolicy: NSApplicationActivationPolicyRegular];

        set_timer_to_low_resolution(&this->m_Gnet->timersng, 0);
    }
}

void NativeCocoaUIInterface::renderSystemTray()
{
    NativeSystemMenuItem *item = m_SystemMenu.items();
    if (!item) {
        return;
    }

    if (!m_StatusItem) {
        m_StatusItem = [[[NSStatusBar systemStatusBar] statusItemWithLength:NSVariableStatusItemLength] retain];
        m_StatusItem.highlightMode = YES;

        size_t icon_len, icon_width, icon_height;
        const uint8_t *icon_custom = m_SystemMenu.getIcon(&icon_len,
                                        &icon_width, &icon_height);

        NSImage *icon;

        if (!icon_len || !icon_custom) {
            icon = [NSApp applicationIconImage];
            [icon setSize: NSMakeSize(20.0, 20.0)];
        } else {

            NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:(unsigned char **)&icon_custom
                                                                            pixelsWide:icon_width
                                                                            pixelsHigh:icon_height
                                                                            bitsPerSample:8
                                                                            samplesPerPixel:4
                                                                            hasAlpha:YES
                                                                            isPlanar:NO
                                                                            colorSpaceName:NSDeviceRGBColorSpace
                                                                            bitmapFormat:NSAlphaNonpremultipliedBitmapFormat
                                                                            bytesPerRow:icon_width*4
                                                                            bitsPerPixel:32];

            icon = [[[NSImage alloc] initWithSize:NSMakeSize(32.f, 32.f)] autorelease];

            [icon addRepresentation:rep];
        }
        m_StatusItem.image = icon;
    }

    NSMenu *stackMenu = [m_Wrapper renderSystemTray];

    [m_StatusItem setMenu:stackMenu];
}


@implementation NativeCocoaUIInterfaceWrapper

- (id) initWithUI:(NativeCocoaUIInterface *)ui
{
    if (self = [super init]) {
        self->base = ui;
    }
    return self;
}

- (void) menuClicked:(id)sender
{
    NSString *identifier = [sender representedObject];

    NativeJSwindow *window = NativeJSwindow::GetObject(self->base->m_NativeCtx->getNJS());
    if (window) {
        window->systemMenuClicked([identifier cStringUsingEncoding:NSUTF8StringEncoding]);
    }
}

- (NSMenu *) renderSystemTray
{
    NativeCocoaUIInterface *ui = self->base;
    NativeSystemMenu &m_SystemMenu = ui->getSystemMenu();

    NativeSystemMenuItem *item = m_SystemMenu.items();
    if (!item) {
        return nil;
    }

    NSMenu *stackMenu = [[[NSMenu alloc] initWithTitle:@""] retain];

    while (item) {
        NSString *title = [NSString stringWithCString:item->title() encoding:NSUTF8StringEncoding];
        NSString *identifier = [NSString stringWithCString:item->id() encoding:NSUTF8StringEncoding];
        NSMenuItem *curMenu;
        if ([title isEqualToString:@"-"]) {
            curMenu = [NSMenuItem separatorItem];
        } else {
            curMenu =
                [[[NSMenuItem alloc] initWithTitle:title action:@selector(menuClicked:) keyEquivalent:@""] autorelease];
        }
        [stackMenu addItem:curMenu];
        [curMenu setEnabled:YES];

        item = item->m_Next;
        curMenu.target = self;

        [curMenu setRepresentedObject:identifier];
    }

    return stackMenu;
}

@end
