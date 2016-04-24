#ifndef av_audionodedelay_h__
#define av_audionodedelay_h__

#include "Core/Utils.h"
#include "Audio.h"
#include "AudioNode.h"

#include "processor/Delay.h"

namespace Nidium {
namespace AV {

class NativeAudioProcessorDelay;

class NativeAudioNodeDelay: public NativeAudioNodeProcessor
{
    public :
        NativeAudioNodeDelay(int inCount, int outCount, NativeAudio *audio);

        enum Args {
            WET, DELAY, FEEDBACK
        };

        static void argCallback(NativeAudioNode *node, int id, void *val, int size);

        ~NativeAudioNodeDelay() {};
    private :
        NativeAudioProcessorDelay *m_DelayProcessor;
};

} // namespace AV
} // namespace Nidium

#endif

