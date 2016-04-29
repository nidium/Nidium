//
//  NativeStudioAppDelegate.h
//  NativeOSX
//
//  Created by Anthony Catel on 8/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

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

