/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeAudioNodeGain.h"
#include "processor/Gain.hpp"

NativeAudioNodeGain::NativeAudioNodeGain(int inCount, int outCount, NativeAudio *audio)
    : NativeAudioNodeProcessor(inCount, outCount, audio)
{
    m_Args[0] = new ExportsArgs("gain", DOUBLE, GAIN, NativeAudioNodeGain::argCallback);
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

