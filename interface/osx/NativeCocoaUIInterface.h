#include "NativeUIInterface.h"
#include "NativeUIConsole.h"
#import <Cocoa/Cocoa.h>

class NativeUICocoaConsole;

class NativeCocoaUIInterface : public NativeUIInterface
{
    public:
        NativeCocoaUIInterface();
        bool createWindow(int width, int height);
        void setWindowTitle(const char *);
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
    private:
        NativeUICocoaConsole *console;
};
