#import "NativeStudioAppDelegate.h"
#import <OpenGL/gl.h>
#import <SDL.h>
#import <SDL_opengl.h>
#import <SDL_video.h>
#import <SDL_syswm.h>
#import <pthread.h>

#import "NativeCocoaUIInterface.h"


int ape_running = 1;

@implementation NativeStudioAppDelegate

@synthesize window = _window;
@synthesize position;

- (void)dealloc
{
    [super dealloc];
}

- (void) setupWorkingDirectory:(BOOL)shouldChdir
{
    if (shouldChdir)
    {
        char parentdir[MAXPATHLEN];
        CFURLRef url = CFBundleCopyBundleURL(CFBundleGetMainBundle());
        CFURLRef url2 = CFURLCreateCopyDeletingLastPathComponent(0, url);
        if (CFURLGetFileSystemRepresentation(url2, 1, (UInt8 *)parentdir, MAXPATHLEN)) {
            chdir(parentdir);   /* chdir to the binary app's parent */
        }
        CFRelease(url);
        CFRelease(url2);
    }
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [self setupWorkingDirectory:YES];
    //NativeConsole *console = [[NativeConsole alloc] init];
    //[console attachToStdout];
    NativeCocoaUIInterface UI;
    [self.window close];
    if (!UI.runApplication("native.npa")) {
        [[NSApplication sharedApplication] terminate:nil];
        return;
    }
    
    UI.runLoop();
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    NSLog(@"drop : %@", filename);
    
    return true;
}

@end
