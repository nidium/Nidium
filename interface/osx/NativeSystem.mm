#include "NativeSystem.h"
#import <Cocoa/Cocoa.h>
#import <sys/stat.h>

NativeSystem::NativeSystem()
{
    fbackingStorePixelRatio = [[NSScreen mainScreen] backingScaleFactor];
}

float NativeSystem::backingStorePixelRatio()
{
    return fbackingStorePixelRatio;
}

const char *NativeSystem::getCacheDirectory()
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString* cacheDir = [paths objectAtIndex:0];

    if (cacheDir) {
        NSString *path = [NSString stringWithFormat:@"%@/nidium/",cacheDir];
        const char *cpath = [path cStringUsingEncoding:NSASCIIStringEncoding];
        if (mkdir(cpath, 0777) == -1 && errno != EEXIST) {
            printf("Cant create cache directory %s\n", cpath);
            return NULL;
        }  
        return cpath;
    }
    return NULL;
}