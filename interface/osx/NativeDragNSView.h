/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#import <Cocoa/Cocoa.h>

class NativeJSwindow;

@interface NativeDragNSView : NSView

@property (assign, nonatomic) NativeJSwindow *responder;

@end

