#import "NativeStudioAppDelegate.h"
#import <OpenGL/gl.h>
#import <SDL.h>
#import <SDL_opengl.h>
#import <SDL_video.h>
#import <SDL_syswm.h>
#import <pthread.h>

#import "NativeCocoaUIInterface.h"
#import "NativeSystem.h"

NativeSystemInterface *NativeSystemInterface::_interface = new NativeSystem();

int ape_running = 1;
int _nativebuild = 1002;
unsigned long _ape_seed;

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
    _ape_seed = time(NULL) ^ (getpid() << 16);
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
