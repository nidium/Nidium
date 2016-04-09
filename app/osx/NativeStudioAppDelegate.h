/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import <client/mac/Framework/Breakpad.h>

class NativeCocoaUIInterface;

@interface NativeStudioAppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
{
    NSArray *position;
    NSString *appfile;
    NativeCocoaUIInterface *UI;
    BOOL isRunning;
#ifdef NATIVE_ENABLE_BREAKPAD
    BreakpadRef breakpad;
#endif
}

#ifdef NATIVE_ENABLE_BREAKPAD
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
#endif

@property (retain, nonatomic) NSArray *position;
@property (retain, nonatomic) NSString *appfile;

@end

