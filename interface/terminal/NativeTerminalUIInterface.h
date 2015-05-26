#include "NativeUIInterface.h"

class NativeUITerminalConsole : public NativeUIInterface::NativeUIConsole
{
    public:
        NativeUITerminalConsole();
        ~NativeUITerminalConsole();
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
            return this->console;
        }

        struct {
            char *buf;
            size_t len;
            size_t offset;
        } mainjs;
    private:
        NativeUITerminalConsole *console;
};

