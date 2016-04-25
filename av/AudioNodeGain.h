#ifndef av_audionodegain_h__
#define av_audionodegain_h__

#include "Audio.h"
#include "AudioNode.h"

namespace Nidium {
namespace AV {

class AudioProcessorGain;

class AudioNodeGain: public NativeAudioNodeProcessor
{
    public :
        AudioNodeGain(int inCount, int outCount, NativeAudio *audio);

        enum Args {
            GAIN
        };

        static void argCallback(NativeAudioNode *node, int id, void *val, int size);

        ~AudioNodeGain() {};
    private :
        AudioProcessorGain *m_GainProcessor;
};

} // namespace AV
} // namespace Nidium

#endif

