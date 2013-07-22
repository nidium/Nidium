#ifndef nativesysteminterface_h__
#define nativesysteminterface_h__

#include <stdlib.h>

class NativeSystem;

class NativeSystemInterface
{
	public:
		static NativeSystemInterface *_interface;

		virtual float backingStorePixelRatio()=0;

        static NativeSystemInterface* getInstance()
        {
        	return NativeSystemInterface::_interface;
        }
  	private:
		void operator=(NativeSystem const&);
	protected:
		float fbackingStorePixelRatio;
};


#endif