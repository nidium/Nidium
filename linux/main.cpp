#include <GL/gl.h>
#include <SDL.h>
#include <SDL_video.h>

#include "NativeJS.h"
#include "NativeSkia.h"

#define kNativeWidth 1024
#define kNativeHeight 768
#define kFPS 70


uint32_t tfps = 0, ttfps = 0;
#define NELEM(x) (sizeof(x)/sizeof(double))

NativeJS *NJS;
//NativeConsole *console;

Uint32 NativeFPS(Uint32 interval, 
                 void*  param)
{
    NJS->currentFPS = tfps*2;
    tfps = 0;
    return 500;
}

double arrLowpass(double *array, size_t arrsize, double x)
{
    double ret = 0;
    double decel = 1;
    
    while (arrsize) {
        ret += array[arrsize-1] * decel; 
        arrsize--;
        decel *= 0.8;
    }
    return (ret+x) / (arrsize+1);
}

void NativeEvents(SDL_Window *win)
{   
    uint32_t starttime = SDL_GetTicks();
    uint32_t newtime;
    SDL_Event event;
    int32_t lastwheel = -1;
    double arrdelta[8];
    memset(arrdelta, 0, sizeof(arrdelta));
    
    int ndelta = 0;
    
    while(1) {
        
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    return;
                case SDL_MOUSEMOTION:
                    NJS->mouseMove(event.motion.x, event.motion.y,
                                   event.motion.xrel, event.motion.yrel);
                    break;
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                {
                    double delta = 0;
                    double detail = 0;
                    if (event.button.state == 1 &&
                        (event.button.button == 4 || event.button.button == 5)) {
                        uint32_t wheeltime = SDL_GetTicks();
                        uint32_t t = (lastwheel == -1 ? 40 : 1 + (wheeltime - lastwheel));
                        
                        delta = 12 * 1./t;
                        arrdelta[ndelta%NELEM(arrdelta)] = delta;
                        
                        detail = arrLowpass(arrdelta, NELEM(arrdelta), delta);
                        
                        
                        ndelta++;
                        
                        lastwheel = wheeltime;
                    }
                    NJS->mouseClick(event.button.x, event.button.y,
                                    event.button.state, event.button.button,
                                    event.motion.xrel, event.motion.yrel, detail);
                    
                }
                break;
                case SDL_KEYDOWN:
                    if (
                        (&event.key)->keysym.sym == SDLK_r) {
                        //printf("Refresh...\n");
                        //[console clear];
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
        
        NJS->callFrame();
        NJS->nskia->flush();
        //glFlush();
        //glFinish();
        //glDrawBuffer(GL_FRONT);
        //glDrawBuffer(GL_BACK);
        //SDL_GL_SwapBuffers();
        SDL_GL_SwapWindow(win);
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
    //glClearDepth(1.0f);
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);    
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    return 1;
}

int main(int argc, char **argv) {
    SDL_Window *win;
    SDL_GLContext contexteOpenGL;
    
    if( SDL_Init( SDL_INIT_EVERYTHING | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1 )
    {
        printf( "Can't init SDL:  %s\n", SDL_GetError( ) );
        return 1;
    }

    
    //SDL_GL_SetSwapInterval(1);
    //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);
    /*
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 4 );
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 4 );
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 4 );
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8 );
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    */
    
    NJS = new NativeJS();
    
    win = SDL_CreateWindow("Swelen Browser", 100, 100, kNativeWidth, kNativeHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (win == NULL) {
        printf("%s\n", SDL_GetError());
        return 1;
    }
    /*
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    */

    contexteOpenGL = SDL_GL_CreateContext(win);
    printf("after\n");
    if (contexteOpenGL == NULL) {
        printf("%s\n", SDL_GetError());
        return 1;
    }

    //SDL_SetWindowFullscreen(win, SDL_TRUE);

    resizeGLScene(kNativeWidth, kNativeHeight);
    printf("[DEBUG] OpenGL %s\n", glGetString(GL_VERSION));
    printf("after\n");
    initGL();
    printf("after\n");
    
    NJS->nskia->bindGL(kNativeWidth, kNativeHeight);
    printf("after\n");
    
    //[self setupWorkingDirectory:YES];
    
    SDL_AddTimer(500, NativeFPS, NULL);
    if (!NJS->LoadScript("./main.js")) {
        printf("Cant load script");
    }

    printf("[DEBUG] OpenGL %s\n", glGetString(GL_VERSION));
    atexit( SDL_Quit );
    
    
    //[self.window dealloc];
    NativeEvents(win);

    return 1;
}

