#include "NativeAudioNodeGain.h"
#include "processor/Gain.hpp"

NativeAudioNodeGain::NativeAudioNodeGain(int inCount, int outCount, NativeAudio *audio) 
    : NativeAudioNodeProcessor(inCount, outCount, audio), m_Gain(1)
{
    this->args[0] = new ExportsArgs("gain", 
        DOUBLE, GAIN, NativeAudioNodeGain::argCallback);

    m_GainProcessor = new NativeAudioProcessorGain();

    this->setProcessor(0, m_GainProcessor);
    this->setProcessor(1, m_GainProcessor);
}

void NativeAudioNodeGain::argCallback(NativeAudioNode *node, int id, void *tmp, int size)
{
    NativeAudioNodeGain *thiz = static_cast<NativeAudioNodeGain*>(node);
    switch (id) {
        case GAIN:
            thiz->m_GainProcessor->setGain(*(double*)tmp);
        break;
    }
}

