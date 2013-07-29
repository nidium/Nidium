#import "NativeStudioAppDelegate.h"
#import <OpenGL/gl.h>
#import <SDL.h>
#import <SDL_opengl.h>
#import <SDL_video.h>
#import <SDL_syswm.h>
#import <pthread.h>

#import "NativeCocoaUIInterface.h"
#import "NativeSystem.h"
#import <dispatch/dispatch.h>


NativeSystemInterface *NativeSystemInterface::_interface = new NativeSystem();

int ape_running = 1;
int _nativebuild = 1002;
unsigned long _ape_seed;

@implementation NativeStudioAppDelegate

@synthesize window = _window;
@synthesize position, appfile;

- (void)dealloc
{
    [super dealloc];
}

- (id) init
{
    self = [super init];

    if (self) {

        [[NSAppleEventManager sharedAppleEventManager] setEventHandler:self andSelector:@selector(handleURLEvent:withReplyEvent:) forEventClass:kInternetEventClass andEventID:kAEGetURL];

        CFStringRef bundleID = (CFStringRef)[[NSBundle mainBundle] bundleIdentifier];
        OSStatus nativeResult = LSSetDefaultHandlerForURLScheme(CFSTR("native"), bundleID);
    }
    return self;
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

- (void)handleURLEvent:(NSAppleEventDescriptor*)event withReplyEvent:(NSAppleEventDescriptor*)replyEvent
{
    NSString* filename = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
    
    NSString *f = [[[NSString alloc] initWithFormat:@"http%@", [filename substringFromIndex:sizeof("native")-1]] autorelease];

    self.appfile = f;

    if (self->isRunning) {
        self->UI->restartApplication([f UTF8String]);
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{

    [self setupWorkingDirectory:YES];
    _ape_seed = time(NULL) ^ (getpid() << 16);
    //NativeConsole *console = [[NativeConsole alloc] init];
    //[console attachToStdout];
    NativeCocoaUIInterface *nUI = new NativeCocoaUIInterface;
    [self.window close];

    self->UI = nUI;
    self->isRunning = YES;

    dispatch_async(dispatch_get_main_queue(), ^{
        const char *filename;
        
        if (self.appfile == nil) {
            filename = "index.nml";
        } else {
            filename = [self.appfile UTF8String];
        }

        NSLog(@"Launching... %s\n", filename);
        if (!nUI->runApplication(filename)) {
            [[NSApplication sharedApplication] terminate:nil];
            return;
        }

        nUI->runLoop();
    });

}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{

    NSLog(@"Running? %d", self->isRunning);
    NSLog(@"drop : %@", filename);

    self.appfile = filename;

    if (self->isRunning) {
        self->UI->restartApplication([filename UTF8String]);
    }
    
    return true;
}

@end
