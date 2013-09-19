#ifndef nativesysteminterface_h__
#define nativesysteminterface_h__

#include <stdlib.h>

class NativeSystem;

class NativeSystemInterface
{
    public:
        virtual float backingStorePixelRatio()=0;
        virtual const char *getCacheDirectory()=0;

        static NativeSystemInterface* getInstance()
        {
            return NativeSystemInterface::_interface;
        }
        static NativeSystemInterface *_interface;
    private:
        void operator=(NativeSystem const&);
    protected:
        float fbackingStorePixelRatio;
};


#endif