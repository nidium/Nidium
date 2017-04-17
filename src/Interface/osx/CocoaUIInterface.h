/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_osx_cocoauiinterface_h__
#define interface_osx_cocoauiinterface_h__

#include "UIInterface.h"

#import <Cocoa/Cocoa.h>

#include "UIConsole.h"

@class DragNSView;
@class UICocoaInterfaceWrapper;

namespace Nidium {
namespace Interface {

class UICocoaConsole;

class UICocoaInterface : public UIInterface
{
public:
    UICocoaInterface();

    void quitApplication();
    void runLoop();
    void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void setWindowControlsOffset(double x, double y);
    void initControls();
    void stopApplication();
    void setWindowFrame(int x, int y, int w, int h);
    void openFileDialog(const char *files[],
                        void (*cb)(void *nof, const char *lst[], uint32_t len),
                        void *arg,
                        int flags = 0);
    UICocoaConsole *getConsole(bool create = false, bool *created = NULL);
    void enableSysTray(const void *imgData = NULL, size_t imageDataSize = 0);
    void disableSysTray();

    void hideWindow();
    void showWindow();
    void quit();

    void setGLContextAttribute() override;

    struct
    {
        CGRect closeFrame;
        CGRect zoomFrame;
        CGRect minFrame;
    } controls;

#if 0
        virtual bool makeMainGLCurrent();
        virtual bool makeGLCurrent(SDL_GLContext ctx);
        virtual SDL_GLContext getCurrentGLContext();
#endif
protected:
    void setSystemCursor(CURSOR_TYPE cursor);
    void setupWindow();
    void onWindowCreated();
    void hitRefresh() override;

private:
    void renderSystemTray();

    /*
        We need to patch SDLView in order to add NSView(drawRect:) method.
        This is done, so that we can avoid flickering during resize by
        redrawing the OpenGL scene when it's dirty.
    */
    void patchSDLView(NSView *sdlview);

    UICocoaConsole *m_Console = nullptr;
    DragNSView *m_DragNSView;
    NSStatusItem *m_StatusItem;
    UICocoaInterfaceWrapper *m_Wrapper;
};

} // namespace Interface
} // namespace Nidium

#endif
