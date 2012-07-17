/**
 **   Copyright (c) 2012 All Right Reserved, Troll Face Studio
 **
 **   Authors :
 **       * Anthony Catel <a.catel@trollfacestudio.com>
 **/

#ifndef nativeskia_h__
#define nativeskia_h__

class SkCanvas;

class NativeSkia
{
    private:

        NativeSkia(){};
        NativeSkia(NativeSkia const&); 
        void operator=(NativeSkia const&);

        SkCanvas *canvas;

    public:
        int bindGL(int width, int height);
        static NativeSkia &getInstance() {
            static NativeSkia ret;

            return ret;
        }

};


#endif
