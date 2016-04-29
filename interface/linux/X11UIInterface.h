#ifndef interface_linux_x11uiinterface_h__
#define interface_linux_x11uiinterface_h__

#include "UIInterface.h"

namespace Nidium {
namespace Interface {

// {{{ UIX11Console
class UIX11Console : public UIInterface::UIConsole
{
    public:
        UIX11Console() {};
        ~UIX11Console() {};
        void log(const char *str);
        void show() {};
        void hide() {};
        void clear() {};
        void flush() {};
        bool hidden() { return true; };
};
// }}}

// {{{ UIX11Interface
class UIX11Interface : public UIInterface
{
    public:
        UIX11Interface();
        void runLoop();
        void quitApplication();
        void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {};
        void setWindowControlsOffset(double x, double y) {};
        void openFileDialog(const char *files[],
            void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg, int flags = 0);

        void enableSysTray();

        void initControls() {};
        UIX11Console *getConsole(bool create=false, bool *created=NULL) {
            return this->console;
        }

        void vlog(const char *buf, va_list ap);
        void log(const char *buf);
        void logf(const char *format, ...);
        void logclear() {};
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
    protected:
        void renderSystemTray();
        void setSystemCursor(CURSOR_TYPE cursor);
        void hitRefresh();
        void onWindowCreated();
    private:
        UIX11Console *console;
};
// }}}

} // namespace Interface
} // namespace Nidium

#endif

