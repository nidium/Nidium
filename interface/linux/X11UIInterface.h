#ifndef interface_linux_x11uiinterface_h__
#define interface_linux_x11uiinterface_h__

#include "UIInterface.h"

namespace Nidium {
namespace Interface {

// {{{ NativeUIX11Console
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
        bool hidden();
};
// }}}

// {{{ NativeX11UIInterface
class NativeX11UIInterface : public NativeUIInterface
{
    public:
        NativeX11UIInterface();
        void runLoop();
        void quitApplication();
        void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void setWindowControlsOffset(double x, double y);
        void openFileDialog(const char *files[],
            void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg, int flags = 0);

        void initControls();
        NativeUIX11Console *getConsole(bool create=false, bool *created=NULL) {
            return this->console;
        }

        void stopApplication();

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
        void setSystemCursor(CURSOR_TYPE cursor);
        void hitRefresh();
        void onWindowCreated();
    private:
        NativeUIX11Console *console;
};
// }}}

} // namespace Nidium
} // namespace Interface

#endif

