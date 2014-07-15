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


void NativeShadowLooper::init(SkScalar sigma, SkScalar dx, SkScalar dy,
                            SkColor color, uint32_t flags) {
    fSigma = sigma;
    fDx = dx;
    fDy = dy;
    fBlurColor = color;
    fBlurFlags = flags;

    this->initEffects();
}

void NativeShadowLooper::initEffects()
{
    SkASSERT(fBlurFlags <= kAll_BlurFlag);
    if (fSigma > 0) {
        uint32_t flags = fBlurFlags & kIgnoreTransform_BlurFlag ?
                            SkBlurMaskFilter::kIgnoreTransform_BlurFlag :
                            SkBlurMaskFilter::kNone_BlurFlag;

        flags |= fBlurFlags & kHighQuality_BlurFlag ?
                    SkBlurMaskFilter::kHighQuality_BlurFlag :
                    SkBlurMaskFilter::kNone_BlurFlag;

        fBlur = SkBlurMaskFilter::Create(kNormal_SkBlurStyle, fSigma, flags);
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

NativeShadowLooper::NativeShadowLooper(SkReadBuffer& buffer) : INHERITED(buffer) {

    fSigma = buffer.readScalar();
    fDx = buffer.readScalar();
    fDy = buffer.readScalar();
    fBlurColor = buffer.readColor();
    fBlurFlags = buffer.readUInt() & kAll_BlurFlag;

    this->initEffects();
}

void NativeShadowLooper::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSigma);
    buffer.writeScalar(fDx);
    buffer.writeScalar(fDy);
    buffer.writeColor(fBlurColor);
    buffer.write32(fBlurFlags);
}

NativeShadowLooper::~NativeShadowLooper() {
    SkSafeUnref(fBlur);
    SkSafeUnref(fColorFilter);
}

bool NativeShadowLooper::asABlurShadow(BlurShadowRec* rec) const {
    if (fSigma <= 0 || (fBlurFlags & fBlurFlags & kIgnoreTransform_BlurFlag)) {
        return false;
    }

    if (rec) {
        rec->fSigma = fSigma;
        rec->fColor = fBlurColor;
        rec->fOffset.set(fDx, fDy);
        rec->fStyle = kNormal_SkBlurStyle;
        rec->fQuality = (fBlurFlags & kHighQuality_BlurFlag) ?
                        kHigh_SkBlurQuality : kLow_SkBlurQuality;
    }
    return true;
}

NativeShadowLooper::Context* NativeShadowLooper::createContext(SkCanvas*, void* storage) const {
    return SkNEW_PLACEMENT_ARGS(storage, NativeShadowLooperContext, (this));
}


NativeShadowLooper::NativeShadowLooperContext::NativeShadowLooperContext(
        const NativeShadowLooper* looper)
    : fLooper(looper), fState(NativeShadowLooper::kBeforeEdge) {}

bool NativeShadowLooper::NativeShadowLooperContext::next(SkCanvas* canvas, SkPaint* paint) {
    U8CPU a;
    switch (fState) {
        case kBeforeEdge:
            // we do nothing if a maskfilter is already installed
            if (paint->getMaskFilter()) {
                fState = kDone;
                return false;
            }
            a = paint->getAlpha();
            paint->setColor(fLooper->fBlurColor);
            paint->setAlpha(SkAlphaMul(paint->getAlpha(), SkAlpha255To256(a)));

            paint->setShader(NULL);
            paint->setMaskFilter(fLooper->fBlur);

            canvas->save(SkCanvas::kMatrix_SaveFlag);

            if (fLooper->fBlurFlags & kIgnoreTransform_BlurFlag) {
                SkMatrix transform(canvas->getTotalMatrix());
                transform.postTranslate(fLooper->fDx, fLooper->fDy);
                canvas->setMatrix(transform);
            } else {
                canvas->translate(fLooper->fDx, fLooper->fDy);
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

#ifdef SK_IGNORE_TO_STRING
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
