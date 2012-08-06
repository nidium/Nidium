#include "NativeSkImage.h"
#include "NativeSkia.h"
#include "SkCanvas.h"
#include "SkBitmap.h"
#include "SkRect.h"

NativeSkImage::NativeSkImage(SkCanvas *canvas)
{
	SkBitmap bitmap;

	//bool ret = canvas->readPixels(SkIRect::MakeSize(canvas->getDeviceSize()), &bitmap);
	//printf("Draw image : %d\n", ret);
	//canvas->drawBitmap(bitmap, SkIntToScalar(30), SkScalar(30));
	//canvas->writePixels(bitmap, 30, 30);
	//canvas->flush();
}