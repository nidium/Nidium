#include "NativeSystem.h"

NativeSystem::NativeSystem()
{
	fbackingStorePixelRatio = 1.0;
}

float NativeSystem::backingStorePixelRatio()
{
	return fbackingStorePixelRatio;
}
