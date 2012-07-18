/**
 **   Copyright (c) 2012 All Right Reserved, Troll Face Studio
 **
 **   Authors :
 **       * Anthony Catel <a.catel@trollfacestudio.com>
 **/

#ifndef nativejs_h__
#define nativejs_h__

#include <jsapi.h>

/* JSAPI binding (singleton) */
class NativeJS
{
    private:
        NativeJS();
        NativeJS(NativeJS const&); 
        void LoadCanvasObject();

        void operator=(NativeJS const&);

        JSContext *cx;

    public:
        int LoadScript(const char *filename);
        
        static NativeJS &getInstance() {
            static NativeJS ret;

            return ret;
        }

};

#endif