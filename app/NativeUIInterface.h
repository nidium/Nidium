#include <stdint.h>

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

        int getWidth() const { return this->width; }
        int getHeight() const { return this->height; }
    protected:
        int width;
        int height;
};
