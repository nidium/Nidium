#include "NativeUIInterface.h"
#include "NativeUIConsole.h"
#import <Cocoa/Cocoa.h>

class NativeUICocoaConsole;

@class NativeDragNSView;

class NativeCocoaUIInterface : public NativeUIInterface
{
    public:
        NativeCocoaUIInterface();
        void setWindowTitle(const char *);
        const char *getWindowTitle() const;
        void setCursor(CURSOR_TYPE);
        void runLoop();
        void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void setWindowControlsOffset(double x, double y);
        void initControls();
        void setClipboardText(const char *text);
        char *getClipboardText();
        void stopApplication();
        void setWindowSize(int w, int h);
        void restartApplication(const char *path=NULL);
        bool runApplication(const char *path);
        void openFileDialog(const char *files[],
            void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg);
        const char *getCacheDirectory() const;
        NativeUICocoaConsole *getConsole(bool create=false, bool *created = NULL);

        void alert(const char *message);

        struct {
            CGRect closeFrame;
            CGRect zoomFrame;
            CGRect minFrame;
        } controls;
        
        struct {
            char *buf;
            size_t len;
            size_t offset;
        } mainjs;

        void onNMLLoaded();

        void log(const char *buf);
        void logf(const char *format, ...);
        void vlog(const char *format, va_list ap);

        virtual bool makeMainGLCurrent();
        virtual bool makeGLCurrent(SDL_GLContext ctx);
        virtual SDL_GLContext getCurrentGLContext();
        
    private:
        bool initContext();
        bool createWindow(int width, int height);

        /*
            We need to patch SDLView in order to add NSView(drawRect:) method.
            This is done, so that we can avoid flickering during resize by
            redrawing the OpenGL scene when it's dirty.
        */
        void patchSDLView(NSView *sdlview);

        NativeUICocoaConsole *console;
        NativeDragNSView *dragNSView;
};
