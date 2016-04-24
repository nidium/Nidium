#ifndef av_processor_delay_h__
#define av_processor_delay_h__

#include <math.h>

#include "Core/Utils.h"
#include "../AudioNode.h"

namespace Nidium {
namespace AV {

class NativeAudioProcessorDelay: public NativeAudioProcessor
{
  public:
    NativeAudioProcessorDelay(int sampleRate, int maxDelay)
        : m_Buffer(NULL), m_Delay(0), m_Wet(1), m_Feedback(0), m_BuffIndex(0),
          m_Idx(0), m_MaxDelay(maxDelay), m_SamplRate(sampleRate)
    {
        int size = ceil(m_MaxDelay/1000) * sampleRate;

        m_Buffer = (float *)calloc(size, NativeAudio::FLOAT32);
        m_BufferSize = size;
        m_MaxSamples = m_BufferSize * 0.5;

        this->setWet(1);
        this->setDelay(0);
        this->setFeedback(0.15);
    };

    void process(float *in, int *i)
    {
        int j = 0;

        if (m_Idx >= m_BufferSize) m_Idx = 0;
        j = m_Idx - m_BuffIndex;

        if (j < 0) j += m_BufferSize;

        *in = *in + m_Wet *(m_Buffer[j] * m_Feedback);
        m_Buffer[m_Idx++] = *in;
    }

    void setDelay(int delay)
    {
        int size = ceil(m_MaxDelay/1000) * m_SamplRate * NativeAudio::FLOAT32;
        memset(m_Buffer, 0.0, size);

        m_Idx = 0;
        m_Delay = nidium_max(nidium_min(delay, m_MaxDelay), 1);
        m_BuffIndex = ceil((m_Delay/1000) * m_MaxSamples);
    }

    void setFeedback(double feedback)
    {
        m_Feedback = nidium_max(nidium_min(feedback, 0.8), 0.0);
    }

    void setWet(double wet)
    {
        m_Wet = nidium_max(nidium_min(wet, 1), 0);
    }

    ~NativeAudioProcessorDelay() {
        free(m_Buffer);
    };
  private:
    float *m_Buffer;
    double m_Delay;
    double m_Wet;
    double m_Feedback;
    int m_BufferSize;
    int m_BuffIndex;
    int m_Idx;
    int m_MaxSamples;
    int m_MaxDelay;
    int m_SamplRate;
};

} // namespace AV
} // namespace Nidium

#endif
