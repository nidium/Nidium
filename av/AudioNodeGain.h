#ifndef av_audionodegain_h__
#define av_audionodegain_h__

#include "Audio.h"
#include "AudioNode.h"

namespace Nidium {
namespace AV {

class NativeAudioProcessorGain;

class NativeAudioNodeGain: public NativeAudioNodeProcessor
{
    public :
        NativeAudioNodeGain(int inCount, int outCount, NativeAudio *audio);

        enum Args {
            GAIN
        };

        static void argCallback(NativeAudioNode *node, int id, void *val, int size);

        ~NativeAudioNodeGain() {};
    private :
        NativeAudioProcessorGain *m_GainProcessor;
};

} // namespace AV
} // namespace Nidium

#endif

