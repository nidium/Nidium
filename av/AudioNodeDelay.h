#ifndef av_audionodedelay_h__
#define av_audionodedelay_h__

#include "Core/Utils.h"
#include "Audio.h"
#include "AudioNode.h"

#include "processor/Delay.h"

namespace Nidium {
namespace AV {

class AudioProcessorDelay;

class AudioNodeDelay: public AudioNodeProcessor
{
    public :
        AudioNodeDelay(int inCount, int outCount, NativeAudio *audio);

        enum Args {
            WET, DELAY, FEEDBACK
        };

        static void argCallback(AudioNode *node, int id, void *val, int size);

        ~AudioNodeDelay() {};
    private :
        AudioProcessorDelay *m_DelayProcessor;
};

} // namespace AV
} // namespace Nidium

#endif

