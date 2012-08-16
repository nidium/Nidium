#include "NativeSkImage.h"
#include "NativeSkia.h"
#include "SkCanvas.h"
#include "SkBitmap.h"
#include "SkRect.h"
#include "SkImageRef_GlobalPool.h"
#include "SkStream.h"

static bool SetImageRef(SkBitmap* bitmap, SkStream* stream,
                        SkBitmap::Config pref, const char name[] = NULL)
{
    if (SkImageDecoder::DecodeStream(stream, bitmap, pref,
                                 SkImageDecoder::kDecodeBounds_Mode, NULL)) {
        SkASSERT(bitmap->config() != SkBitmap::kNo_Config);

        SkImageRef* ref = new SkImageRef_GlobalPool(stream, bitmap->config());
        ref->setURI(name);
        bitmap->setPixelRef(ref)->unref();
        return true;
    } else {
        return false;
    }
}

NativeSkImage::NativeSkImage(SkCanvas *canvas)
{
	bool ret = canvas->readPixels(SkIRect::MakeSize(canvas->getDeviceSize()), &img);

	isCanvas = 1;
	canvasRef = canvas;

	//printf("Draw image : %d\n", ret);
	canvas->drawBitmap(img, SkIntToScalar(200), SkScalar(200));
	//canvas->writePixels(bitmap, 30, 30);

	canvas->flush();
	
}

NativeSkImage::NativeSkImage(const char *imgPath)
{
	/* TODO: put this at start */

	static int ramAllocated = 0;

	if (!ramAllocated) {
		SkImageRef_GlobalPool::SetRAMBudget(32 * 1024);
		ramAllocated = 1;
	}

	isCanvas = 0;

	SkFILEStream* stream = new SkFILEStream(imgPath);

	SetImageRef(&img, stream, SkBitmap::kNo_Config, imgPath);

	stream->unref();
	//img->buildMipMap();
	//canvas->drawBitmap(*img, SkIntToScalar(0), SkScalar(0));
}