#import "NativeStudioAppDelegate.h"
#import <OpenGL/gl.h>
#import <SDL2/SDL.h>
//#import <SDL2/SDL_opengl.h>
#import <SDL2/SDL_video.h>


#import "NativeJS.h"
#import "NativeSkia.h"
#import "NativeConsole.h"

#define kNativeWidth 1024
#define kNativeHeight 768
#define kFPS 100


uint32_t tfps = 0, ttfps = 0;

NativeJS *NJS;
NativeConsole *console;


@implementation NativeStudioAppDelegate

@synthesize window = _window;

- (void)dealloc
{
    [super dealloc];
}

- (void) setupWorkingDirectory:(BOOL)shouldChdir
{
    if (shouldChdir)
    {
        char parentdir[MAXPATHLEN];
        CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
        if (CFURLGetFileSystemRepresentation(url2, 1, (UInt8 *)parentdir, MAXPATHLEN)) {
            chdir(parentdir);   /* chdir to the binary app's parent */
        }
        CFRelease(url);
        CFRelease(url2);
    }
}

Uint32 NativeFPS(Uint32 interval, 
                 void*  param)
{
    NJS->currentFPS = tfps;
    tfps = 0;
    return 1000;
}

#define NUM_SOUNDS 1
struct sample {
    Uint8 *data;
    Uint32 dpos;
    Uint32 dlen;
} sounds[NUM_SOUNDS];

void mixaudio(void *unused, Uint8 *stream, int len)
{
    int i;
    Uint32 amount;
    
    memset(stream, 0, len);

        amount = (sounds[i].dlen-sounds[i].dpos);
        if ( amount > len ) {
            amount = len;
        }
        
        memcpy(stream, &sounds[i].data[sounds[i].dpos], amount);
        //SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, SDL_MIX_MAXVOLUME);
        sounds[i].dpos += amount;
    SDL_Event user_event;
    
    int16_t *jsbuffer = (int16_t *)malloc(sizeof(int16_t) * len);
    memcpy(jsbuffer, &sounds[i].data[sounds[i].dpos], amount/2);
    
    user_event.type=SDL_USEREVENT;
    user_event.user.code=42;
    user_event.user.data1=jsbuffer;
    user_event.user.data2=NULL;
    SDL_PushEvent(&user_event);

}

void PlaySound(char *file)
{
    int index;
    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;
    SDL_AudioCVT cvt;
    
    /* Look for an empty (or finished) sound slot */
    for ( index=0; index<NUM_SOUNDS; ++index ) {
        if ( sounds[index].dpos == sounds[index].dlen ) {
            break;
        }
    }
    if ( index == NUM_SOUNDS )
        return;
    
    /* Load the sound file and convert it to 16-bit stereo at 22kHz */
    if ( SDL_LoadWAV(file, &wave, &data, &dlen) == NULL ) {
        fprintf(stderr, "Couldn't load %s: %s\n", file, SDL_GetError());
        return;
    }
    SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
                      AUDIO_S16,   2,             44100);
    cvt.buf = (Uint8 *)malloc(dlen*cvt.len_mult);
    memcpy(cvt.buf, data, dlen);
    cvt.len = dlen;
    SDL_ConvertAudio(&cvt);
    SDL_FreeWAV(data);
    
    /* Put the sound data in the slot (it starts playing immediately) */
    if ( sounds[index].data ) {
        free(sounds[index].data);
    }
    SDL_LockAudio();
    sounds[index].data = cvt.buf;
    sounds[index].dlen = cvt.len_cvt;
    sounds[index].dpos = 0;
    SDL_UnlockAudio();
}

