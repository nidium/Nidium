
#include <GL/gl.h>
#include <SDL.h>
//#include <SDL2/SDL_opengl.h>
#include <SDL_video.h>
#include <pthread.h>

#include "NativeJS.h"
#include "NativeSkia.h"
#include "native_netlib.h"
#include "ape_timers.h"

#define kNativeWidth 1024
#define kNativeHeight 768
#define kFPS 100


uint32_t tfps = 0, ttfps = 0;
int ape_running = 1;

NativeJS *NJS;
ape_global *gnet = NULL;
int NativeEvents(SDL_Window *win);

uint32_t starttime = SDL_GetTicks();
uint32_t newtime;

Uint32 NativeFPS(Uint32 interval, 
                 void*  param)
{
    NJS->currentFPS = tfps;
    tfps = 0;
    return 1000;
}


static int NativeProcessUI(void *arg)
{
    SDL_Window *win = (SDL_Window *)arg;
    //printf("process UI\n");
    return NativeEvents(win);
}

static void NativeRunMainLoop(ape_global *net, SDL_Window *win)
{
    add_timer(&net->timersng, 1, NativeProcessUI, (void *)win);
    
    events_loop(net);
}


int NativeEvents(SDL_Window *win)
{   

    SDL_Event event;
    uint32_t tstart, tend;
    tstart = SDL_GetTicks();
    //while(1) {
    int nevents = 0;
        while(SDL_PollEvent(&event)) {
            nevents++;
            switch(event.type) {
                case SDL_TEXTINPUT:
                    NJS->textInput(event.text.text);
                    break;
                case SDL_USEREVENT:
                    //NJS->bufferSound((int16_t *)event.user.data1, 4096);
                    break;
                case SDL_QUIT:
                    ape_running = 0;
                    delete NJS;
                    return 0;
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
                case SDL_KEYUP:
                {
                    int keyCode = 0;
                    
                    int mod = 0;
                    if (
                        (&event.key)->keysym.sym == SDLK_r &&
                        (event.key.keysym.mod & KMOD_GUI ||
                            event.key.keysym.mod & KMOD_CTRL) &&
                            event.type == SDL_KEYUP) {

                        printf("Refresh...\n");
                        //[console clear];
                        delete NJS;
                        
                        glClearColor(0, 0, 0, 0);
                        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                        
                        NJS = new NativeJS();
                        NJS->nskia->bindGL(kNativeWidth, kNativeHeight);
                        NJS->bindNetObject(gnet);
                        NJS->LoadScript("./main.js");
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
                    
                    NJS->keyupdown(keyCode, mod, event.key.state, event.key.repeat);

                    break;
                }
            }
        }
        
        //NJS->nskia->redrawScreen();
        /*glClear (GL_STENCIL_BUFFER_BIT);

        glEnable(GL_STENCIL_TEST) ;
        glStencilFunc(GL_ALWAYS, 1, 1);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);*/
        

//        glDisable(GL_ALPHA_TEST);
        /*glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
        glEnable(GL_TEXTURE_2D);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);*/

        NJS->callFrame();
        NJS->nskia->flush();
    
        /*for (int x = 0; x < 3000; x++) {
            glWindowPos2i(10+(x), 700);
            glDrawPixels(2, 2, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pxls);
        }*/
        /*if (glGetError() != GL_NO_ERROR) {
            printf("got an err\n");
        }*/
        //glDisable(GL_STENCIL_TEST);
        //glFlush();
        //glDrawBuffer(GL_FRONT);
        //glDrawBuffer(GL_BACK);
        //SDL_GL_SwapBuffers();:q
         
        SDL_GL_SwapWindow(win);

        tend = SDL_GetTicks();
        
        if (tend - tstart > 10) {
            //NSLog(@"Took : %d\n", tend - tstart);
        //NJS->gc();
            ttfps = 0;
        }        
        tfps++;
    //}

    //NSLog(@"ret : %d for %d events", (tend - tstart), nevents);
    return 16;
}



void resizeGLScene(int width, int height)
{
    glViewport(0, 0, width, height);
    
#if 0    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    //gluPerspective( 0, 640.f/480.f, 1.0, 1024.0 );
    //gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
    glOrtho(0, width, height, 0, 0, 1024);
    
    glMatrixMode(GL_MODELVIEW);
    
    glLoadIdentity();
#endif    
}
int initGL()
{
    //glShadeModel(GL_SMOOTH);
    //glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    //glEnable(GL_TEXTURE_2D);
    //glClearColor(1, 1, 1, 0);
    //glClear(GL_COLOR_BUFFER_BIT);
    //glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_FASTEST);
    //glEnable(GL_MULTISAMPLE);
    //glClearDepth(1.0f);
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);    
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    return 1;
}

int main(int argc, char **argv)
{

    SDL_Window *win;
    SDL_GLContext contexteOpenGL;

    if( SDL_Init( SDL_INIT_EVERYTHING | SDL_INIT_TIMER | SDL_INIT_AUDIO) == -1 )
    {
        printf( "Can't init SDL:  %s\n", SDL_GetError( ) );
        return 0;
    }

    //SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
    //SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, true );
    //SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 4 ); /* TODO check */

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5 );
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8 );
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32 );
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1 );
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    
    NJS = new NativeJS();
    
    win = SDL_CreateWindow("Native - Running", 100, 100, kNativeWidth, kNativeHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    contexteOpenGL = SDL_GL_CreateContext(win);
    if (SDL_GL_SetSwapInterval(1) == -1) {
        printf("Cant vsync\n");
    }
    //SDL_SetWindowFullscreen(win, SDL_TRUE);

    resizeGLScene(kNativeWidth, kNativeHeight);

    initGL();

    NJS->nskia->bindGL(kNativeWidth, kNativeHeight);

    SDL_AddTimer(1000, NativeFPS, NULL);
    SDL_StartTextInput();
    
    gnet = native_netlib_init();
    
    NJS->bindNetObject(gnet);
    
    int e = 0;
    if ((e = glGetError()) != GL_NO_ERROR) {
        printf("first error %d\n", e);
    }
    
    if (!NJS->LoadScript("./main.js")) {
        printf("Cant load script\n");
    }
    e = 0;
    if ((e = glGetError()) != GL_NO_ERROR) {
        printf("got an error here %d\n", e);
    }    
    printf("[DEBUG] OpenGL %s\n", glGetString(GL_VERSION));
    atexit( SDL_Quit );
    //glClear(GL_COLOR_BUFFER_BIT);
    //NJS->nskia->flush();
    SDL_GL_SwapWindow(win);
    //glPixelStorei(GL_PACK_ALIGNMENT, 1);


        //[self.window dealloc];
    //NativeEvents(win);
    NativeRunMainLoop(gnet, win);
}