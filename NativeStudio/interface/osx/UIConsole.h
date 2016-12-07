/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_osx_uiconsole_h__
#define interface_osx_uiconsole_h__

#import "UIInterface.h"

#import <Cocoa/Cocoa.h>

namespace Nidium {
namespace Interface {

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


class UICocoaConsole : public UIInterface::UIConsole
{
    public:
        UICocoaConsole();
        ~UICocoaConsole();
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

} // namespace Interface
} // namespace Nidium

#endif

