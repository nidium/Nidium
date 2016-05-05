/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
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
        AudioNodeDelay(int inCount, int outCount, Audio *audio);

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

