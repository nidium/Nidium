#ifndef interface_osx_cocoauiinterface_h__
#define interface_osx_cocoauiinterface_h__

#include "UIInterface.h"

#import <Cocoa/Cocoa.h>

#include "UIConsole.h"

namespace Nidium {
namespace Interface {

class NativeUICocoaConsole;

@class NativeDragNSView;
@class NativeCocoaUIInterfaceWrapper;

class NativeCocoaUIInterface : public NativeUIInterface
{
    public:
        NativeCocoaUIInterface();

        void quitApplication();
        void runLoop();
        void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void setWindowControlsOffset(double x, double y);
        void initControls();
        void setClipboardText(const char *text);
        char *getClipboardText();
        void stopApplication();
        void setWindowSize(int w, int h);
        void setWindowFrame(int x, int y, int w, int h);
        void restartApplication(const char *path=NULL);
        bool runApplication(const char *path);
        bool runJSWithoutNML(const char *path, int width = 800, int height = 600);
        void openFileDialog(const char *files[],
            void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg, int flags = 0);
        const char *getCacheDirectory() const;
        NativeUICocoaConsole *getConsole(bool create=false, bool *created = NULL);
        void enableSysTray(const void *imgData = NULL, size_t imageDataSize = 0);
        void disableSysTray();

        void hideWindow();
        void showWindow();
        void alert(const char *message);
        void quit();

        struct {
            CGRect closeFrame;
            CGRect zoomFrame;
            CGRect minFrame;
        } controls;

        struct {
            char *buf;
            size_t len;
            size_t offset;
        } m_Mainjs;

        void onNMLLoaded();

        void log(const char *buf);
        void logf(const char *format, ...);
        void vlog(const char *format, va_list ap);
        void logclear();
#if 0
        virtual bool makeMainGLCurrent();
        virtual bool makeGLCurrent(SDL_GLContext ctx);
        virtual SDL_GLContext getCurrentGLContext();
#endif
    protected:
        void setSystemCursor(CURSOR_TYPE cursor);
    private:
        void renderSystemTray();
        bool initContext();
        bool createWindow(int width, int height);

        /*
            We need to patch SDLView in order to add NSView(drawRect:) method.
            This is done, so that we can avoid flickering during resize by
            redrawing the OpenGL scene when it's dirty.
        */
        void patchSDLView(NSView *sdlview);

        NativeUICocoaConsole *m_Console;
        NativeDragNSView *m_DragNSView;
        NSStatusItem *m_StatusItem;
        NativeCocoaUIInterfaceWrapper *m_Wrapper;
};

} // namespace Nidium
} // namespace Interface

#endif

