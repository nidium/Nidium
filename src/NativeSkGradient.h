#ifndef nativeskgradient_h__
#define nativeskgradient_h__

#include <stdint.h>

#include <GrGLTexture.h>
#include <SkCanvas.h>

class SkShader;

struct _colorStop
{
    SkColor color;
    SkScalar position;
};

class NativeSkGradient
{
  private:

    struct
    {
        double x;
        double y;
        double radius;
    } startPoint;

    struct
    {
        double x;
        double y;
        double radius;
    } endPoint;

    struct {
        uint32_t count;
        uint32_t allocated;

        struct _colorStop *items;
    } colorsStop;

    int isRadial;
    int needUpdate;
    SkShader *currentShader;

  public:
    ~NativeSkGradient();
    NativeSkGradient(double x1, double y1, double x2, double y2);
    NativeSkGradient(double x0, double y0, double r0, double x1,
      double y1, double r1);
    void addColorStop(double position, char *color);
    SkShader *build();

};

#endif

