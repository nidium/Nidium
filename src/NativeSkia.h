/**
 **   Copyright (c) 2012 All Right Reserved, Troll Face Studio
 **
 **   Authors :
 **       * Anthony Catel <a.catel@trollfacestudio.com>
 **/

#ifndef nativeskia_h__
#define nativeskia_h__

class SkCanvas;
class SkPaint;

class NativeSkia
{
    private:

        NativeSkia(){};
        NativeSkia(NativeSkia const&); 
        void operator=(NativeSkia const&);

        SkCanvas *canvas;
        SkPaint *paint;
        SkPaint *paint_stroke;

    public:
        int bindGL(int width, int height);
        void drawText(const char *text, int x, int y);
        void drawRect(int, int, int, int, int);
        void setFillColor(const char *str);
        void setStrokeColor(const char *str);
        void setLineWidth(int size);
        void clearRect(int, int, int, int);
        static NativeSkia &getInstance() {
            static NativeSkia ret;

            return ret;
        }

};


#endif
