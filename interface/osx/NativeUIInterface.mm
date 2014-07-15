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
#import <NativeMessages.h>
#import <NativePath.h>

#import <NativeNML.h>
#import <sys/stat.h>
#import "NativeSystem.h"
#import "SDL_keycode_translate.h"

#import "NativeDragNSView.h"
#import <objc/runtime.h>

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
            if (NUII->NativeCtx) {
                NUII->makeMainGLCurrent();
            }
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
                                //printf("Size changed ??? %d %d\n", event.window.data1, event.window.data2);
                                //window->resized(event.window.data1, event.window.data2);
                                break;
                            case SDL_WINDOWEVENT_RESIZED:
                                //printf("Resized\n");
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
            NUII->makeMainGLCurrent();
            NUII->NativeCtx->frame();
        }
        if (NUII->getConsole()) {
            NUII->getConsole()->flush();
        }

        if (NUII->getFBO() != 0 && NUII->NativeCtx) {
            //glFlush();
            //glFinish();
            glReadBuffer(GL_COLOR_ATTACHMENT0);

            glReadPixels(0, 0, NUII->getWidth(), NUII->getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, NUII->getFrameBufferData());
            uint8_t *pdata = NUII->getFrameBufferData();
            
            NUII->NativeCtx->rendered(pdata, NUII->getWidth(), NUII->getHeight());
        } else {
            SDL_GL_SwapWindow(NUII->win);
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

    if (ui->NativeCtx) {
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

    this->setWindowTitle(this->nml->getMetaTitle());
}

void NativeCocoaUIInterface::stopApplication()
{
    [this->dragNSView setResponder:nil];

    if (this->nml) {
        delete this->nml;
        this->nml = NULL;
    }
    if (this->NativeCtx) {
        delete this->NativeCtx;
        this->NativeCtx = NULL;
        NativeMessages::destroyReader();
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

bool NativeCocoaUIInterface::runJSWithoutNML(const char *path, int width, int height)
{
    NativeMessages::initReader(gnet);
    if (path != this->filePath) {
        if (this->filePath) {
            free(this->filePath);
        }
        this->filePath = strdup(path);
    }
    if (strlen(path) < 5) {
        return false;
    }

    if (!this->createWindow(
        width, height+kNativeTitleBarHeight)) {
        return false;
    }

    this->setWindowTitle("Nidium");

    NativeJS::initNet(gnet);

    NativePath jspath(path);

    NativePath::cd(jspath.dir());
    NativePath::chroot(jspath.dir());

    NativeCtx->getNJS()->LoadScript(path);

    return true;
}

bool NativeCocoaUIInterface::runApplication(const char *path)
{
    NativeMessages::initReader(gnet);

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

    this->dragNSView = nil;

    this->currentCursor = NOCHANGE;
    this->NativeCtx = NULL;

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
    if (!this->initialized) {
        NSWindow *window;
        SDL_GLContext contexteOpenGL;
        
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == -1)
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
            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL /*| SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN*/);

        if (win == NULL) {
            printf("Cant create window (SDL)\n");
            return false;
        }

        this->width = width;
        this->height = height;

        window = NativeCocoaWindow(win);

        this->dragNSView = [[NativeDragNSView alloc] initWithFrame:NSMakeRect(0, 0, width, height)];
        [[window contentView] addSubview:this->dragNSView];

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
        contexteOpenGL = SDL_GL_CreateContext(win);
        m_mainGLCtx = contexteOpenGL;
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
    } else {
        //this->patchSDLView([NativeCocoaWindow(win) contentView]);
    }

    this->setWindowFrame(NATIVE_WINDOWPOS_UNDEFINED_MASK,
        NATIVE_WINDOWPOS_UNDEFINED_MASK, width, height);

    NativeCtx = new NativeContext(this, this->nml, width, height, gnet);
    [this->dragNSView setResponder:NativeJSwindow::getNativeClass(NativeCtx->getNJS())];

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

    //[openDlg runModal];

#if 1
    // TODO: set a flag so that nidium can't be refreshed and unset after the block is called

    [openDlg beginSheetModalForWindow:NativeCocoaWindow(win) completionHandler:^(NSInteger result) {
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
    add_timer(&gnet->timersng, 1, NativeProcessUI, (void *)this);
    add_timer(&gnet->timersng, 1, NativeProcessSystemLoop, (void *)this);
    
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

void NativeCocoaUIInterface::setWindowFrame(int x, int y, int w, int h)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSWindow *nswindow = NativeCocoaWindow(this->win);
    
    this->width = w;
    this->height = h;
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
    NSWindow *nswindow = NativeCocoaWindow(this->win);

    NSSize size;
    size.width = w;
    size.height = h;
    
    this->width = w;
    this->height = h;

    NSRect frame = [nswindow frame];
    frame.origin.y += frame.size.height;
    frame.origin.y -= h;

    frame.size = size;

    //[nswindow setFrame:frame display:YES];

    SDL_SetWindowSize(this->win, w, h);
    //[(NSOpenGLContext *)this->getGLContext() update];
    //[nswindow setContentSize:size];
}

void NativeCocoaUIInterface::alert(const char *message)
{
    
}

bool NativeCocoaUIInterface::makeMainGLCurrent()
{
    [((NSOpenGLContext *)m_mainGLCtx) makeCurrentContext];
    
    return true;
}

SDL_GLContext NativeCocoaUIInterface::getCurrentGLContext()
{
    return (SDL_GLContext)[NSOpenGLContext currentContext];
}

bool NativeCocoaUIInterface::makeGLCurrent(SDL_GLContext ctx)
{
    [((NSOpenGLContext *)ctx) makeCurrentContext];

    return true;
}

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
    NativeContext *ctx = UI->getNativeContext();
    
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
