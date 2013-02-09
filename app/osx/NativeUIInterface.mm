#import "NativeUIInterface.h"
#import <NativeJS.h>
#import <NativeSkia.h>
#import <SDL2/SDL.h>
#import <SDL2/SDL_opengl.h>
#import <SDL2/SDL_syswm.h>
#import <Cocoa/Cocoa.h>
#import <native_netlib.h>

#define kNativeWidth 1024
#define kNativeHeight 760
#define kNativeVSYNC 0

uint32_t ttfps = 0;

int NativeEvents(NativeUIInterface *NUII)
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
                        
                        NUII->NJS = new NativeJS(kNativeWidth, kNativeHeight);
                        NUII->NJS->UI = NUII;
                        NUII->NJS->bindNetObject(NUII->gnet);
                        NUII->NJS->LoadScript("./main.js");
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
        if (NUII->currentCursor != NativeUIInterface::NOCHANGE) {
            switch(NUII->currentCursor) {
                case NativeUIInterface::ARROW:
                    [[NSCursor arrowCursor] set];
                    break;
                case NativeUIInterface::BEAM:
                    [[NSCursor IBeamCursor] set];
                    break;
                case NativeUIInterface::CROSS:
                    [[NSCursor crosshairCursor] set];
                    break;
                case NativeUIInterface::POINTING:
                    [[NSCursor pointingHandCursor] set];
                    break;
                case NativeUIInterface::CLOSEDHAND:
                    [[NSCursor closedHandCursor] set];
                    break;
                default:
                    break;
            }
            NUII->currentCursor = NativeUIInterface::NOCHANGE;
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
    return NativeEvents((NativeUIInterface *)arg);
}


NativeUIInterface::NativeUIInterface() :
    currentCursor(NOCHANGE), NJS(NULL)
{
    /* Set the current working directory relative to the .app */
    char parentdir[MAXPATHLEN];

    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
    if (CFURLGetFileSystemRepresentation(url2, 1, (UInt8 *)parentdir, MAXPATHLEN)) {
        chdir(parentdir);   /* chdir to the binary app's parent */
    }
    CFRelease(url);
    CFRelease(url2);
}

void NativeUIInterface::createWindow()
{
    SDL_GLContext contexteOpenGL;
    NSWindow *window;

    if (SDL_Init( SDL_INIT_EVERYTHING | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1)
    {
        printf( "Can't init SDL:  %s\n", SDL_GetError( ));
        return;
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
        kNativeWidth, kNativeHeight,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if (win == NULL) {
        printf("Cant create window (SDL)\n");
        return;
    }

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(win, &info);

    window = (NSWindow *)info.info.cocoa.window;

    [(NSWindow *)info.info.cocoa.window setFrameAutosaveName:@"nativeMainWindow"];

    //[window setStyleMask:NSTitledWindowMask|NSBorderlessWindowMask];

    contexteOpenGL = SDL_GL_CreateContext(win);
    SDL_StartTextInput();

    if (SDL_GL_SetSwapInterval(kNativeVSYNC) == -1) {
        printf("Cant vsync\n");
    }

    glViewport(0, 0, kNativeWidth, kNativeHeight);

    NJS = new NativeJS(kNativeWidth, kNativeHeight);
    NJS->UI = this;

    gnet = native_netlib_init();

    /* Set ape_global private to the JSContext
    and start listening for thread messages */

    NJS->bindNetObject(gnet);

    NJS->LoadScript("./main.js");

}

void NativeUIInterface::setCursor(CURSOR_TYPE type)
{
    this->currentCursor = type;
}

void NativeUIInterface::setWindowTitle(const char *name)
{
    SDL_SetWindowTitle(win, (*name == '\0' ? "-" : name));
}

void NativeUIInterface::runLoop()
{
    add_timer(&gnet->timersng, 1, NativeProcessUI, (void *)this);
    
    events_loop(gnet);  
}
