#ifndef nativeskgradient_h__
#define nativeskgradient_h__

#include "SkCanvas.h"
#include <stdint.h>

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
  	} startPoint;

  	struct 
  	{
  		double x;
  		double y;
  	} endPoint;

  	struct {
  		uint32_t count;
  		uint32_t allocated;

  		struct _colorStop *items;
  	} colorsStop;

  	int needUpdate;
  	SkShader *currentShader;

  public:
  	~NativeSkGradient();
    NativeSkGradient(double x1, double y1, double x2, double y2);
    void addColorStop(double position, char *color);
    SkShader *build();

};

#endif