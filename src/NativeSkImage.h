#ifndef nativeskimage_h__
#define nativeskimage_h__

#include "SkBitmap.h"
/*
drawImage(R,9,2,282,148,0,3,300,150)
*/
class SkCanvas;
class SkBitmap;

class NativeSkImage
{	
  public:
  	int isCanvas;
  	SkCanvas *canvasRef;
  	SkBitmap img;
  	NativeSkImage(SkCanvas *canvas);
  	NativeSkImage(const char *imgpath);
  	~NativeSkImage();
};

#endif