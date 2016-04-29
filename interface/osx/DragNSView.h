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

