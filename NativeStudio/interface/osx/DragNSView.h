/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_osx_dragnsview_h__
#define interface_osx_dragnsview_h__

#import <Cocoa/Cocoa.h>

namespace Nidium {
namespace Interface {

class JSWindow;

@interface DragNSView : NSView

@property (assign, nonatomic) JSWindow *responder;

@end

} // namespace Interface
} // namespace Nidium

#endif

