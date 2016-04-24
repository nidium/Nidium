#import "StudioAppDelegate.h"
#import <pthread.h>

#import <dispatch/dispatch.h>
#include <crt_externs.h>

#import "CocoaUIInterface.h"
#import "System.h"

#define NIDIUM_DISPATCH_MAINTHREAD 0

Nidium::Interface::NativeSystemInterface *NativeSystemInterface::_interface = new Nidium::Interface::NativeSystem();
Nidium::Interface::NativeUIInterface *__NativeUI;


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
        LSSetDefaultHandlerForURLScheme(CFSTR("native"), bundleID);
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

- (void) refreshApp
{
    self->UI->refreshApplication();
    NativeUICocoaConsole *console = self->UI->getConsole(false, NULL);
    if (console) {
        console->clear();
    }
}

- (void) openConsole
{
    bool created;
    NativeUICocoaConsole *console = self->UI->getConsole(true, &created);

    if (!created) {
        if (console->hidden()) {
            console->show();
        } else {
            console->hide();
        }
    }
}

- (void) stopApplication
{
    self->UI->stopApplication();
}

- (void)createMenu
{
    NSMenu *mainmenu = [NSApp mainMenu];

    NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:@"_Application" action:NULL keyEquivalent:@""];
    NSMenu *submenu = [[NSMenu alloc] initWithTitle:@"Application"];

    [submenu addItemWithTitle:@"Refresh" action:@selector(refreshApp) keyEquivalent:@"r"];
    [submenu addItemWithTitle:@"Toggle console" action:@selector(openConsole) keyEquivalent:@"d"];
    [submenu addItemWithTitle:@"Stop" action:@selector(stopApplication) keyEquivalent:@"u"];
    [item setSubmenu:submenu];
    [mainmenu addItem:item];

/*
    NSMenuItem *HelpMenu = [[NSMenuItem alloc] initWithTitle:@"_Doc" action:NULL keyEquivalent:@""];
    NSMenu *Helpsubmenu = [[NSMenu alloc] initWithTitle:@"Documentation"];

    [HelpMenu setSubmenu:Helpsubmenu];

    [mainmenu addItem:HelpMenu];

    NSSearchField *searchField = [[NSSearchField alloc] init];
    NSRect cellFrame = [searchField frame];
    NSMenuItem *search = [[NSMenuItem alloc] initWithTitle:@"Search" action:NULL keyEquivalent:@""];
    [search setView:searchField];

    searchField.frame = NSMakeRect(50, 0, 200, 40);

    [Helpsubmenu addItem:search];
*/
    /*
NSMenuItem *testItem = [[[NSMenuItem alloc] initWithTitle:@"Testing!" action:nil keyEquivalent:@""] autorelease];
NSMenu *subMenu = [[[NSMenu alloc] initWithTitle:@"Testing!"] autorelease];
[subMenu addItemWithTitle:@"Another test." action:nil keyEquivalent:@""];
[testItem setSubmenu:subMenu];
[menubar addItem:testItem];
    */
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification{
    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    [self createMenu];
    [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];

    NSUserDefaults *args = [NSUserDefaults standardUserDefaults];
    BOOL noNML = ([args objectForKey:@"no-nml"] != nil);
    int appWidth = 0, appHeight = 0;

    if (noNML) {
        appWidth  = [args integerForKey:@"width"];
        appHeight = [args integerForKey:@"height"];
    }

    //[self setupWorkingDirectory:YES];
    _ape_seed = time(NULL) ^ (getpid() << 16);
    //NativeConsole *console = [[NativeConsole alloc] init];
    //[console attachToStdout];
    NativeCocoaUIInterface *nUI = new NativeCocoaUIInterface;
    __NativeUI = nUI;
    nUI->setArguments(*_NSGetArgc(), *_NSGetArgv());

    self->UI = nUI;
    self->isRunning = YES;
#if NIDIUM_DISPATCH_MAINTHREAD
    dispatch_async(dispatch_get_main_queue(), ^{
#endif
        const char *filename;

        if (self.appfile == nil) {
            filename = "private://default.nml";
        } else {
            filename = [self.appfile UTF8String];
        }
        if (!noNML) {
            if (!nUI->runApplication(filename)) {
                [[NSApplication sharedApplication] terminate:nil];
                return;
            }
        } else {
            if (!nUI->runJSWithoutNML(filename, appWidth, appHeight)) {
                [[NSApplication sharedApplication] terminate:nil];
                return;
            }
        }
        nUI->runLoop();
#if NIDIUM_DISPATCH_MAINTHREAD
    });
#endif
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    NSLog(@"[Cocoa] application:openFile: <%@>", filename);

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
    SetCrashKeyValue(breakpad, @"product", @"Nidium");

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

    NSLog(@"quit????");
    return NSTerminateNow;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed
{
    return NO;
}

#endif

@end
