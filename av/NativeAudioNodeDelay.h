/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativeaudionodedelay_h__
#define nativeaudionodedelay_h__

#include "NativeUtils.h"
#include "NativeAudio.h"
#include "NativeAudioNode.h"

#include "processor/Delay.hpp"

class NativeAudioProcessorDelay;

class NativeAudioNodeDelay: public NativeAudioNodeProcessor
{
    public :
        NativeAudioNodeDelay(int inCount, int outCount, NativeAudio *audio);

        enum Args {
            WET, DELAY, FEEDBACK
        };

        static void argCallback(NativeAudioNode *node, int id, void *val, int size);

        ~NativeAudioNodeDelay() {};
    private :
        NativeAudioProcessorDelay *m_DelayProcessor;
};
#endif

