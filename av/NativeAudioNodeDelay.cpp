/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeAudioNodeDelay.h"

NativeAudioNodeDelay::NativeAudioNodeDelay(int inCount, int outCount, NativeAudio *audio)
    : NativeAudioNodeProcessor(inCount, outCount, audio)
{
    m_Args[0] = new ExportsArgs("wet", DOUBLE, WET, NativeAudioNodeDelay::argCallback);
    m_Args[1] = new ExportsArgs("delay", INT, DELAY, NativeAudioNodeDelay::argCallback);
    m_Args[2] = new ExportsArgs("feedback", DOUBLE, FEEDBACK, NativeAudioNodeDelay::argCallback);

    m_DelayProcessor = new NativeAudioProcessorDelay(m_Audio->m_OutputParameters->m_SampleRate, 2000);

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

