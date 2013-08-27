#include "NativeUIInterface.h"
//#include "NativeUIConsole.h"
//#import <X11/X11.h>

class NativeUIX11Console : public NativeUIInterface::NativeUIConsole
{
    public:
        NativeUIX11Console();
        ~NativeUIX11Console();
        void log(const char *str);
        void show();
        void hide();
        void clear();
        void flush();
    private:
//        Window *window;
        bool isHidden;
        bool needFlush;

};

class NativeX11UIInterface : public NativeUIInterface
{
    public:
        NativeX11UIInterface();
        bool createWindow(int width, int height);
        void setWindowTitle(const char *);
        void setCursor(CURSOR_TYPE);
        void runLoop();
        void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void setWindowControlsOffset(double x, double y);
        void openFileDialog(const char *files[],
            void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg);
        const char *getCacheDirectory() const {
            return "/tmp/";
        };
        void setClipboardText(const char *text);
        char *getClipboardText();
        void initControls();
        bool runApplication(const char *path);
        NativeUIX11Console *getConsole(bool create=false, bool *created=NULL) {
            return this->console;
        }
        /*
        struct {
            CGRect closeFrame;
            CGRect zoomFrame;
            CGRect minFrame;
        } controls;
        */
        
        struct {
            char *buf;
            size_t len;
            size_t offset;
        } mainjs;
    private:
        NativeUIX11Console *console;
};

