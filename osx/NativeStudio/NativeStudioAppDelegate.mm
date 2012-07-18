//
//  NativeStudioAppDelegate.m
//  NativeStudio
//
//  Created by Anthony Catel on 7/16/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "NativeStudioAppDelegate.h"
#import "NativeJS.h"
#import "NativeSkia.h"
#import <OpenGL/gl.h>
#import <OpengL/glu.h>
#import <SDL/SDL.h>

#import "SkRefCnt.h"


//extern "C" {
    void foo();
//}

#define kNativeWidth 640
#define kNativeHeight 480


@implementation NativeStudioAppDelegate

//@synthesize window = _window;

- (void)dealloc
{
    [super dealloc];
}

void sdl_pause()
{
    int continuer = 1;
    SDL_Event event;
    
    while (continuer)
    {
        
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                continuer = 0;
                break;
            case SDL_KEYUP:
                if ((&event.key)->keysym.scancode == 0x00) {
                    SDL_Quit();
                    exit(0);
                    return;
                }
                
                
                break;
        }
    }
}

void resizeGLScene(int width, int height)
{
    glViewport(0, 0, width, height);
    
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    //gluPerspective( 0, 640.f/480.f, 1.0, 1024.0 );
    //gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
    glOrtho(0, 640, 480, 0, 0, 1024);
    
    
    glMatrixMode(GL_MODELVIEW);
    
    glLoadIdentity();
    
}

int initGL()
{
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    //glClearDepth(1.0f);
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);    
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    return 1;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    
    SDL_Surface *screen;
    if( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        printf( "Can't init SDL:  %s\n", SDL_GetError( ) );
        return;
    }
    
    atexit( SDL_Quit );
    
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
    //SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
    SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, 32 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    
    screen = SDL_SetVideoMode(kNativeWidth, kNativeHeight, 0, SDL_HWSURFACE | SDL_OPENGL);
    
    if (screen == NULL) {
        NSLog(@"Cant init screen");
        return;
    }
    
    resizeGLScene(kNativeWidth, kNativeHeight);
    initGL();
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    NativeSkia::getInstance().bindGL(kNativeWidth, kNativeHeight);
    
    SDL_GL_SwapBuffers();
    
    SDL_WM_SetCaption("Native Studio - Troll Face Studio", NULL);
    
    SDL_Flip(screen);
    
    if (!NativeJS::getInstance().LoadScript("/Users/anthonycatel/Nativestudio/main.js")) {
        NSLog(@"Cant load script");
        return;
    }
  
    
    //foo();
    sdl_pause();
}

@end
