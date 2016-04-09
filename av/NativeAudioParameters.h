/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativeaudionparameters_h__
#define nativeaudionparameters_h__

class NativeAudioParameters {
    public :
        int m_BufferSize, m_Channels, m_SampleFmt, m_SampleRate, m_FramesPerBuffer;
        NativeAudioParameters(int bufferSize, int channels,
                              int sampleFmt, int sampleRate)
            : m_BufferSize(bufferSize), m_Channels(channels),
              m_SampleFmt(sampleFmt), m_SampleRate(sampleRate),
              m_FramesPerBuffer(bufferSize/(sampleFmt * channels))
        {
        }
};

enum {
    NATIVE_AUDIO_NODE_SET,
    NATIVE_AUDIO_NODE_CALLBACK,
    NATIVE_AUDIO_TRACK_CALLBACK,
    NATIVE_AUDIO_SHUTDOWN
};
#endif

