/**
 **   Copyright (c) 2012 All Right Reserved, Troll Face Studio
 **
 **   Authors :
 **       * Anthony Catel <a.catel@trollfacestudio.com>
 **/

#ifndef nativejs_h__
#define nativejs_h__

#include <jsapi.h>

#include "NativeSkia.h"

/* JSAPI binding (singleton) */
class NativeJS
{
    private:   
        void LoadCanvasObject();

        //void operator=(NativeJS const&);

        JSContext *cx;

    public:
        NativeSkia *nskia;
        NativeJS();
        ~NativeJS();
        int LoadScript(const char *filename);

};

#endif