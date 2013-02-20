#import "NativeUIConsole.h"


@implementation NativeConsole

@synthesize window, textview;

- (id) init
{
    if (!(self = [super init])) return nil;
    
    self.window = [[NSWindow alloc] initWithContentRect:NSMakeRect(100, 100, 640, 500) styleMask: NSTitledWindowMask backing:NSBackingStoreBuffered defer:NO];
    
    [self.window setFrameAutosaveName:@"nativeConsole"];
    
    NSScrollView *scrollview = [[NSScrollView alloc]
                                
                                initWithFrame:[[self.window contentView] frame]];
    
    NSSize contentSize = [scrollview contentSize];
    
    [scrollview setBorderType:NSNoBorder];
    
    [scrollview setHasVerticalScroller:YES];
    
    [scrollview setHasHorizontalScroller:NO];
    
    [scrollview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    
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
    
    [window setContentView:scrollview];
    
    [window makeKeyAndOrderFront:nil];
    
    [window makeFirstResponder:textview];

    
    [self.window release];
    [self.textview release];
    
    [textview insertText:@"Console ready.\n"];
    
    [textview setFont:[NSFont fontWithName:@"Monaco" size:10]];

    return self;
}

- (void) log:(NSString *)str
{

    [[[textview textStorage] mutableString] appendString: str];
}

- (void) clear
{
    [textview setString:@""];
    [textview insertText:@"Console ready.\n"];
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
    [this->window attachToStdout];
    this->hide();
}

void NativeUICocoaConsole::clear()
{    
    [this->window clear];
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
}

void NativeUICocoaConsole::show()
{
    if (!this->isHidden) {
        return;
    }
    [this->window.window orderFront:nil];
    this->isHidden = false;
}

void NativeUICocoaConsole::log(const char *str)
{
    if (!needFlush) {
        [[[this->window textview] textStorage] beginEditing];
        needFlush = true;
    }
    if (this->isHidden) {
        return;
    }
    NSString *nstr = [NSString stringWithCString:str encoding:NSASCIIStringEncoding];
    [this->window log:nstr];
}

NativeUICocoaConsole::~NativeUICocoaConsole()
{
    [this->window release];
}
