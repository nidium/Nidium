#include "NativeSystem.h"
#import <Cocoa/Cocoa.h>

NativeSystem::NativeSystem()
{
    fbackingStorePixelRatio = [[NSScreen mainScreen] backingScaleFactor];
}

float NativeSystem::backingStorePixelRatio()
{
    return fbackingStorePixelRatio;
}
