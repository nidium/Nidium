#import "NativeUIInterface.h"
#import <Cocoa/Cocoa.h>


@interface NativeConsole : NSObject
{
    NSWindow *window;
    NSTextView *textview;
}

- (void) log:(NSString *)str;
- (void) attachToStdout;
- (void) clear;

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
    private:
        NativeConsole *window;
        bool isHidden;
        bool needFlush;
};
