/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#import "AppDelegate.h"
#import <pthread.h>

#import <dispatch/dispatch.h>
#include <crt_externs.h>

#import "CocoaUIInterface.h"
#import "System.h"

#define NIDIUM_DISPATCH_MAINTHREAD 0

unsigned long _ape_seed;

namespace Nidium {
    namespace Interface {
        SystemInterface *SystemInterface::_interface = new System();
        UIInterface *__NidiumUI;
    }
}

@implementation AppDelegate

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
        LSSetDefaultHandlerForURLScheme(CFSTR("nidium"), bundleID);
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

    NSString *f = [[[NSString alloc] initWithFormat:@"http%@", [filename substringFromIndex:sizeof("nidium")-1]] autorelease];

    self.appfile = f;

    if (self->isRunning) {
        self->UI->restartApplication([f UTF8String]);
    }
}

- (void) refreshApp
{
    self->UI->refreshApplication(true);
}

- (void) openConsole
{
    bool created;
    Nidium::Interface::UICocoaConsole *console = self->UI->getConsole(true, &created);

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
#if 0
    BOOL noNML = ([args objectForKey:@"no-nml"] != nil);
#endif
    int appWidth = 0, appHeight = 0;
#if 0
    if (noNML) {
        appWidth  = [args integerForKey:@"width"];
        appHeight = [args integerForKey:@"height"];
    }
#endif
    //[self setupWorkingDirectory:YES];
    _ape_seed = time(NULL) ^ (getpid() << 16);
    //Console *console = [[Console alloc] init];
    //[console attachToStdout];
    Nidium::Interface::UICocoaInterface *nUI = new Nidium::Interface::UICocoaInterface;
    Nidium::Interface::__NidiumUI = nUI;
    nUI->setArguments(*_NSGetArgc(), *_NSGetArgv());

    self->UI = nUI;
    self->isRunning = YES;
#if NIDIUM_DISPATCH_MAINTHREAD
    dispatch_async(dispatch_get_main_queue(), ^{
#endif
        const char *filename;

        if (self.appfile == nil) {
            filename = "embed://default.nml";
        } else {
            filename = [self.appfile UTF8String];
        }

        if (!nUI->runApplication(filename)) {
            [[NSApplication sharedApplication] terminate:nil];
            return;
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

#ifdef NIDIUM_ENABLE_CRASHREPORTER
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
    [config setObject:@"http://"NIDIUM_CRASH_COLLECTOR_HOST":"STR(NIDIUM_CRASH_COLLECTOR_PORT)""NIDIUM_CRASH_COLLECTOR_ENDPOINT
        forKey:@BREAKPAD_URL];
    [config setObject:@NIDIUM_VERSION_STR
        forKey:@BREAKPAD_VERSION];

    breakpad = BreakpadCreate(config);

    SetCrashKeyValue(breakpad, @"version", @NIDIUM_VERSION_STR);
    SetCrashKeyValue(breakpad, @"build", @NIDIUM_BUILD);
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
