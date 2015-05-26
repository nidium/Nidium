#include "NativeAudioNodeDelay.h"

NativeAudioNodeDelay::NativeAudioNodeDelay(int inCount, int outCount, NativeAudio *audio)
    : NativeAudioNodeProcessor(inCount, outCount, audio)
{
    this->args[0] = new ExportsArgs("wet",
        DOUBLE, WET, NativeAudioNodeDelay::argCallback);
    this->args[1] = new ExportsArgs("delay",
        INT, DELAY, NativeAudioNodeDelay::argCallback);
    this->args[2] = new ExportsArgs("feedback",
        DOUBLE, FEEDBACK, NativeAudioNodeDelay::argCallback);

    m_DelayProcessor = new NativeAudioProcessorDelay(this->audio->outputParameters->sampleRate, 2000);

    this->setProcessor(0, m_DelayProcessor);
    this->setProcessor(1, m_DelayProcessor);
}

void NativeAudioNodeDelay::argCallback(NativeAudioNode *node, int id, void *tmp, int size)
{
    NativeAudioNodeDelay *thiz = static_cast<NativeAudioNodeDelay*>(node);
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

