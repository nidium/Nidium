#ifndef av_processor_gain_h__
#define av_processor_gain_h__

#include "../AudioNode.h"

namespace Nidium {
namespace AV {

class NativeAudioProcessorGain: public NativeAudioProcessor
{
  public:
    NativeAudioProcessorGain() : m_Gain(1) {};

    void process(float *in, int *i)
    {
        *in  = *in * m_Gain;
    }

    void setGain(double gain)
    {
        m_Gain = gain;
    }

    ~NativeAudioProcessorGain() {};
  private:
    double m_Gain;
};

} // namespace AV
} // namespace Nidium

#endif

