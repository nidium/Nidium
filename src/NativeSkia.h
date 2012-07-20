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

    public:
        int bindGL(int width, int height);

        /* Basics */
        void drawText(const char *text, int x, int y);
        void drawRect(int, int, int, int, int);
        void setFillColor(const char *str);
        void setStrokeColor(const char *str);
        void setLineWidth(int size);
        void clearRect(int, int, int, int);

        /* Shapes */
        void beginPath();
        void moveTo(int x, int y);
        void lineTo(int x, int y);
        void fill();
        void stroke();
        void closePath();
        void arc(int, int, int, double, double, int);
        void quadraticCurveTo(int cpx, int cpy, int x, int y);
        void bezierCurveTo(double cpx, double cpy, double cpx2,
            double cpy2, double x, double y);

        static NativeSkia &getInstance() {
            static NativeSkia ret;

            return ret;
        }

};


#endif
