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
        bool runApplication(const char *path);
        NativeUICocoaConsole *getConsole() const {
            return this->console;
        }
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
    private:
        NativeUICocoaConsole *console;
};
