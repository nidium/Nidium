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

        SkImageRef* ref = new SkImageRef_GlobalPool(stream, bitmap->config(), 1);
        ref->setURI(name);
        bitmap->setPixelRef(ref)->unref();
        return true;
    } else {
        return false;
    }
}


NativeSkImage::NativeSkImage(SkCanvas *canvas)
{
	canvas->readPixels(SkIRect::MakeSize(canvas->getDeviceSize()), &img);

	isCanvas = 1;
	canvasRef = canvas;
	
}

NativeSkImage::NativeSkImage(void *data, size_t len)
{
	static int ramAllocated = 0;

	if (!ramAllocated) {
		SkImageRef_GlobalPool::SetRAMBudget(32 * 1024);
		ramAllocated = 1;
	}

	isCanvas = 0;
	
	SkMemoryStream *stream = new SkMemoryStream(data, len, false);

	if (!SetImageRef(&img, stream, SkBitmap::kNo_Config, NULL)) {
		printf("Failed to parse image\n");
	} else {
		printf("Image parsed\n");
	}

	stream->unref();

}

NativeSkImage::NativeSkImage(const char *imgPath)
{
	/* TODO: put this at start */

	static int ramAllocated = 0;

	printf("New image\n");

	if (!ramAllocated) {
		SkImageRef_GlobalPool::SetRAMBudget(32 * 1024);
		ramAllocated = 1;
	}

	isCanvas = 0;

	SkFILEStream* stream = new SkFILEStream(imgPath);

	if (!SetImageRef(&img, stream, SkBitmap::kNo_Config, imgPath)) {
		printf("failed to parse image\n");
	} else {
		printf("parsed\n");
	}

	stream->unref();
	//img->buildMipMap();
	//canvas->drawBitmap(*img, SkIntToScalar(0), SkScalar(0));
}

int NativeSkImage::getWidth()
{
	return img.width();
}

int NativeSkImage::getHeight()
{
	return img.height();
}
