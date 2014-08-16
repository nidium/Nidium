#import "NativeUIConsole.h"


@implementation NativeConsole

@synthesize window, textview, isHidden;

- (id) init
{
    if (!(self = [super init])) return nil;

    self.isHidden = NO;
    
    self.window = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, 700, 480) styleMask: NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];

    [self.window setFrameAutosaveName:@"nativeConsole"];
    [self.window setTitle:@"nidium console"];
    CGRect frame = [[self.window contentView] frame];
    NSButton *btn = [[NSButton alloc] initWithFrame:NSMakeRect(10, 3, 100, 25)];
    [btn setButtonType:NSMomentaryLightButton]; //Set what type button You want
    [btn setBezelStyle:NSRegularSquareBezelStyle]; //Set what style You want
    btn.title = @"Clear";
    [btn setTarget:self];
    [btn setAction:@selector(clearPressed)];

    frame.origin.y = 32;
    frame.size.height -= 32;

    NSView *mview = [[NSView alloc] initWithFrame:frame];
    [mview setWantsLayer:YES];

    NSScrollView *scrollview = [[NSScrollView alloc]
                                
                                initWithFrame:frame];

    [mview addSubview:scrollview];
    
    NSSize contentSize = [scrollview contentSize];

    [mview addSubview:btn];
    
    [scrollview setBorderType:NSNoBorder];
    
    [scrollview setHasVerticalScroller:YES];
    
    [scrollview setHasHorizontalScroller:NO];
    
    [scrollview setAutoresizingMask:NSViewWidthSizable];
    
    self.textview = [[NSTextView alloc] initWithFrame:NSMakeRect(0,
                                                                 0,  
                                                                 contentSize.width,
                                                                 contentSize.height)];
    
    [textview setMinSize:NSMakeSize(0.0, contentSize.height)];
    
    [textview setMaxSize:NSMakeSize(FLT_MAX, FLT_MAX)];
    
    [textview setVerticallyResizable:YES];
    
    [textview setHorizontallyResizable:NO];
    [[textview layoutManager] setBackgroundLayoutEnabled:YES];
    [[textview layoutManager] setAllowsNonContiguousLayout:YES];
    
    [textview setAutoresizingMask:NSViewWidthSizable];
 
    [[textview textContainer] setContainerSize:NSMakeSize(contentSize.width, FLT_MAX)];
    
    [[textview textContainer] setWidthTracksTextView:YES];    

    [scrollview setDocumentView:textview];
    
    [window setContentView:mview];
    
    [window makeKeyAndOrderFront:nil];
    
    [window makeFirstResponder:textview];

    [self.window setContentBorderThickness:32.0 forEdge:NSMinYEdge];
    [self.window release];
    [self.textview release];
    
    [textview insertText:@"Console ready.\n"];
    [textview setBackgroundColor:[NSColor blackColor]];
    [textview setTextColor:[NSColor greenColor]];
    [textview setFont:[NSFont fontWithName:@"Monaco" size:10]];

    return self;
}

-(void)clearPressed {
    [self clear];
}

- (void) log:(NSString *)str
{
    if (!self.isHidden) {
        @try {
            [[[textview textStorage] mutableString] appendString: str];
        } @catch(NSException *e) {}
    }
}

- (void) clear
{
    [textview setString:@""];
    @try {
        [textview insertText:@"Console ready.\n"];
    } @catch(NSException *e){}
    
    [textview setFont:[NSFont fontWithName:@"Monaco" size:10]];
}

- (void) attachToStdout
{
    NSPipe *pipe = [NSPipe pipe];
    NSFileHandle *pipeReadHandle = [pipe fileHandleForReading];
    
    dup2([[pipe fileHandleForWriting] fileDescriptor], fileno(stdout));
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleNotification:)
                                                 name:NSFileHandleReadCompletionNotification
                                               object:pipeReadHandle];
    [pipeReadHandle readInBackgroundAndNotify];

}

- (void) handleNotification:(NSNotification *)notification
{
    NSFileHandle *pipeReadHandle = notification.object;
    [pipeReadHandle readInBackgroundAndNotify];
    NSString *str = [[NSString alloc] initWithData: [[notification userInfo] objectForKey: NSFileHandleNotificationDataItem] encoding: NSASCIIStringEncoding];
    
    [self log:str];

}

@end

NativeUICocoaConsole::NativeUICocoaConsole()
{
    this->window = [[NativeConsole alloc] init];
    this->needFlush = false;
    //[this->window attachToStdout];
    this->show();
    //this->hide();
}

void NativeUICocoaConsole::clear()
{   
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            this->flush();
            [this->window clear];
        });        
    } else {
        this->flush();
        [this->window clear];
    }

}

void NativeUICocoaConsole::flush()
{
    if (needFlush) {
        [[[this->window textview] textStorage] endEditing];
        [[this->window textview] scrollRangeToVisible: NSMakeRange ([[this->window.textview string] length], 0)];
        needFlush = false;
    }
}

void NativeUICocoaConsole::hide()
{
    if (this->isHidden) {
        return;
    }
    [this->window.window orderOut:nil];
    this->isHidden = true;
    [this->window setIsHidden:YES];
}

bool NativeUICocoaConsole::hidden()
{
    return this->isHidden;
}

void NativeUICocoaConsole::show()
{
    if (!this->isHidden) {
        return;
    }
    [this->window.window orderFront:nil];
    this->isHidden = false;
    [this->window setIsHidden:NO];
}

void NativeUICocoaConsole::log(const char *str)
{
    char *copy_str = strdup(str);
    typedef void(^_closure)();

    _closure func = ^{
        if (!needFlush) {
            [[[this->window textview] textStorage] beginEditing];
            needFlush = true;
        }
        if (this->isHidden) {
            return;
        }
        NSString *nstr = [NSString stringWithCString:copy_str encoding:NSUTF8StringEncoding];
        [this->window log:nstr];

        free(copy_str);
    };

    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            func();
        });
    } else {
        func();
    }
}

NativeUICocoaConsole::~NativeUICocoaConsole()
{
    [this->window release];
}
