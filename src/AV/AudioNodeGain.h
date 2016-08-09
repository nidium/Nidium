/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef av_audionodegain_h__
#define av_audionodegain_h__

#include "Audio.h"
#include "AudioNode.h"

namespace Nidium {
namespace AV {

class AudioProcessorGain;

class AudioNodeGain : public AudioNodeProcessor
{
public:
    AudioNodeGain(int inCount, int outCount, Audio *audio);

    enum Args
    {
        GAIN
    };

    static void argCallback(AudioNode *node, int id, void *val, int size);

    ~AudioNodeGain(){};

private:
    AudioProcessorGain *m_GainProcessor;
};

} // namespace AV
} // namespace Nidium

#endif
