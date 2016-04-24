#ifndef interface_terminal_uiinterface_h__
#define interface_terminal_uiinterface_h__

#include "UIInterface.h"

namespace Nidium {
namespace Interface {

class NativeUITerminalConsole : public NativeUIInterface::NativeUIConsole
{
    public:
        NativeUITerminalConsole();
        virtual ~NativeUITerminalConsole();
        void log(const char *str);
        void show();
        void hide();
        bool hidden();
        void clear();
        void flush();
    private:
//        Window *m_Window;
        bool m_IsHidden;
        bool m_NeedFlush;

};

class NativeTerminalUIInterface : public NativeUIInterface
{
    public:
        NativeTerminalUIInterface();
        void setWindowTitle(const char *);
        void setCursor(CURSOR_TYPE);
        void runLoop();
        void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void setWindowControlsOffset(double x, double y);
        void openFileDialog(const char const *files[],
            void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg);
        const char *getCacheDirectory() const {
            return "/tmp/";
        };
        void initControls();
        bool runApplication(const char *path);
        NativeUITerminalConsole *getConsole() const {
            return this->m_Console;
        }

        struct {
            char *buf;
            size_t len;
            size_t offset;
        } m_Mainjs;
    private:
        NativeUITerminalConsole *m_Console;
};

} // namespace Nidium
} // namespace Interface

#endif

