/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef av_audionparameters_h__
#define av_audionparameters_h__

namespace Nidium {
namespace AV {

class AudioParameters
{
public:
    int m_BufferSize, m_Channels, m_SampleFmt, m_SampleRate, m_FramesPerBuffer;
    AudioParameters(int bufferSize, int channels, int sampleFmt, int sampleRate)
        : m_BufferSize(bufferSize), m_Channels(channels),
          m_SampleFmt(sampleFmt), m_SampleRate(sampleRate),
          m_FramesPerBuffer(bufferSize / (sampleFmt * channels))
    {
    }
};

} // namespace AV
} // namespace Nidium

#endif
