#import "NativeUIInterface.h"

#import <Cocoa/Cocoa.h>

@interface NativeConsole : NSObject
{
    NSWindow *m_Window;
    NSTextView *textview;
    BOOL m_IsHidden;
}

- (void) log:(NSString *)str;
- (void) attachToStdout;
- (void) clear;

@property (assign, nonatomic) BOOL m_IsHidden;
@property (retain, nonatomic) NSWindow *m_Window;
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
        bool m_IsHidden;
    private:
        NativeConsole *m_Window;
        bool m_NeedFlush;
};

