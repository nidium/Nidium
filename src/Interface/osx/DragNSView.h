/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_osx_dragnsview_h__
#define interface_osx_dragnsview_h__

#import <Cocoa/Cocoa.h>

namespace Nidium {
namespace Binding {
class JSWindow;
}
}

@interface DragNSView : NSView

@property (assign, nonatomic) Nidium::Binding::JSWindow *responder;

@end

#endif
