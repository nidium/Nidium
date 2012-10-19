//
//  NativeConsole.h
//  NativeStudio
//
//  Created by Anthony Catel on 8/1/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

@interface NativeConsole : NSObject
{
    NSWindow *window;
    NSTextView *textview;
}

- (void) log:(NSString *)str;
- (void) attachToStdout;
- (void) clear;

@property (retain, nonatomic) NSWindow *window;
@property (retain, nonatomic) NSTextView *textview;
@end
