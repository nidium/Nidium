#ifndef nativeaudionparameters_h__
#define nativeaudionparameters_h__

class NativeAudioParameters {
    public :
        int bufferSize, channels, sampleFmt, sampleRate, framesPerBuffer;
        NativeAudioParameters(int bufferSize, int channels, 
                              int sampleFmt, int sampleRate)
            : bufferSize(bufferSize), channels(channels), 
              sampleFmt(sampleFmt), sampleRate(sampleRate), 
              framesPerBuffer(bufferSize/(sampleFmt * channels)) 
        {
        }
};


enum {
    NATIVE_AUDIO_NODE_SET,
    NATIVE_AUDIO_SHUTDOWN
};
#endif
