#include "NativeSystem.h"
#import <Cocoa/Cocoa.h>
#import <sys/stat.h>

NativeSystem::NativeSystem()
{
#if NIDIUM_ENABLE_HIDPI
    fbackingStorePixelRatio = [[NSScreen mainScreen] backingScaleFactor];
#else
    fbackingStorePixelRatio = 1.0;
#endif
    printf("Canvas Ratio (HIDPI) : %f\n", fbackingStorePixelRatio);
}

float NativeSystem::backingStorePixelRatio()
{
    return fbackingStorePixelRatio;
}

const char *NativeSystem::getPrivateDirectory()
{
    return "private/";
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

void NativeSystem::alert(const char *message, AlertType type)
{
    NSAlert *alert = [[NSAlert alloc] init];

    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"nidium"];
    [alert setInformativeText:[NSString stringWithCString:message encoding:NSUTF8StringEncoding]];

    [alert setAlertStyle:type];

    [alert runModal];

    [alert release];
}
