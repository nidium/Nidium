#include "NativeSystem.h"
#import <Cocoa/Cocoa.h>
#import <sys/stat.h>
#include <unistd.h>

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

static NSString *runCommand(NSString *commandToRun)
{
    NSTask *task;
    task = [[NSTask alloc] init];
    [task setLaunchPath: @"/bin/sh"];

    NSArray *arguments = [NSArray arrayWithObjects:
                          @"-c" ,
                          [NSString stringWithFormat:@"%@", commandToRun],
                          nil];
    NSLog(@"run command: %@",commandToRun);
    [task setArguments: arguments];

    NSPipe *pipe;
    pipe = [NSPipe pipe];
    [task setStandardOutput: pipe];

    NSFileHandle *file;
    file = [pipe fileHandleForReading];

    [task launch];

    NSData *data;
    data = [file readDataToEndOfFile];

    NSString *output;
    output = [[NSString alloc] initWithData: data encoding: NSUTF8StringEncoding];
    return output;
}

const char *NativeSystem::execute(const char *cmd)
{
    NSString *ret = runCommand([NSString stringWithCString:cmd encoding:NSUTF8StringEncoding]);

    return [ret UTF8String];
}

void NativeSystem::openURLInBrowser(const char *url)
{
    NSString *nsurl = [NSString stringWithCString:url encoding:NSASCIIStringEncoding];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:nsurl]];
}

const char *NativeSystem::getPrivateDirectory()
{
    static char parentdir[MAXPATHLEN];
    static bool resolved = false;

    if (resolved) {
        return parentdir;
    }

    parentdir[0] = '/';
    parentdir[1] = '\0';

    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
    if (CFURLGetFileSystemRepresentation(url2, 1, (UInt8 *)parentdir, MAXPATHLEN)) {
        strcat(parentdir, "/private/");
        resolved = true;
    }
    CFRelease(url);
    CFRelease(url2);

    return parentdir;
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

const char *NativeSystem::getUserDirectory()
{
    NSString* userDir = [NSString stringWithFormat:@"%@/", NSHomeDirectory()];

    if (!userDir) {
        return NULL;
    }

    return [userDir cStringUsingEncoding:NSUTF8StringEncoding];
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


const char *NativeSystem::pwd()
{
    static char dir[MAXPATHLEN];

    getcwd(dir, MAXPATHLEN);

    return dir;
}


void NativeSystem::sendNotification(const char *title, const char *content, bool sound)
{
    NSUserNotification *notification = [[NSUserNotification alloc] init];

    notification.title = [NSString stringWithCString:title encoding:NSUTF8StringEncoding];
    notification.informativeText = [NSString stringWithCString:content encoding:NSUTF8StringEncoding];
    if (sound) {
        notification.soundName = NSUserNotificationDefaultSoundName;
    }

    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
}
