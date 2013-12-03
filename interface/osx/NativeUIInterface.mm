#import "NativeCocoaUIInterface.h"
#import "NativeUIConsole.h"
#import <NativeJS.h>
#import <NativeContext.h>
#import <NativeSkia.h>
#import <NativeApp.h>
#import <NativeJSWindow.h> 
#import <SDL.h>
#import <SDL_opengl.h>
#import <SDL_syswm.h>
#import <Cocoa/Cocoa.h>
#import <native_netlib.h>
#import <NativeMacros.h>

#import <NativeNML.h>
#import <sys/stat.h>
#import "NativeSystem.h"
#import "SDL_keycode_translate.h"

#define kNativeWidth 1024
#define kNativeHeight 768

#define kNativeTitleBarHeight 0

#define kNativeVSYNC 1

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
            NativeJSwindow *window = NULL;
            if (NUII->NativeCtx) {
                window = NativeJSwindow::getNativeClass(NUII->NativeCtx->getNJS());
            }
            nevents++;
            switch(event.type) {
                case SDL_WINDOWEVENT:
                    if (window) {
                        switch (event.window.event) {
                            case SDL_WINDOWEVENT_SIZE_CHANGED:
                                printf("Size changed ??? %d %d\n", event.window.data1, event.window.data2);
                                //window->resized(event.window.data1, event.window.data2);
                                break;
                            case SDL_WINDOWEVENT_RESIZED:
                                printf("Resized\n");
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
                    if (window) {
                        window->textInput(event.text.text);
                    }
                    break;
                case SDL_USEREVENT:
                    break;
                case SDL_QUIT:
                    NSLog(@"Quit?");
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
                                    event.button.state, event.button.button);
                    }
                break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    int keyCode = 0;
                    
                    int mod = 0;
                    if (
                        (&event.key)->keysym.sym == SDLK_r &&
                        event.key.keysym.mod & KMOD_GUI && event.key.keysym.mod & KMOD_SHIFT && event.type == SDL_KEYDOWN) {

                        NativeUICocoaConsole *console = NUII->getConsole();

                        if (console && !console->isHidden) {
                            console->clear();
                        }
                    }
                    else if (
                        (&event.key)->keysym.sym == SDLK_r &&
                        event.key.keysym.mod & KMOD_GUI && event.type == SDL_KEYDOWN) {
                        if (++nrefresh > 1) {
                            break;
                        }
                        if (NUII->NativeCtx) {
                            NUII->NativeCtx->getNJS()->gc();
                        }
                        NUII->restartApplication();
                        //SDL_GL_SwapBuffers();
                        break;
                    }
                    else if (
                        (&event.key)->keysym.sym == SDLK_d &&
                        event.key.keysym.mod & KMOD_GUI && event.type == SDL_KEYDOWN) {

                        bool created;
                        NativeUICocoaConsole *console = NUII->getConsole(true, &created);

                        if (!created) {
                            if (console->isHidden) {
                                console->show();
                            } else {
                                console->hide();
                            }
                        }
                    }
                    else if (
                        (&event.key)->keysym.sym == SDLK_u &&
                        event.key.keysym.mod & KMOD_GUI && event.type == SDL_KEYDOWN) {

                        NUII->stopApplication();
                        break;
                    }
                    if (event.key.keysym.sym >= 97 && event.key.keysym.sym <= 122) {
                        keyCode = event.key.keysym.sym - 32;
                    } else {
                        keyCode = SDL_KEYCODE_TO_DOMCODE(event.key.keysym.sym);
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
                    if (event.key.keysym.mod & KMOD_GUI) {
                        mod |= NATIVE_KEY_META;
                    }
                    if (window) {
                        window->keyupdown(SDL_KEYCODE_GET_CODE(keyCode), mod,
                            event.key.state, event.key.repeat,
                            SDL_KEYCODE_GET_LOCATION(keyCode));
                    }
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
        if (ttfps%300 == 0 && NUII->NativeCtx != NULL) {
            NUII->NativeCtx->getNJS()->gc();
        }
        if (NUII->currentCursor != NativeCocoaUIInterface::NOCHANGE) {
            switch(NUII->currentCursor) {
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
                case NativeCocoaUIInterface::HIDDEN:
                    SDL_ShowCursor(0);
                    break;
                default:
                    break;
            }
            NUII->currentCursor = NativeCocoaUIInterface::NOCHANGE;
        }
        //glUseProgram(0);
        if (NUII->NativeCtx) {
            NUII->NativeCtx->frame();
        }
        if (NUII->getConsole()) {
            NUII->getConsole()->flush();
        }
        SDL_GL_SwapWindow(NUII->win);

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
    NativeCocoaUIInterface *ui = (NativeCocoaUIInterface *)closure;
    if (chdir(fpath) != 0) {
        printf("Cant enter cache directory (%d)\n", errno);
        return;
    }
    printf("Changing directory to : %s\n", fpath);

    ui->nml = new NativeNML(ui->gnet);
    ui->nml->setNJS(ui->NativeCtx->getNJS());
    ui->nml->loadFile("./index.nml", NativeCocoaUIInterface_onNMLLoaded, ui);
}

void NativeCocoaUIInterface::log(const char *buf)
{
    if (this->console && !this->console->isHidden) {
        this->console->log(buf);
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

void NativeCocoaUIInterface::onNMLLoaded()
{
    if (!this->createWindow(
        this->nml->getMetaWidth(),
        this->nml->getMetaHeight()+kNativeTitleBarHeight)) {

        return;
    }
    this->nml->setNJS(this->NativeCtx->getNJS());
    this->setWindowTitle(this->nml->getMetaTitle());
}

void NativeCocoaUIInterface::stopApplication()
{
    if (this->nml) {
        delete this->nml;
        this->nml = NULL;
    }
    if (this->NativeCtx) {
        delete this->NativeCtx;
        this->NativeCtx = NULL;
    }

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    /* Also clear the front buffer */
    SDL_GL_SwapWindow(this->win);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);  
}

void NativeCocoaUIInterface::restartApplication(const char *path)
{
    this->stopApplication();
    this->runApplication(path == NULL ? this->filePath : path);
}

bool NativeCocoaUIInterface::runApplication(const char *path)
{
    if (path != this->filePath) {
        if (this->filePath) {
            free(this->filePath);
        }
        this->filePath = strdup(path);
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
        NativeApp *app = new NativeApp(path);
        if (app->open()) {
            if (!this->createWindow(app->getWidth(), app->getHeight()+kNativeTitleBarHeight)) {
                return false;
            }
            this->setWindowTitle(app->getTitle());

            app->runWorker(this->gnet);

            const char *cachePath = this->getCacheDirectory();
            char *uidpath = (char *)malloc(sizeof(char) *
                                (strlen(app->getUDID()) + strlen(cachePath) + 16));
            sprintf(uidpath, "%s%s.content/", cachePath, app->getUDID());
            
            app->extractApp(uidpath, NativeDoneExtracting, this);
            free(uidpath);
            /*this->mainjs.buf = (char *)malloc(fsize);
            this->mainjs.len = fsize;
            this->mainjs.offset = 0;

            printf("Start looking for main.js of size : %ld\n", fsize);*/
            return true;
        } else {
            delete app;
        }
    } else if (strncasecmp(ext, ".nml", 4) == 0) {
        this->nml = new NativeNML(this->gnet);
        this->nml->loadFile(path, NativeCocoaUIInterface_onNMLLoaded, this);

        return true;
    }
    return false;
}

NativeCocoaUIInterface::NativeCocoaUIInterface()
{
    this->width = 0;
    this->height = 0;
    this->initialized = false;
    this->nml = NULL;
    this->filePath = NULL;
    this->console = NULL;
    /* Set the current working directory relative to the .app */
    char parentdir[MAXPATHLEN];

    this->currentCursor = NOCHANGE;
    this->NativeCtx = NULL;

    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
    if (CFURLGetFileSystemRepresentation(url2, 1, (UInt8 *)parentdir, MAXPATHLEN)) {
        chdir(parentdir);   /* chdir to the binary app's parent */
    }
    CFRelease(url);
    CFRelease(url2);

    gnet = native_netlib_init();
}

NativeUICocoaConsole *NativeCocoaUIInterface::getConsole(bool create, bool *created) {
    if (created) *created = false;
    if (this->console == NULL && create) {
        this->console = new NativeUICocoaConsole;
        if (created) *created = true;
    }
    return this->console;
}

bool NativeCocoaUIInterface::createWindow(int width, int height)
{
    NSWindow *window;
    if (!this->initialized) {
        SDL_GLContext contexteOpenGL;
        
        if (SDL_Init( SDL_INIT_EVERYTHING) == -1)
        {
            printf( "Can't init SDL:  %s\n", SDL_GetError( ));
            return false;
        }
        //SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, true );
        //SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 );
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0 );
        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
        
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        
        win = SDL_CreateWindow("nidium", 100, 100,
            width, height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL/* | SDL_WINDOW_FULLSCREEN*/);

        if (win == NULL) {
            printf("Cant create window (SDL)\n");
            return false;
        }

        this->width = width;
        this->height = height;

        window = NativeCocoaWindow(win);

        //[center addObserver:self selector:@selector(windowBeingResized) name:NSWindowWillStartLiveResizeNotification object window];
/*
    NSNotificationCenter *center;
    NSWindow *window = data->nswindow;
    NSView *view = [window contentView];

    _data = data;
    observingVisible = YES;
    wasVisible = [window isVisible];

    center = [NSNotificationCenter defaultCenter];

    if ([window delegate] != nil) {
        [center addObserver:self selector:@selector(windowDidExpose:) name:NSWindowDidExposeNotification object:window];
        [center addObserver:self selector:@selector(windowDidMove:) name:NSWindowDidMoveNotification object:window];
        [center addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:window];
        [center addObserver:self selector:@selector(windowDidMiniaturize:) name:NSWindowDidMiniaturizeNotification object:window];
        [center addObserver:self selector:@selector(windowDidDeminiaturize:) name:NSWindowDidDeminiaturizeNotification object:window];
        [center addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:window];
        [center addObserver:self selector:@selector(windowDidResignKey:) name:NSWindowDidResignKeyNotification object:window];
    } else {
        [window setDelegate:self];
    }
*/

        [window setCollectionBehavior:
                 NSWindowCollectionBehaviorFullScreenPrimary];

        initControls();

        //[btn setFrame:CGRectMake(btn.frame.origin.x, btn.frame.origin.y-4, btn.frame.size.width, btn.frame.size.width)];

        //[window setBackgroundColor:[NSColor colorWithSRGBRed:0.0980 green:0.1019 blue:0.09411 alpha:1.]];

        [window setFrameAutosaveName:@"nativeMainWindow"];
        if (kNativeTitleBarHeight != 0) {
            [window setStyleMask:NSTexturedBackgroundWindowMask|NSTitledWindowMask];

            //[window setContentBorderThickness:32.0 forEdge:NSMinYEdge];
            //[window setOpaque:NO];
        }
        //[window setMovableByWindowBackground:NO];
        //[window setOpaque:NO]; // YES by default
        //[window setAlphaValue:0.5];
#if NIDIUM_ENABLE_HIDPI
        NSView *openglview = [window contentView];
        [openglview setWantsBestResolutionOpenGLSurface:YES];
#endif
        contexteOpenGL = SDL_GL_CreateContext(win);
        if (contexteOpenGL == NULL) {
            NLOG("Failed to create OpenGL context : %s", SDL_GetError());
        }
        SDL_StartTextInput();

        if (SDL_GL_SetSwapInterval(kNativeVSYNC) == -1) {
            NLOG("Cant vsync");
        }

        this->initialized = true;
        glViewport(0, 0, width, height);
        NLOG("[DEBUG] OpenGL %s", glGetString(GL_VERSION));
    }
    
    NativeCtx = new NativeContext(this, this->nml, width, height, gnet);
    window = NativeCocoaWindow(win);
#if 0
    id observation = [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidResizeNotification
    object:window queue:nil usingBlock:^(NSNotification *notif){
        NSRect rect = [window contentRectForFrameRect:[window frame]];
        int x, y, w, h;
        ConvertNSRect(&rect);
        x = (int)rect.origin.x;
        y = (int)rect.origin.y;
        w = (int)rect.size.width;
        h = (int)rect.size.height;
        NativeJSwindow::getNativeClass(NativeCtx->getNJS())->resized(w, h);
    }];
#endif
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
    this->currentCursor = type;
}

void NativeCocoaUIInterface::setWindowTitle(const char *name)
{
    SDL_SetWindowTitle(win, (name == NULL || *name == '\0' ? "nidium" : name));
}

const char *NativeCocoaUIInterface::getWindowTitle() const
{
    return SDL_GetWindowTitle(win);
}

void NativeCocoaUIInterface::setTitleBarRGBAColor(uint8_t r, uint8_t g,
    uint8_t b, uint8_t a)
{
    NSWindow *window = NativeCocoaWindow(win);
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
}

void NativeCocoaUIInterface::initControls()
{
    NSWindow *window = NativeCocoaWindow(win);
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
    NSWindow *window = NativeCocoaWindow(win);
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
    void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg)
{
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];

    if (files != NULL) {
        NSMutableArray *fileTypesArray = [NSMutableArray arrayWithCapacity:0];

        for (int i = 0; files[i] != NULL; i++) {
            [fileTypesArray addObject:[NSString stringWithCString:files[i] encoding:NSASCIIStringEncoding]];
        }
        [openDlg setAllowedFileTypes:fileTypesArray];
    }
    [openDlg setCanChooseFiles:YES];
    [openDlg setCanChooseDirectories:NO];
    [openDlg setAllowsMultipleSelection:YES];

    [openDlg beginSheetModalForWindow:NativeCocoaWindow(win) completionHandler:^(NSInteger result) {
        if (result == NSFileHandlingPanelOKButton) {
            uint32_t len = [openDlg.URLs count];
            const char **lst = (const char **)malloc(sizeof(char **) * len);

            int i = 0;
            for (NSURL *url in openDlg.URLs) {
                lst[i] = [[url relativePath] cStringUsingEncoding:NSASCIIStringEncoding];
                i++;
            }

            cb(arg, lst, i);

            free(lst);
        }
    }];

}

void NativeCocoaUIInterface::runLoop()
{
    add_timer(&gnet->timersng, 1, NativeProcessUI, (void *)this);
    
    events_loop(gnet);
}

void NativeCocoaUIInterface::setClipboardText(const char *text)
{
    SDL_SetClipboardText(text);
}

char *NativeCocoaUIInterface::getClipboardText()
{
    return SDL_GetClipboardText();
}

void NativeCocoaUIInterface::setWindowSize(int w, int h)
{
    //NativeJSwindow *window = NativeJSwindow::getNativeClass(NativeCtx->getNJS());
    printf("Calling changing size to SDL\n");
    SDL_SetWindowSize(win, w, h);
    this->width = w;
    this->height = h;
}

void NativeCocoaUIInterface::alert(const char *message)
{
    
}