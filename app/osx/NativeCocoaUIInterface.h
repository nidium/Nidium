#include "NativeUIInterface.h"
#import <Cocoa/Cocoa.h>

class NativeCocoaUIInterface : public NativeUIInterface
{
    public:
        NativeCocoaUIInterface();
        void createWindow();
        void setWindowTitle(const char *);
        void setCursor(CURSOR_TYPE);
        void runLoop();
        void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void setWindowControlsOffset(double x, double y);
        void initControls();

        struct {
            CGRect closeFrame;
            CGRect zoomFrame;
            CGRect minFrame;
        } controls;
        
};
