#include "AudioNodeDelay.h"

namespace Nidium {
namespace AV {

AudioNodeDelay::AudioNodeDelay(int inCount, int outCount, Audio *audio)
    : AudioNodeProcessor(inCount, outCount, audio)
{
    m_Args[0] = new ExportsArgs("wet", DOUBLE, WET, AudioNodeDelay::argCallback);
    m_Args[1] = new ExportsArgs("delay", INT, DELAY, AudioNodeDelay::argCallback);
    m_Args[2] = new ExportsArgs("feedback", DOUBLE, FEEDBACK, AudioNodeDelay::argCallback);

    m_DelayProcessor = new AudioProcessorDelay(m_Audio->m_OutputParameters->m_SampleRate, 2000);

    this->setProcessor(0, m_DelayProcessor);
    this->setProcessor(1, m_DelayProcessor);
}

void AudioNodeDelay::argCallback(AudioNode *node, int id, void *tmp, int size)
{
    AudioNodeDelay *thiz = static_cast<AudioNodeDelay*>(node);
    switch (id) {
        case DELAY:
            thiz->m_DelayProcessor->setDelay(*(int*)tmp);
        break;
        case WET:
            thiz->m_DelayProcessor->setWet(*(double*)tmp);
        break;
        case FEEDBACK:
            thiz->m_DelayProcessor->setFeedback(*(double*)tmp);
        break;
    }
}

} // namespace AV
} // namespace Nidium

