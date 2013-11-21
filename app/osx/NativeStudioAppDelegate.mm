#import "NativeStudioAppDelegate.h"
#import <pthread.h>

#import "NativeCocoaUIInterface.h"
#import "NativeSystem.h"
#import <dispatch/dispatch.h>

NativeSystemInterface *NativeSystemInterface::_interface = new NativeSystem();
NativeUIInterface *__NativeUI;

int ape_running = 1;
int _nativebuild = 1002;
unsigned long _ape_seed;

@implementation NativeStudioAppDelegate

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
    __NativeUI = nUI;
    
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

#ifdef NATIVE_ENABLE_BREAKPAD
static BreakpadRef InitBreakpad(void) {
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    BreakpadRef breakpad = 0;
    NSString *execPath = [[NSBundle mainBundle] resourcePath];
    NSString *contentPath = [execPath stringByDeletingLastPathComponent];
    NSString *resourcesPath = [contentPath stringByAppendingPathComponent:@"Resources/"];

    NSString *inspector = [resourcesPath stringByAppendingPathComponent:@"crash_inspector"];
    NSString *reporter =
               [resourcesPath stringByAppendingPathComponent:@"crash_report_sender.app"];
    reporter = [[NSBundle bundleWithPath:reporter] executablePath];

    NSDictionary *dict = [[NSBundle mainBundle] infoDictionary];
    NSMutableDictionary *config=
        [[dict mutableCopy] autorelease];
    [config setObject:inspector
        forKey:@BREAKPAD_INSPECTOR_LOCATION];
    [config setObject:reporter
        forKey:@BREAKPAD_REPORTER_EXE_LOCATION];
    [config setObject:@"http://"NATIVE_CRASH_COLLECTOR_HOST":"STR(NATIVE_CRASH_COLLECTOR_PORT)""NATIVE_CRASH_COLLECTOR_ENDPOINT
        forKey:@BREAKPAD_URL];
    [config setObject:@NATIVE_VERSION_STR
        forKey:@BREAKPAD_VERSION];

    breakpad = BreakpadCreate(config);

    SetCrashKeyValue(breakpad, @"version", @NATIVE_VERSION_STR);
    SetCrashKeyValue(breakpad, @"build", @NATIVE_BUILD);
    SetCrashKeyValue(breakpad, @"product", "Nidium");

    [pool release];
    return breakpad;
#undef STR
#undef STR_HELPER
}

void SetCrashKeyValue(BreakpadRef breakpad, NSString *key, NSString *value) {
  if (breakpad == NULL) {
    return;
  }

  BreakpadAddUploadParameter(breakpad, key, value);
}

- (void)awakeFromNib {
    breakpad = InitBreakpad();
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    BreakpadRelease(breakpad);
    return NSTerminateNow;
}
#endif

@end
