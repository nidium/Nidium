/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_shadowlooper_h__
#define graphics_shadowlooper_h__

#include <SkDrawLooper.h>
#include <SkColor.h>
#include <SkBlurMask.h>

class SkArenaAlloc;
class SkMaskFilter;
class SkColorFilter;

namespace Nidium {
namespace Graphics {

/** class ShadowLooper
    This class draws a shadow of the object (possibly offset), and then draws
    the original object in its original position.
    should there be an option to just draw the shadow/blur layer? webkit?
*/
class SK_API ShadowLooper : public SkDrawLooper
{
public:
    enum BlurFlags
    {
        kNone_BlurFlag = 0x00,
        /**
            The blur layer's dx/dy/radius aren't affected by the canvas
            transform.
        */
        kIgnoreTransform_BlurFlag = 0x01,
        kOverrideColor_BlurFlag   = 0x02,
        kHighQuality_BlurFlag     = 0x04,
        /** mask for all blur flags */
        kAll_BlurFlag = 0x07
    };

    static sk_sp<ShadowLooper> Make(SkColor color,
                                SkScalar sigma,
                                SkScalar dx,
                                SkScalar dy,
                                uint32_t flags = kNone_BlurFlag)
    {
        return sk_sp<ShadowLooper>(new ShadowLooper(color, sigma, dx, dy, flags));
    }

    static sk_sp<ShadowLooper> Make(SkScalar radius,
                                SkScalar dx,
                                SkScalar dy,
                                SkColor color,
                                uint32_t flags)
    {
        return sk_sp<ShadowLooper>(new ShadowLooper(color,
            SkBlurMask::ConvertRadiusToSigma(radius), dx, dy, flags));
    }

    ShadowLooper::Context *makeContext(SkCanvas *, SkArenaAlloc*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(ShadowLooper)

protected:
    ShadowLooper(SkColor color,
                 SkScalar sigma,
                 SkScalar dx,
                 SkScalar dy,
                 uint32_t flags = kNone_BlurFlag);
    ShadowLooper(SkScalar radius,
                 SkScalar dx,
                 SkScalar dy,
                 SkColor color,
                 uint32_t flags = kNone_BlurFlag);
    ShadowLooper(SkReadBuffer &);

    void flatten(SkWriteBuffer &) const override;
    bool asABlurShadow(BlurShadowRec *) const override;

private:
    sk_sp<SkMaskFilter>  m_fBlur;
    sk_sp<SkColorFilter> m_fColorFilter;
    SkScalar m_fDx, m_fDy, m_fSigma;
    SkColor m_fBlurColor;
    uint32_t m_fBlurFlags;

    enum State
    {
        kBeforeEdge,
        kAfterEdge,
        kDone
    };
    class ShadowLooperContext : public SkDrawLooper::Context
    {
    public:
        explicit ShadowLooperContext(const ShadowLooper *looper);

        bool next(SkCanvas *canvas, SkPaint *paint) override;

    private:
        const ShadowLooper *m_fLooper;
        State m_fState;
    };

    void init(SkScalar sigma,
              SkScalar dx,
              SkScalar dy,
              SkColor color,
              uint32_t flags);
    void initEffects();

    typedef SkDrawLooper INHERITED;
};

} // namespace Graphics
} // namespace Nidium

#endif
