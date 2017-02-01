/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/ShadowLooper.h"

#include <stdbool.h>

#include <SkBlurMaskFilter.h>
#include <SkCanvas.h>
#include <SkReadBuffer.h>
#include <SkWriteBuffer.h>
#include <SkColorPriv.h>
#include <SkStringUtils.h>

namespace Nidium {
namespace Graphics {


/// {{{ Construct and init
ShadowLooper::ShadowLooper(
    SkScalar radius, SkScalar dx, SkScalar dy, SkColor color, uint32_t flags)
{
    this->init(SkBlurMask::ConvertRadiusToSigma(radius), dx, dy, color, flags);
}

ShadowLooper::ShadowLooper(
    SkColor color, SkScalar sigma, SkScalar dx, SkScalar dy, uint32_t flags)
{
    this->init(sigma, dx, dy, color, flags);
}


void ShadowLooper::init(
    SkScalar sigma, SkScalar dx, SkScalar dy, SkColor color, uint32_t flags)
{
    m_fSigma     = sigma;
    m_fDx        = dx;
    m_fDy        = dy;
    m_fBlurColor = color;
    m_fBlurFlags = flags;

    this->initEffects();
}

void ShadowLooper::initEffects()
{
    SkASSERT(m_fBlurFlags <= kAll_BlurFlag);
    if (m_fSigma > 0) {
        uint32_t flags = m_fBlurFlags & kIgnoreTransform_BlurFlag
                             ? SkBlurMaskFilter::kIgnoreTransform_BlurFlag
                             : SkBlurMaskFilter::kNone_BlurFlag;

        flags |= m_fBlurFlags & kHighQuality_BlurFlag
                     ? SkBlurMaskFilter::kHighQuality_BlurFlag
                     : SkBlurMaskFilter::kNone_BlurFlag;

        m_fBlur
            = SkBlurMaskFilter::Make(kNormal_SkBlurStyle, m_fSigma, flags);
    } else {
        m_fBlur = NULL;
    }

    m_fColorFilter = NULL;
}

sk_sp<SkFlattenable> ShadowLooper::CreateProc(SkReadBuffer& buffer) {
    const SkColor color = buffer.readColor();
    const SkScalar sigma = buffer.readScalar();
    const SkScalar dx = buffer.readScalar();
    const SkScalar dy = buffer.readScalar();
    const uint32_t flags = buffer.read32();

    return Make(color, sigma, dx, dy, flags);
}


void ShadowLooper::flatten(SkWriteBuffer &buffer) const
{
    buffer.writeColor(m_fBlurColor);
    buffer.writeScalar(m_fSigma);
    buffer.writeScalar(m_fDx);
    buffer.writeScalar(m_fDy);
    buffer.write32(m_fBlurFlags);
}

bool ShadowLooper::asABlurShadow(BlurShadowRec *rec) const
{
    if (m_fSigma <= 0 || (m_fBlurFlags & kIgnoreTransform_BlurFlag)) {
        return false;
    }

    if (rec) {
        rec->fSigma = m_fSigma;
        rec->fColor = m_fBlurColor;
        rec->fOffset.set(m_fDx, m_fDy);
        rec->fStyle   = kNormal_SkBlurStyle;
        rec->fQuality = (m_fBlurFlags & kHighQuality_BlurFlag)
                            ? kHigh_SkBlurQuality
                            : kLow_SkBlurQuality;
    }
    return true;
}

#if 1
void ShadowLooper::toString(SkString *str) const
{
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
        SkAddFlagToString(str,
                          SkToBool(kIgnoreTransform_BlurFlag & m_fBlurFlags),
                          "IgnoreTransform", &needsSeparator);
        SkAddFlagToString(str, SkToBool(kOverrideColor_BlurFlag & m_fBlurFlags),
                          "OverrideColor", &needsSeparator);
        SkAddFlagToString(str, SkToBool(kHighQuality_BlurFlag & m_fBlurFlags),
                          "HighQuality", &needsSeparator);
    }
    str->append(")");

    // TODO: add optional "fBlurFilter->toString(str);" when
    // SkMaskFilter::toString is added
    // alternatively we could cache the radius in SkBlurDrawLooper and just add
    // it here
}
// }}}

// {{{ Context
ShadowLooper::Context *ShadowLooper::createContext(SkCanvas *,
                                                   void *storage) const
{
    return new (storage) ShadowLooperContext(this);
}


ShadowLooper::ShadowLooperContext::ShadowLooperContext(
    const ShadowLooper *looper)
    : m_fLooper(looper), m_fState(ShadowLooper::kBeforeEdge)
{
}

bool ShadowLooper::ShadowLooperContext::next(SkCanvas *canvas, SkPaint *paint)
{
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

            canvas->save();

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


} // namespace Graphics
} // namespace Nidium

#endif
