#include <stdint.h>
#include <stdio.h>

class NativeJS;

class NativeUIInterface
{
    public:
        enum CURSOR_TYPE {
            ARROW,
            BEAM,
            CROSS,
            CLOSEDHAND,
            OPENHAND,
            POINTING,
            RESIZELEFT,
            RESIZERIGHT,
            RESIZELEFTRIGHT,
            RESIZEUP,
            RESIZEDOWN,
            RESIZEUPDOWN,
            NOCHANGE
        } currentCursor;
        NativeJS *NJS;
        struct SDL_Window *win;
        struct _ape_global *gnet;
        virtual bool runApplication(const char *path)=0;
        virtual bool createWindow(int width, int height)=0;
        virtual void setWindowTitle(const char *)=0;
        virtual void setCursor(CURSOR_TYPE)=0;
        virtual void runLoop()=0;
        virtual void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a){};
        virtual void setWindowControlsOffset(double x, double y){};
        virtual void openFileDialog(const char const *files[],
            void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg)=0;
        
        int getWidth() const { return this->width; }
        int getHeight() const { return this->height; }
        class NativeUIConsole
        {
            public:
            virtual void log(const char *str)=0;
            virtual void show()=0;
            virtual void hide()=0;
            virtual void clear()=0;
        };
        virtual NativeUIConsole *getConsole() const =0;
    protected:
        int width;
        int height;
        NativeUIConsole *console;
};
