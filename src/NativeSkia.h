/**
 **   Copyright (c) 2012 All Right Reserved, Troll Face Studio
 **
 **   Authors :
 **       * Anthony Catel <a.catel@trollfacestudio.com>
 **/

#ifndef nativeskia_h__
#define nativeskia_h__

#include <stdint.h>

class SkCanvas;
class SkPaint;
class SkPath;

class NativeSkia
{
    private:

        NativeSkia(){};
        NativeSkia(NativeSkia const&); 
        void operator=(NativeSkia const&);

        SkCanvas *canvas;
        SkPaint *paint;
        SkPaint *paint_stroke;
        SkPath *currentPath;

        uint8_t globalAlpha;

    public:
        int bindGL(int width, int height);

        /* Basics */
        void drawText(const char *text, int x, int y);
        void drawRect(double, double, double, double, double);
        void setFillColor(const char *str);
        void setStrokeColor(const char *str);
        void setLineWidth(double size);
        void setGlobalAlpha(double value);
        void clearRect(int, int, int, int);

        /* Shapes */
        void beginPath();
        void moveTo(double x, double y);
        void lineTo(double x, double y);
        void fill();
        void stroke();
        void closePath();
        void clip();
        void arc(int, int, int, double, double, int);
        void quadraticCurveTo(int cpx, int cpy, int x, int y);
        void bezierCurveTo(double cpx, double cpy, double cpx2,
            double cpy2, double x, double y);
        void rotate(double angle);
        void scale(double x, double y);
        void translate(double x, double y);
        void skew(double x, double y);
        void transform(double scalex, double skewx, double skewy,
            double scaley, double translatex, double translatey, int set);
        void save();
        void restore();

        static NativeSkia &getInstance() {
            static NativeSkia ret;

            return ret;
        }

};


#endif
