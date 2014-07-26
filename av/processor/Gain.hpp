#ifndef nativeaudioprocessorgain_h__
#define nativeaudioprocessorgain_h__

#include "../NativeAudioNode.h"

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
#endif
