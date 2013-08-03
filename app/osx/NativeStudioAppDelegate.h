//
//  NativeStudioAppDelegate.h
//  NativeOSX
//
//  Created by Anthony Catel on 8/15/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>

class NativeCocoaUIInterface;

@interface NativeStudioAppDelegate : NSObject <NSApplicationDelegate>
{
    NSArray *position;
    NSString *appfile;
    NativeCocoaUIInterface *UI;
    BOOL isRunning;
}
@property (retain, nonatomic) NSArray *position;
@property (retain, nonatomic) NSString *appfile;

@end
