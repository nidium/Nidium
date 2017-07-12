#import "IOSScrollView.h"
#include "IOSUIInterface.h"
#include "Frontend/InputHandler.h"
#include "Frontend/Context.h"


using Nidium::Interface::IOSUIInterface;
using Nidium::Frontend::InputEvent;
using Nidium::Frontend::InputHandler;

/*
    The size of the scroll view must be large enough so
    it can't be fully scrolled with one quick gesture
*/
#define NDM_SCROLLVIEW_SIZE 16000

@interface PassiveGestureRecognizer : UIGestureRecognizer<UIGestureRecognizerDelegate>

- (instancetype)initWithEventForwardingTarget:(UIResponder*)target;

@end

@implementation PassiveGestureRecognizer {
    UIResponder* eventForwardingTarget;
}

- (instancetype)initWithEventForwardingTarget:(UIResponder*)target {
    self = [super init];

    if (self) {
        self.delegate = self;
        eventForwardingTarget = target;
    }

    return self;
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [eventForwardingTarget touchesBegan:touches withEvent:event];
}

- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [eventForwardingTarget touchesMoved:touches withEvent:event];
}

- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [eventForwardingTarget touchesEnded:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [eventForwardingTarget touchesCancelled:touches withEvent:event];
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer*)gestureRecognizer
shouldRecognizeSimultaneouslyWithGestureRecognizer:
(UIGestureRecognizer*)otherGestureRecognizer {
    return YES;
}

@end

@implementation IOSScrollView {
    BOOL ignoreScrollEvent;
    CGPoint touchPoint;
    IOSUIInterface *interface;
    CGPoint lastOffset;
}

- (id)initWithFrame:(CGRect)frame target:(UIView *)target interface:(IOSUIInterface *)iosInterface
{
    self = [super initWithFrame:frame];

    if (self)
    {
        self.userInteractionEnabled = YES;
        self.delaysContentTouches   = NO;
        self.backgroundColor        = [UIColor clearColor];
        self.scrollsToTop           = NO;
        self.delegate               = self;
        self.bounces                = NO;

        self.showsHorizontalScrollIndicator = NO;
        self.showsVerticalScrollIndicator   = NO;

        [self setContentSize:CGSizeMake(NDM_SCROLLVIEW_SIZE, NDM_SCROLLVIEW_SIZE)];
        [self setDefaultPosition];

        interface   = iosInterface;
        lastOffset  = CGPointZero;
        touchPoint  = CGPointZero;
        ignoreScrollEvent = NO;

        /*
            Add a gesture recognizer that will forward received events to
            SDL (as an UIScrollView consume all the events received)
        */
        [self addGestureRecognizer:[[PassiveGestureRecognizer alloc]
                                    initWithEventForwardingTarget:target]];
    }
    return self;
}

- (void)scrollViewWillBeginDragging:(UIScrollView *)scrollView
{
    lastOffset.x = self.contentOffset.x;
    lastOffset.y = self.contentOffset.y;

    [self sendScrollEvent:InputEvent::kScrollState_start relX:0 relY:0];
}

- (void)scrollViewDidEndDecelerating:(UIScrollView *)scrollView
{
    [self sendScrollEvent:InputEvent::kScrollState_end relX:0 relY:0];
    [self setDefaultPosition];
}


- (void)setDefaultPosition
{
    CGPoint defaultPosition = CGPointMake((NDM_SCROLLVIEW_SIZE - self.bounds.size.width)/2,
                                          (NDM_SCROLLVIEW_SIZE - self.bounds.size.height)/2);

    ignoreScrollEvent = YES;
    [self setContentOffset:defaultPosition];
    ignoreScrollEvent = NO;

    lastOffset = defaultPosition;
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView
{
    if (ignoreScrollEvent) {
        return;
    }

    int x = (int)self.contentOffset.x;
    int y = (int)self.contentOffset.y;

    if (x == lastOffset.x && y == lastOffset.y) {
        /*
            XXX : iOS works with float, but nidium use int. Sending scroll events
            in float may not actually scroll something so we throttle events here.
            The dowside of this mechanism is that on device with a high pixel ratio
            scrolling may not be smooth enough.
        */
        return;
    }

    [self sendScrollEvent:InputEvent::kScrollState_move
                     relX:(lastOffset.x - x) * -1
                     relY:(lastOffset.y - y) * -1];

    lastOffset.x = x;
    lastOffset.y = y;
}


- (void)sendScrollEvent:(InputEvent::ScrollState)state relX:(int)relX relY:(int) relY
{
    InputHandler *inputHandler = interface->m_NidiumCtx->getInputHandler();
    InputEvent ev(InputEvent::kTouchScroll_type,
                  touchPoint.x,
                  touchPoint.y);

    ev.m_data[0] = relX;
    ev.m_data[1] = relY;
    ev.m_data[2] = 0; // velocityX
    ev.m_data[3] = 0; // velocityY
    ev.m_data[4] = state;
    ev.m_data[5] = 0; // consumed

    inputHandler->pushEvent(ev);
}

/*
    Override hitTest method so we can easily which touch event/point start the scroll
*/
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event
{
    [super hitTest:point withEvent:event];

    touchPoint.x = point.x - self.contentOffset.x;
    touchPoint.y = point.y - self.contentOffset.y;

    return self;
}


@end
