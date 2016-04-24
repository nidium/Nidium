#ifndef interface_osx_dragnsview_h__
#define interface_osx_dragnsview_h__

#import <Cocoa/Cocoa.h>

namespace Nidium {
namespace Interface {

class NativeJSwindow;

@interface NativeDragNSView : NSView

@property (assign, nonatomic) NativeJSwindow *responder;

@end

} // namespace Nidium
} // namespace Interface

#endif

