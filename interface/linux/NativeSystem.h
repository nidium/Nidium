#ifndef nativesystem_h__
#define nativesystem_h__

#include "NativeSystemInterface.h"

class NativeSystem : public NativeSystemInterface
{
	public:
		NativeSystem();
		~NativeSystem(){};
		float backingStorePixelRatio();		
};

#endif