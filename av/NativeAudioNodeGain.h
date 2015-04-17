#ifndef nativeaudionodegain_h__
#define nativeaudionodegain_h__

#include "NativeAudio.h"
#include "NativeAudioNode.h"

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
