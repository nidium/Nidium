/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#import <client/mac/Framework/Breakpad.h>

namespace Nidium {
namespace App {

class UICocoaUIIInterface;

@interface NativeStudioAppDelegate : NSObject <NSApplicationDelegate, NSUserNotificationCenterDelegate>
{
    NSArray *position;
    NSString *appfile;
    UICocoaUIIInterface *UI;
    BOOL isRunning;
#ifdef NIDIUM_ENABLE_CRASHREPORTER
    BreakpadRef breakpad;
#endif
}

#ifdef NIDIUM_ENABLE_CRASHREPORTER
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
#endif

@property (retain, nonatomic) NSArray *position;
@property (retain, nonatomic) NSString *appfile;

@end

} // namespace App
} // namespace Nidium