void NativeEvents(SDL_Window *win)
{   
    uint32_t starttime = SDL_GetTicks();
    uint32_t newtime;
    SDL_Event event;

    while(1) {
        
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_USEREVENT:
                    //NJS->bufferSound((int16_t *)event.user.data1, 4096);
                    break;
                case SDL_QUIT:
                    return;
                case SDL_MOUSEMOTION:
                    NJS->mouseMove(event.motion.x, event.motion.y,
                                   event.motion.xrel, event.motion.yrel);
                    break;
                case SDL_MOUSEWHEEL:
                {
                    int cx, cy;
                    SDL_GetMouseState(&cx, &cy);
                    NJS->mouseWheel(event.wheel.x, event.wheel.y, cx, cy);
                    break;
                }
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                    NJS->mouseClick(event.button.x, event.button.y,
                                    event.button.state, event.button.button);
                break;
                case SDL_KEYDOWN:
                    if (
                        (&event.key)->keysym.sym == SDLK_r) {
                        //printf("Refresh...\n");
                        [console clear];
                        delete NJS;
                        
                        glClearColor(1, 1, 1, 0);
                        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                        
                        NJS = new NativeJS();
                        NJS->nskia->bindGL(kNativeWidth, kNativeHeight);
                        NJS->LoadScript("./main.js");
                        //SDL_GL_SwapBuffers();
                        
                        
                    }
                    //return;
                    break;

            }
        }
        
        //glClear( GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        //NJS->nskia->redrawScreen();
        /*glClear (GL_STENCIL_BUFFER_BIT);

        glEnable(GL_STENCIL_TEST) ;
        glStencilFunc(GL_ALWAYS, 1, 1);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);*/
        NJS->callFrame();
        NJS->nskia->flush();
        //glDisable(GL_STENCIL_TEST);
        glFlush();
        //glFinish();
        //glDrawBuffer(GL_FRONT);
        //glDrawBuffer(GL_BACK);
        //SDL_GL_SwapBuffers();
        //SDL_GL_SwapWindow(win);
        if (++ttfps == 30) {
            NJS->gc();
            ttfps = 0;
        }
        //NJS->gc();
        
        newtime = SDL_GetTicks();
        
        if (newtime - starttime < 1000/kFPS) {
            /* TODO: JS_GC() */
            SDL_Delay((1000/kFPS) - (newtime - starttime));
        }
        
        tfps++;
        starttime = SDL_GetTicks();
    }
}



void resizeGLScene(int width, int height)
{
    glViewport(0, 0, width, height);
    
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    //gluPerspective( 0, 640.f/480.f, 1.0, 1024.0 );
    //gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
    glOrtho(0, width, height, 0, 0, 1024);
    
    glMatrixMode(GL_MODELVIEW);
    
    glLoadIdentity();
    
}
int initGL()
{
    //glShadeModel(GL_SMOOTH);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glEnable(GL_TEXTURE_2D);
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);
    glEnable(GL_MULTISAMPLE);
    //glClearDepth(1.0f);
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);    
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    return 1;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    SDL_Window *win;
    SDL_GLContext contexteOpenGL;
    
    console = [[NativeConsole alloc] init];
    [console attachToStdout];

    if( SDL_Init( SDL_INIT_EVERYTHING | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1 )
    {
        printf( "Can't init SDL:  %s\n", SDL_GetError( ) );
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
    
    //SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, true );
    //SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 ); /* TODO check */
    //SDL_GL_SetSwapInterval(1);
    //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8 );
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0 );
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    NJS = new NativeJS();
    
    win = SDL_CreateWindow("Native - Running", 100, 100, kNativeWidth, kNativeHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    contexteOpenGL = SDL_GL_CreateContext(win);

    //SDL_SetWindowFullscreen(win, SDL_TRUE);

    resizeGLScene(kNativeWidth, kNativeHeight);
    initGL();
    
    NJS->nskia->bindGL(kNativeWidth, kNativeHeight);
    
    [self setupWorkingDirectory:YES];
    
    SDL_AddTimer(1000, NativeFPS, NULL);
    
    SDL_AudioSpec desired, obtained, soundfile;
    SDL_AudioSpec fmt;
    
    /* Un son sereo de 16 bits à 22kHz */
    fmt.freq = 44100;
    fmt.format = AUDIO_S16;
    fmt.channels = 2;
    fmt.samples = 4096;        /*Une bonne valeur pour les jeux */
    fmt.callback = mixaudio;
    fmt.userdata = NULL;
    
    /* Ouvre le contexte audio et joue le son */
    if ( SDL_OpenAudio(&fmt, NULL) < 0 ) {
        fprintf(stderr, "Impossible d'accéder à l'audio: %s\n", SDL_GetError());
        exit(1);
    }

    PlaySound("/Users/anthonycatel/pof2.wav");
    
    //SDL_PauseAudio(0);
    if (!NJS->LoadScript("./main.js")) {
        NSLog(@"Cant load script");
    }

    printf("[DEBUG] OpenGL %s\n", glGetString(GL_VERSION));
    atexit( SDL_Quit );
    //glClear(GL_COLOR_BUFFER_BIT);
    NJS->nskia->flush();
    SDL_GL_SwapWindow(win);
    
    //[self.window dealloc];
    NativeEvents(win);

}

@end
