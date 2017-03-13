/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "System.h"

#import <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#import <Cocoa/Cocoa.h>

namespace Nidium {
namespace Interface {

System::System()
{
#if NIDIUM_ENABLE_HIDPI
    m_fBackingStorePixelRatio = [[NSScreen mainScreen] backingScaleFactor];
#else
    m_fBackingStorePixelRatio = 1.0;
#endif
    ndm_logf(NDM_LOG_DEBUG, "System", "Canvas Ratio (HIDPI) : %f", m_fBackingStorePixelRatio);

    char embedPath[MAXPATHLEN];

    CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
    if (CFURLGetFileSystemRepresentation(url2, 1, (UInt8 *)embedPath, MAXPATHLEN)) {
        char *dir = dirname(embedPath);
        snprintf(m_EmbedPath, MAXPATHLEN, "%s/%s", dir, "src/Embed/");
    } else {
        m_EmbedPath[0] = '/';
        m_EmbedPath[1] = '\0';
    }
    CFRelease(url);
    CFRelease(url2);
}

float System::backingStorePixelRatio()
{
    return m_fBackingStorePixelRatio;
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

const char *System::execute(const char *cmd)
{
    NSString *ret = runCommand([NSString stringWithCString:cmd encoding:NSUTF8StringEncoding]);

    return [ret UTF8String];
}

void System::openURLInBrowser(const char *url)
{
    NSString *nsurl = [NSString stringWithCString:url encoding:NSASCIIStringEncoding];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:nsurl]];
}

const char *System::getEmbedDirectory()
{
    return m_EmbedPath;
}

const char *System::getCacheDirectory()
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString* cacheDir = [paths objectAtIndex:0];

    if (cacheDir) {
        NSString *path = [NSString stringWithFormat:@"%@/nidium/",cacheDir];
        const char *cpath = [path cStringUsingEncoding:NSASCIIStringEncoding];
        if (mkdir(cpath, 0777) == -1 && errno != EEXIST) {
            ndm_logf(NDM_LOG_ERROR, "System", "Can't create cache directory %s", cpath);
            return NULL;
        }
        return cpath;
    }
    return NULL;
}

const char *System::getUserDirectory()
{
    NSString* userDir = [NSString stringWithFormat:@"%@/", NSHomeDirectory()];

    if (!userDir) {
        return NULL;
    }

    return [userDir cStringUsingEncoding:NSUTF8StringEncoding];
}

void System::alert(const char *message, AlertType type)
{
    NSAlert *alert = [[NSAlert alloc] init];

    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"nidium"];
    [alert setInformativeText:[NSString stringWithCString:message encoding:NSUTF8StringEncoding]];

    [alert setAlertStyle:(NSAlertStyle)type];

    [alert runModal];

    [alert release];
}


const char *System::cwd()
{
    static char dir[MAXPATHLEN];

    getcwd(dir, MAXPATHLEN);

    return dir;
}

const char *System::getLanguage()
{
    NSString* language = [[NSLocale preferredLanguages] objectAtIndex:0];
    const char *clang = [language cStringUsingEncoding:NSASCIIStringEncoding];

    return clang;
}

void System::sendNotification(const char *title, const char *content, bool sound)
{
    NSUserNotification *notification = [[NSUserNotification alloc] init];

    notification.title = [NSString stringWithCString:title encoding:NSUTF8StringEncoding];
    notification.informativeText = [NSString stringWithCString:content encoding:NSUTF8StringEncoding];
    if (sound) {
        notification.soundName = NSUserNotificationDefaultSoundName;
    }

    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];
}

} // namespace Interface
} // namespace Nidium

