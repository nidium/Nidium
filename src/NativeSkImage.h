#ifndef nativeskimage_h__
#define nativeskimage_h__
/*
drawImage(R,9,2,282,148,0,3,300,150)
*/
class SkCanvas;

class NativeSkImage
{
  public:
  	NativeSkImage(SkCanvas *canvas);
  	NativeSkImage(const char *imgpath);
  	~NativeSkImage();
};

#endif