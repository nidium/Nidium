/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
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

