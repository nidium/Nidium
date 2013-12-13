#import "NativeDragNSView.h"
#import <NativeJSWindow.h> 

@implementation NativeDragNSView

@synthesize responder = _responder;

- (id) initWithFrame:(NSRect)rect
{
    if (!(self = [super initWithFrame:rect])) return nil;
    
    [self registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];

    self.responder = nil;

    return self;
}

- (NSDragOperation)draggingOperation:(id < NSDraggingInfo >)sender update:(BOOL)update {
    if (self.responder == nil) {
        return NSDragOperationNone;
    }
    NSPasteboard *pboard = [sender draggingPasteboard];
    NSArray *filenames = [pboard propertyListForType:NSFilenamesPboardType];

    if (!update) {
        char const **files = (char const **)malloc(sizeof(char *) * [filenames count]);
        int idx = 0;
        for (NSString *file in filenames) {
            files[idx] = [file UTF8String];
            idx++;
        }

        if (!self.responder->dragBegin([sender draggingLocation].x, [sender draggingLocation].y, files, [filenames count])) {

            free(files);
            return NSDragOperationNone;
        }

        free(files);
    } else {
        if (!self.responder->dragUpdate([sender draggingLocation].x, [sender draggingLocation].y)) {
            return NSDragOperationNone;
        }
    }

    return NSDragOperationCopy;
}

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender {
    return [self draggingOperation:sender update:NO];
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
    return [self draggingOperation:sender update:YES];
}

- (void)draggingEnded:(id < NSDraggingInfo >)sender {
    if (self.responder == nil) {
        return;
    }

    self.responder->dragEnd();
}

- (void)draggingExited:(id < NSDraggingInfo >)sender {
    if (self.responder == nil) {
        return;
    }

    self.responder->dragLeave();    
}

- (BOOL)wantsPeriodicDraggingUpdates {
    return NO;
}

- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender {
    if (self.responder == nil) {
        return NO;
    }

    return self.responder->dragDroped([sender draggingLocation].x, [sender draggingLocation].y);
}

@end