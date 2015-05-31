#ifndef nativeshadowlooper_h__
#define nativeshadowlooper_h__

#include "SkDrawLooper.h"
#include "SkColor.h"
#include "SkBlurMask.h"

class SkMaskFilter;
class SkColorFilter;

/** \class NativeShadowLooper
    This class draws a shadow of the object (possibly offset), and then draws
    the original object in its original position.
    should there be an option to just draw the shadow/blur layer? webkit?
*/
class SK_API NativeShadowLooper : public SkDrawLooper {
public:
    enum BlurFlags {
        kNone_BlurFlag = 0x00,
        /**
            The blur layer's dx/dy/radius aren't affected by the canvas
            transform.
        */
        kIgnoreTransform_BlurFlag   = 0x01,
        kOverrideColor_BlurFlag     = 0x02,
        kHighQuality_BlurFlag       = 0x04,
        /** mask for all blur flags */
        kAll_BlurFlag = 0x07
    };

    static NativeShadowLooper* Create(SkColor color, SkScalar sigma, SkScalar dx, SkScalar dy,
                                    uint32_t flags = kNone_BlurFlag) {
        return SkNEW_ARGS(NativeShadowLooper, (color, sigma, dx, dy, flags));
    }

    static NativeShadowLooper* Create(SkScalar radius, SkScalar dx, SkScalar dy,
                                   SkColor color, uint32_t flags) {
        return SkNEW_ARGS(NativeShadowLooper, (color, SkBlurMask::ConvertRadiusToSigma(radius), dx, dy, flags));
    }

    virtual ~NativeShadowLooper();

    virtual NativeShadowLooper::Context* createContext(SkCanvas*, void* storage) const SK_OVERRIDE;
    virtual size_t contextSize() const SK_OVERRIDE { return sizeof(NativeShadowLooperContext); }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(NativeShadowLooper)

protected:

    NativeShadowLooper(SkColor color, SkScalar sigma, SkScalar dx, SkScalar dy,
                    uint32_t flags = kNone_BlurFlag);
    NativeShadowLooper(SkScalar radius, SkScalar dx, SkScalar dy,
                    SkColor color, uint32_t flags = kNone_BlurFlag);
    NativeShadowLooper(SkReadBuffer&);

    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;
    virtual bool asABlurShadow(BlurShadowRec*) const SK_OVERRIDE;

private:
    SkMaskFilter*   fBlur;
    SkColorFilter*  fColorFilter;
    SkScalar        fDx, fDy, fSigma;
    SkColor         fBlurColor;
    uint32_t        fBlurFlags;

    enum State {
        kBeforeEdge,
        kAfterEdge,
        kDone
    };
    class NativeShadowLooperContext : public SkDrawLooper::Context {
    public:
        explicit NativeShadowLooperContext(const NativeShadowLooper* looper);

        virtual bool next(SkCanvas* canvas, SkPaint* paint) SK_OVERRIDE;

    private:
        const NativeShadowLooper* fLooper;
        State fState;
    };

    void init(SkScalar sigma, SkScalar dx, SkScalar dy, SkColor color, uint32_t flags);
    void initEffects();

    typedef SkDrawLooper INHERITED;
};

#endif

