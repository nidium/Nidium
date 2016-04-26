#ifndef graphics_skgradient_h__
#define graphics_skgradient_h__

#include <stdint.h>

#include <GrGLTexture.h>
#include <SkCanvas.h>

class SkShader;

namespace Nidium {
namespace Graphics {

struct _colorStop
{
    SkColor color;
    SkScalar position;
};

class Gradient
{
  private:

    struct
    {
        double x;
        double y;
        double radius;
    } m_StartPoint;

    struct
    {
        double x;
        double y;
        double radius;
    } m_EndPoint;

    struct {
        uint32_t count;
        uint32_t allocated;

        struct _colorStop *items;
    } m_ColorsStop;

    int m_IsRadial;
    int m_NeedUpdate;
    SkShader *m_CurrentShader;

  public:
    ~Gradient();
    Gradient(double x1, double y1, double x2, double y2);
    Gradient(double x0, double y0, double r0, double x1,
      double y1, double r1);
    void addColorStop(double position, char *color);
    SkShader *build();

};

} // namespace Graphics
} // namespace Nidium

#endif

