#include "NativeShadowLooper.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkFlattenableBuffers.h"
#include "SkPaint.h"
#include "SkMaskFilter.h"
#include "SkColorFilter.h"

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

    if (flags & kOverrideColor_BlurFlag) {
        // Set alpha to 1 for the override since transparency will already
        // be baked into the blurred mask.
        SkColor opaqueColor = SkColorSetA(color, 255);
        //The SrcIn xfer mode will multiply 'color' by the incoming alpha
        fColorFilter = SkColorFilter::CreateModeFilter(opaqueColor,
                                                       SkXfermode::kSrcIn_Mode);
    } else {
        fColorFilter = NULL;
    }
}

NativeShadowLooper::NativeShadowLooper(SkFlattenableReadBuffer& buffer)
: INHERITED(buffer) {

    fDx = buffer.readScalar();
    fDy = buffer.readScalar();
    fBlurColor = buffer.readColor();
    fBlur = buffer.readFlattenableT<SkMaskFilter>();
    fColorFilter = buffer.readFlattenableT<SkColorFilter>();
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
    switch (fState) {
        case kBeforeEdge:
            // we do nothing if a maskfilter is already installed
            if (paint->getMaskFilter()) {
                fState = kDone;
                return false;
            }

            paint->setColor(fBlurColor);

            paint->setMaskFilter(fBlur);
            paint->setColorFilter(fColorFilter);
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
