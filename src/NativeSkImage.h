#ifndef nativeskimage_h__
#define nativeskimage_h__

class SkCanvas;

class NativeSkImage
{
  public:
  	NativeSkImage(SkCanvas *canvas);
  	NativeSkImage(const char *imgpath);
  	~NativeSkImage();
};

#endif