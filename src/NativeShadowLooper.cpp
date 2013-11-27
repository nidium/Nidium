#include "NativeShadowLooper.h"
#include "SkBlurMask.h"     // just for SkBlurMask::ConvertRadiusToSigma
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkFlattenableBuffers.h"
#include "SkPaint.h"
#include "SkMaskFilter.h"
#include "SkColorFilter.h"
#include "SkColorPriv.h"

#include "SkMaskFilter.h"
#include "SkString.h"
#include "SkStringUtils.h"

void NativeShadowLooper::init(SkCanvas* canvas) {
    fState = kBeforeEdge;
}


void NativeShadowLooper::init(SkScalar sigma, SkScalar dx, SkScalar dy,
                            SkColor color, uint32_t flags) {

    fDx = dx;
    fDy = dy;
    fBlurColor = color;
    fBlurFlags = flags;
    fState = kDone;

    SkASSERT(flags <= kAll_BlurFlag);
    if (sigma > 0) {
        uint32_t blurFlags = flags & kIgnoreTransform_BlurFlag ?
            SkBlurMaskFilter::kIgnoreTransform_BlurFlag :
            SkBlurMaskFilter::kNone_BlurFlag;

        blurFlags |= flags & kHighQuality_BlurFlag ?
            SkBlurMaskFilter::kHighQuality_BlurFlag :
            SkBlurMaskFilter::kNone_BlurFlag;

        fBlur = SkBlurMaskFilter::Create(SkBlurMaskFilter::kNormal_BlurStyle,
                                         sigma,
                                         blurFlags);
    } else {
        fBlur = NULL;
    }


    fColorFilter = NULL;

}

NativeShadowLooper::NativeShadowLooper(SkScalar radius, SkScalar dx, SkScalar dy,
                                   SkColor color, uint32_t flags) {
    this->init(SkBlurMask::ConvertRadiusToSigma(radius), dx, dy, color, flags);
}

NativeShadowLooper::NativeShadowLooper(SkColor color, SkScalar sigma,
                                   SkScalar dx, SkScalar dy, uint32_t flags) {
    this->init(sigma, dx, dy, color, flags);
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

#ifdef SK_DEVELOPER
void NativeShadowLooper::toString(SkString* str) const {
    str->append("SkBlurDrawLooper: ");

    str->append("dx: ");
    str->appendScalar(fDx);

    str->append(" dy: ");
    str->appendScalar(fDy);

    str->append(" color: ");
    str->appendHex(fBlurColor);

    str->append(" flags: (");
    if (kNone_BlurFlag == fBlurFlags) {
        str->append("None");
    } else {
        bool needsSeparator = false;
        SkAddFlagToString(str, SkToBool(kIgnoreTransform_BlurFlag & fBlurFlags), "IgnoreTransform",
                          &needsSeparator);
        SkAddFlagToString(str, SkToBool(kOverrideColor_BlurFlag & fBlurFlags), "OverrideColor",
                          &needsSeparator);
        SkAddFlagToString(str, SkToBool(kHighQuality_BlurFlag & fBlurFlags), "HighQuality",
                          &needsSeparator);
    }
    str->append(")");

    // TODO: add optional "fBlurFilter->toString(str);" when SkMaskFilter::toString is added
    // alternatively we could cache the radius in SkBlurDrawLooper and just add it here
}
#endif
