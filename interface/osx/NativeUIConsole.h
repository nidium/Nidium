#import "NativeUIInterface.h"
#import <Cocoa/Cocoa.h>


@interface NativeConsole : NSObject
{
    NSWindow *window;
    NSTextView *textview;
    BOOL isHidden;
}

- (void) log:(NSString *)str;
- (void) attachToStdout;
- (void) clear;

@property (assign, nonatomic) BOOL isHidden;
@property (retain, nonatomic) NSWindow *window;
@property (retain, nonatomic) NSTextView *textview;
@end


class NativeUICocoaConsole : public NativeUIInterface::NativeUIConsole
{
    public:
        NativeUICocoaConsole();
        ~NativeUICocoaConsole();
        void log(const char *str);
        void show();
        void hide();
        void clear();
        void flush();
        bool hidden();
        bool isHidden;
    private:
        NativeConsole *window;
        bool needFlush;
};
