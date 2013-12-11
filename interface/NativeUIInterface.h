#ifndef nativeuiinterface_h__
#define nativeuiinterface_h__

#include <stdint.h>
#include <stdio.h>

class NativeContext;
class NativeNML;

typedef void *SDL_GLContext;

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
            HIDDEN,
            NOCHANGE
        } currentCursor;
        NativeContext *NativeCtx;
        NativeNML *nml;
        struct SDL_Window *win;
        struct _ape_global *gnet;

        virtual void stopApplication()=0;
        virtual void restartApplication(const char *path=NULL)=0;
        virtual bool runApplication(const char *path)=0;
        virtual void setWindowTitle(const char *)=0;
        virtual const char *getWindowTitle() const=0;
        virtual void setCursor(CURSOR_TYPE)=0;
        virtual void runLoop()=0;
        virtual void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a){};
        virtual void setWindowControlsOffset(double x, double y){};
        virtual void setClipboardText(const char *text)=0;
        virtual char *getClipboardText()=0;
        virtual void openFileDialog(const char *files[],
            void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg)=0;
        virtual const char *getCacheDirectory() const=0;
        virtual void setWindowSize(int w, int h)=0;

        virtual void log(const char *buf)=0;
        virtual void logf(const char *format, ...)=0;
        virtual void vlog(const char *buf, va_list ap)=0;

        virtual void alert(const char *message)=0;
        
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
        virtual NativeUIConsole *getConsole(bool create=false, bool *created=NULL)=0;

        bool makeMainGLCurrent();
        bool makeGLCurrent(SDL_GLContext ctx);

        SDL_GLContext getGLContext() {
            return m_mainGLCtx;
        }

        SDL_GLContext getCurrentGLContext();
        SDL_GLContext createSharedContext();
        void deleteGLContext(SDL_GLContext ctx);

    protected:
        int width;
        int height;
        char *filePath;
        bool initialized;
        NativeUIConsole *console;
        SDL_GLContext m_mainGLCtx;
};

#endif