#ifndef av_processor_gain_h__
#define av_processor_gain_h__

#include "../AudioNode.h"

namespace Nidium {
namespace AV {

class AudioProcessorGain: public AudioProcessor
{
  public:
    AudioProcessorGain() : m_Gain(1) {};

    void process(float *in, int *i)
    {
        *in  = *in * m_Gain;
    }

    void setGain(double gain)
    {
        m_Gain = gain;
    }

    ~AudioProcessorGain() {};
  private:
    double m_Gain;
};

} // namespace AV
} // namespace Nidium

#endif

