#import "NativeCocoaUIInterface.h"
#import <NativeJS.h>
#import <NativeSkia.h>
#import <NativeApp.h>
#import <SDL2/SDL.h>
#import <SDL2/SDL_opengl.h>
#import <SDL2/SDL_syswm.h>
#import <Cocoa/Cocoa.h>
#import <native_netlib.h>


#define kNativeWidth 1024
#define kNativeHeight 768
#define kNativeVSYNC 0

uint32_t ttfps = 0;

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
                    NSLog(@"Quit?");
                    SDL_Quit();
                    [[NSApplication sharedApplication] terminate:nil];
                    break;
                case SDL_MOUSEMOTION:
                    NUII->NJS->mouseMove(event.motion.x, event.motion.y,
                                   event.motion.xrel, event.motion.yrel);
                    break;
                case SDL_MOUSEWHEEL:
                {
                    int cx, cy;
                    SDL_GetMouseState(&cx, &cy);
                    NUII->NJS->mouseWheel(event.wheel.x, event.wheel.y, cx, cy);
                    break;
                }
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                    NUII->NJS->mouseClick(event.button.x, event.button.y,
                                    event.button.state, event.button.button);
                break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                {
                    int keyCode = 0;
                    
                    int mod = 0;
                    if (
                        (&event.key)->keysym.sym == SDLK_r &&
                        event.key.keysym.mod & KMOD_GUI && event.type == SDL_KEYDOWN) {
                        printf("Refresh...\n");
                        //[console clear];
                        delete NUII->NJS;
                        
                        glClearColor(1, 1, 1, 0);
                        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                        
                        NUII->NJS = new NativeJS(NUII->getWidth(),
                            NUII->getHeight(), NUII);

                        NUII->NJS->bindNetObject(NUII->gnet);
                        if (NUII->NJS->LoadScript("./main.js")) {
                            NUII->NJS->Loaded();
                        }
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
                default:
                    break;
            }
            NUII->currentCursor = NativeCocoaUIInterface::NOCHANGE;
        }
        NUII->NJS->callFrame();
        NUII->NJS->rootHandler->layerize(NULL, 0, 0, 1.0, NULL);
        NUII->NJS->postDraw();

        glFlush();

    //}
    ttfps++;
    //NSLog(@"ret : %d for %d events", (tend - tstart), nevents);
    return 16;
}

static int NativeProcessUI(void *arg)
{
    return NativeEvents((NativeCocoaUIInterface *)arg);
}


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

static void NativeDoneExtracting(void *closure, const char *fpath)
{
    NativeCocoaUIInterface *ui = (NativeCocoaUIInterface *)closure;
    chdir(fpath);
    printf("Changing directory to : %s\n", fpath);
    if (ui->NJS->LoadScript("./main.js")) {
        printf("Running main?\n");
        ui->NJS->Loaded();
    }    

}

bool NativeCocoaUIInterface::runApplication(const char *path)
{
    NativeApp *app = new NativeApp(path);
    if (app->open()) {
        size_t fsize = 0;
        if (!this->createWindow(app->getWidth(), app->getHeight())) {
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

    return false;
}

NativeCocoaUIInterface::NativeCocoaUIInterface()
{
    this->width = 0;
    this->height = 0;
    /* Set the current working directory relative to the .app */
    char parentdir[MAXPATHLEN];

    this->currentCursor = NOCHANGE;
    this->NJS = NULL;

    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
    if (CFURLGetFileSystemRepresentation(url2, 1, (UInt8 *)parentdir, MAXPATHLEN)) {
        chdir(parentdir);   /* chdir to the binary app's parent */
    }
    CFRelease(url);
    CFRelease(url2);
}

bool NativeCocoaUIInterface::createWindow(int width, int height)
{
    SDL_GLContext contexteOpenGL;
    NSWindow *window;

    if (SDL_Init( SDL_INIT_EVERYTHING | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1)
    {
        printf( "Can't init SDL:  %s\n", SDL_GetError( ));
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0 );
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0 );
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    win = SDL_CreateWindow("Native - Running", 100, 100,
        width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if (win == NULL) {
        printf("Cant create window (SDL)\n");
        return false;
    }

    this->width = width;
    this->height = height;

    window = NativeCocoaWindow(win);

    [window setCollectionBehavior:
             NSWindowCollectionBehaviorFullScreenPrimary];

    
    initControls();

    //[btn setFrame:CGRectMake(btn.frame.origin.x, btn.frame.origin.y-4, btn.frame.size.width, btn.frame.size.width)];

    //[window setBackgroundColor:[NSColor colorWithSRGBRed:0.0980 green:0.1019 blue:0.09411 alpha:1.]];

    [window setFrameAutosaveName:@"nativeMainWindow"];

    //[window setStyleMask:NSTitledWindowMask|NSBorderlessWindowMask];
    //[window setOpaque:NO]; // YES by default
    //[window setAlphaValue:0.5];

    contexteOpenGL = SDL_GL_CreateContext(win);
    SDL_StartTextInput();

    if (SDL_GL_SetSwapInterval(kNativeVSYNC) == -1) {
        printf("Cant vsync\n");
    }

    glViewport(0, 0, width, height);

    NJS = new NativeJS(width, height, this);

    gnet = native_netlib_init();

    /* Set ape_global private to the JSContext
    and start listening for thread messages */

    NJS->bindNetObject(gnet);

    //NJS->LoadApplication("./demo.npa");

    return true;
}

void NativeCocoaUIInterface::setCursor(CURSOR_TYPE type)
{
    this->currentCursor = type;
}

void NativeCocoaUIInterface::setWindowTitle(const char *name)
{
    SDL_SetWindowTitle(win, (*name == '\0' ? "-" : name));
}

void NativeCocoaUIInterface::setTitleBarRGBAColor(uint8_t r, uint8_t g,
    uint8_t b, uint8_t a)
{
    NSWindow *window = NativeCocoaWindow(win);
    NSUInteger mask = [window styleMask];

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

void NativeCocoaUIInterface::runLoop()
{
    add_timer(&gnet->timersng, 1, NativeProcessUI, (void *)this);
    
    events_loop(gnet);  
}
