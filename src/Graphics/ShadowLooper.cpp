#include "Graphics/ShadowLooper.h"

#include <stdbool.h>

#include <SkBlurMaskFilter.h>
#include <SkCanvas.h>
#include <SkFlattenableBuffers.h>
#include <SkColorPriv.h>
#include <SkStringUtils.h>

namespace Nidium {
namespace Graphics {

void NativeShadowLooper::init(SkScalar sigma, SkScalar dx, SkScalar dy,
                            SkColor color, uint32_t flags) {
    m_fSigma = sigma;
    m_fDx = dx;
    m_fDy = dy;
    m_fBlurColor = color;
    m_fBlurFlags = flags;

    this->initEffects();
}

void NativeShadowLooper::initEffects()
{
    SkASSERT(m_fBlurFlags <= kAll_BlurFlag);
    if (m_fSigma > 0) {
        uint32_t flags = m_fBlurFlags & kIgnoreTransform_BlurFlag ?
                            SkBlurMaskFilter::kIgnoreTransform_BlurFlag :
                            SkBlurMaskFilter::kNone_BlurFlag;

        flags |= m_fBlurFlags & kHighQuality_BlurFlag ?
                    SkBlurMaskFilter::kHighQuality_BlurFlag :
                    SkBlurMaskFilter::kNone_BlurFlag;

        m_fBlur = SkBlurMaskFilter::Create(kNormal_SkBlurStyle, m_fSigma, flags);
    } else {
        m_fBlur = NULL;
    }

    m_fColorFilter = NULL;
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

    m_fSigma = buffer.readScalar();
    m_fDx = buffer.readScalar();
    m_fDy = buffer.readScalar();
    m_fBlurColor = buffer.readColor();
    m_fBlurFlags = buffer.readUInt() & kAll_BlurFlag;

    this->initEffects();
}

void NativeShadowLooper::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(m_fSigma);
    buffer.writeScalar(m_fDx);
    buffer.writeScalar(m_fDy);
    buffer.writeColor(m_fBlurColor);
    buffer.write32(m_fBlurFlags);
}

NativeShadowLooper::~NativeShadowLooper() {
    SkSafeUnref(m_fBlur);
    SkSafeUnref(m_fColorFilter);
}

bool NativeShadowLooper::asABlurShadow(BlurShadowRec* rec) const {
    if (m_fSigma <= 0 || (m_fBlurFlags & m_fBlurFlags & kIgnoreTransform_BlurFlag)) {
        return false;
    }

    if (rec) {
        rec->fSigma = m_fSigma;
        rec->fColor = m_fBlurColor;
        rec->fOffset.set(m_fDx, m_fDy);
        rec->fStyle = kNormal_SkBlurStyle;
        rec->fQuality = (m_fBlurFlags & kHighQuality_BlurFlag) ?
                        kHigh_SkBlurQuality : kLow_SkBlurQuality;
    }
    return true;
}

NativeShadowLooper::Context* NativeShadowLooper::createContext(SkCanvas*, void* storage) const {
    return SkNEW_PLACEMENT_ARGS(storage, NativeShadowLooperContext, (this));
}


NativeShadowLooper::NativeShadowLooperContext::NativeShadowLooperContext(
        const NativeShadowLooper* looper)
    : m_fLooper(looper), m_fState(NativeShadowLooper::kBeforeEdge) {}

bool NativeShadowLooper::NativeShadowLooperContext::next(SkCanvas* canvas, SkPaint* paint) {
    U8CPU a;
    switch (m_fState) {
        case kBeforeEdge:
            // we do nothing if a maskfilter is already installed
            if (paint->getMaskFilter()) {
                m_fState = kDone;
                return false;
            }
            a = paint->getAlpha();
            paint->setColor(m_fLooper->m_fBlurColor);
            paint->setAlpha(SkAlphaMul(paint->getAlpha(), SkAlpha255To256(a)));

            paint->setShader(NULL);
            paint->setMaskFilter(m_fLooper->m_fBlur);

            canvas->save(SkCanvas::kMatrix_SaveFlag);

            if (m_fLooper->m_fBlurFlags & kIgnoreTransform_BlurFlag) {
                SkMatrix transform(canvas->getTotalMatrix());
                transform.postTranslate(m_fLooper->m_fDx, m_fLooper->m_fDy);
                canvas->setMatrix(transform);
            } else {
                canvas->translate(m_fLooper->m_fDx, m_fLooper->m_fDy);
            }
            m_fState = kAfterEdge;
            return true;
        case kAfterEdge:
            canvas->restore();
            m_fState = kDone;
            return true;
        default:
            SkASSERT(kDone == m_fState);
            return false;
    }
}

#if 1
void NativeShadowLooper::toString(SkString* str) const {
    str->append("SkBlurDrawLooper: ");

    str->append("dx: ");
    str->appendScalar(m_fDx);

    str->append(" dy: ");
    str->appendScalar(m_fDy);

    str->append(" color: ");
    str->appendHex(m_fBlurColor);

    str->append(" flags: (");
    if (kNone_BlurFlag == m_fBlurFlags) {
        str->append("None");
    } else {
        bool needsSeparator = false;
        SkAddFlagToString(str, SkToBool(kIgnoreTransform_BlurFlag & m_fBlurFlags), "IgnoreTransform",
                          &needsSeparator);
        SkAddFlagToString(str, SkToBool(kOverrideColor_BlurFlag & m_fBlurFlags), "OverrideColor",
                          &needsSeparator);
        SkAddFlagToString(str, SkToBool(kHighQuality_BlurFlag & m_fBlurFlags), "HighQuality",
                          &needsSeparator);
    }
    str->append(")");

    // TODO: add optional "fBlurFilter->toString(str);" when SkMaskFilter::toString is added
    // alternatively we could cache the radius in SkBlurDrawLooper and just add it here
}

} // namespace Graphics
} // namespace Nidium

#endif

