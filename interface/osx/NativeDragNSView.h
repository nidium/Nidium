#import <Cocoa/Cocoa.h>

class NativeJSwindow;

@interface NativeDragNSView : NSView

@property (assign, nonatomic) NativeJSwindow *responder;

@end