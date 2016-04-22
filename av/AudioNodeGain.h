#ifndef nidium_audionodegain_h__
#define nidium_audionodegain_h__

#include "Audio.h"
#include "AudioNode.h"

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

#endif

