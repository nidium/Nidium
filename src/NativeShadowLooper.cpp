#include "NativeShadowLooper.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkFlattenableBuffers.h"
#include "SkPaint.h"
#include "SkMaskFilter.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"

NativeShadowLooper::NativeShadowLooper(SkScalar radius, SkScalar dx, SkScalar dy,
                                   SkColor color, uint32_t flags)
    : fDx(dx), fDy(dy), fBlurColor(color), fBlurFlags(flags), fState(kDone) {

    SkASSERT(flags <= kAll_BlurFlag);
    if (radius > 0) {
        uint32_t blurFlags = flags & kIgnoreTransform_BlurFlag ?
            SkBlurMaskFilter::kIgnoreTransform_BlurFlag :
            SkBlurMaskFilter::kNone_BlurFlag;

        blurFlags |= flags & kHighQuality_BlurFlag ?
            SkBlurMaskFilter::kHighQuality_BlurFlag :
            SkBlurMaskFilter::kNone_BlurFlag;

        fBlur = SkBlurMaskFilter::Create(radius,
                                         SkBlurMaskFilter::kNormal_BlurStyle,
                                         blurFlags);
    } else {
        fBlur = NULL;
    }

    fColorFilter = NULL;
}

NativeShadowLooper::NativeShadowLooper(SkFlattenableReadBuffer& buffer)
: INHERITED(buffer) {

    fDx = buffer.readScalar();
    fDy = buffer.readScalar();
    fBlurColor = buffer.readColor();
    fBlur = buffer.readMaskFilter();
    fColorFilter = buffer.readColorFilter();
    fBlurFlags = buffer.readUInt() & kAll_BlurFlag;
}

NativeShadowLooper::~NativeShadowLooper() {
    SkSafeUnref(fBlur);
    SkSafeUnref(fColorFilter);
}

void NativeShadowLooper::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeColor(fBlurColor);
    buffer.writeFlattenable(fBlur);
    buffer.writeFlattenable(fColorFilter);
    buffer.writeUInt(fBlurFlags);
}

void NativeShadowLooper::init(SkCanvas* canvas) {
    fState = kBeforeEdge;
}

bool NativeShadowLooper::next(SkCanvas* canvas, SkPaint* paint) {
    U8CPU a;
    switch (fState) {
        case kBeforeEdge:
            // we do nothing if a maskfilter is already installed
            if (paint->getMaskFilter()) {
                fState = kDone;
                return false;
            }
            a = paint->getAlpha();
            paint->setColor(fBlurColor);
            paint->setAlpha(SkAlphaMul(paint->getAlpha(), SkAlpha255To256(a)));

            paint->setShader(NULL);
            paint->setMaskFilter(fBlur);

            canvas->save(SkCanvas::kMatrix_SaveFlag);

            if (fBlurFlags & kIgnoreTransform_BlurFlag) {
                SkMatrix transform(canvas->getTotalMatrix());
                transform.postTranslate(fDx, fDy);
                canvas->setMatrix(transform);
            } else {
                canvas->translate(fDx, fDy);
            }
            fState = kAfterEdge;
            return true;
        case kAfterEdge:
            canvas->restore();
            fState = kDone;
            return true;
        default:
            SkASSERT(kDone == fState);
            return false;
    }
}
