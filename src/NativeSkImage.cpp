#include "NativeSkImage.h"
#include "NativeSkia.h"
#include "SkCanvas.h"
#include "SkBitmap.h"
#include "SkRect.h"
#include "SkImageRef_GlobalPool.h"
#include "SkStream.h"
#include "SkData.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkUnPreMultiply.h"


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

static SkData* fileToData(const char path[]) {
    SkFILEStream stream(path);
    if (!stream.isValid()) {
        return SkData::NewEmpty();
    }
    size_t size = stream.getLength();
    void* mem = sk_malloc_throw(size);
    stream.read(mem, size);
    return SkData::NewFromMalloc(mem, size);
}

static SkData* dataToData(void *data, size_t size) {

    return SkData::NewWithCopy(data, size);
}

NativeSkImage::NativeSkImage(SkCanvas *canvas)
{
	//canvas->readPixels(SkIRect::MakeSize(canvas->getDeviceSize()), &img);

	isCanvas = 1;
	canvasRef = canvas;
	canvas->ref();
	fixedImg = NULL;
	
}

NativeSkImage::NativeSkImage(void *data, size_t len) :
	canvasRef(NULL)
{
	static int ramAllocated = 0;
	fixedImg = NULL;

	if (!ramAllocated) {
		SkImageRef_GlobalPool::SetRAMBudget(32 * 1024);
		ramAllocated = 1;
	}

	isCanvas = 0;
	
	SkMemoryStream *stream = new SkMemoryStream();

    SkAutoDataUnref datas(dataToData(data, len));
    fixedImg = SkImage::NewEncodedData(datas);

    stream->setData(datas);

	if (!SetImageRef(&img, stream, SkBitmap::kNo_Config, NULL)) {
		printf("Failed to decode image\n");
	}

    if (fixedImg == NULL) {
    	printf("Fixed is nulll\n");
    }

	stream->unref();

}

NativeSkImage::NativeSkImage(const char *imgPath) :
	canvasRef(NULL)
{
	static int ramAllocated = 0;
	fixedImg = NULL;

	if (!ramAllocated) {
		SkImageRef_GlobalPool::SetRAMBudget(32 * 1024);
		ramAllocated = 1;
	}

	isCanvas = 0;
	
	SkMemoryStream *stream = new SkMemoryStream();

    SkAutoDataUnref datas(fileToData(imgPath));
    fixedImg = SkImage::NewEncodedData(datas);

    stream->setData(datas);

	if (!SetImageRef(&img, stream, SkBitmap::kNo_Config, NULL)) {
		printf("Failed to decode image\n");
	}

    if (fixedImg == NULL) {
    	printf("Fixed is nulll\n");
    }

	stream->unref();
}

NativeSkImage::~NativeSkImage()
{
	if (canvasRef) canvasRef->unref();
	if (fixedImg) fixedImg->unref();
}

int NativeSkImage::getWidth()
{
	return img.width();
}

int NativeSkImage::getHeight()
{
	return img.height();
}

bool NativeSkImage::ConvertToRGBA(NativeSkImage *nimg, unsigned char* rgba, 
        bool flipY, bool premultiply) 
{
    int length;
    int k;
    const unsigned char *pixels;
    int width;

    nimg->img.lockPixels();
    if ((pixels = static_cast<const unsigned char*>(nimg->img.getPixels())) == NULL) {
        nimg->img.unlockPixels();
        return false;
    }

    width = nimg->getWidth();
    length = width * nimg->getHeight() * 4;
    width *= 4;
    k = flipY ? length - width: 0;

    for (int i = 0; i < length; i += 4) 
    {
        const uint32_t pixel = *reinterpret_cast<const uint32_t*>(&pixels[i]);
        int alpha = SkGetPackedA32(pixel);

        if (alpha != 0 && alpha != 255 && !premultiply) {
            SkColor unmultiplied = SkUnPreMultiply::PMColorToColor(pixel);
            rgba[k + 0] = SkColorGetR(unmultiplied);
            rgba[k + 1] = SkColorGetG(unmultiplied);
            rgba[k + 2] = SkColorGetB(unmultiplied);
            rgba[k + 3] = alpha;
        } else {
            rgba[k + 0] = SkGetPackedR32(pixel);
            rgba[k + 1] = SkGetPackedG32(pixel);
            rgba[k + 2] = SkGetPackedB32(pixel);
            rgba[k + 3] = alpha;
        }

        if (flipY && (i+4) % width == 0) {
            k = length - (i+4) - width;
        } else {
            k += 4;
        }
    }
    return true;
}
