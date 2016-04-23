#ifndef nidium_uiinterface_h__
#define nidium_uiinterface_h__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NATIVE_WINDOWPOS_UNDEFINED_MASK   0xFFFFFFF0
#define NATIVE_WINDOWPOS_CENTER_MASK   0xFFFFFFF1

namespace Nidium {
    namespace NML {
        class NativeContext;
        class NativeNML;
    }
}

class NativeUIInterface;

typedef void *SDL_GLContext;

class NativeSystemMenuItem {
public:
    NativeSystemMenuItem(char *title = NULL, char *id = NULL) :
        m_Next(NULL), m_Id(NULL), m_Title(NULL), m_Enabled(false)
    {
        this->id(id);
        this->title(title);
    }

    ~NativeSystemMenuItem() {
        free(m_Title);
        free(m_Id);
    };
    NativeSystemMenuItem *m_Next;

    bool enabled(bool val) {
        m_Enabled = val;

        return m_Enabled;
    }

    bool enabled() const {
        return m_Enabled;
    }

    const char *id() const {
        return (const char *)m_Id;
    }

    void id(const char *id) {
        if (m_Id) {
            free(m_Id);
        }
        m_Id = id ? strdup(id) : NULL;
    }

    const char *title() const {
        return m_Title;
    }

    void title(const char *title) {
        if (m_Title) {
            free(m_Title);
        }
        m_Title = title ? strdup(title) : NULL;
    }

private:
    char *m_Id;
    char *m_Title;
    bool m_Enabled;
};

class NativeSystemMenu {
public:
    NativeSystemMenu(NativeUIInterface *ui);
    ~NativeSystemMenu();

    void enable(bool val);
    void setIcon(const uint8_t *data, size_t width, size_t height);
    void addItem(NativeSystemMenuItem *item);
    void deleteItems();
    const uint8_t *getIcon(size_t *len, size_t *width, size_t *height) const {
        *len = m_Icon.len;
        *width = m_Icon.width;
        *height = m_Icon.height;
        return m_Icon.data;
    }

    NativeSystemMenuItem *items() const {
        return m_Items;
    }
private:
    struct {
        const uint8_t *data;
        size_t len;
        size_t width, height;
    } m_Icon;

    NativeSystemMenuItem *m_Items;
    NativeUIInterface *m_UI;
};

class NativeUIInterface
{
    public:
        friend class NativeSystemMenu;

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
        } m_CurrentCursor;

        enum OPENFILE_FLAGS {
            kOpenFile_CanChooseDir = 1 << 0,
            kOpenFile_CanChooseFile = 1 << 1,
            kOpenFile_AlloMultipleSelection = 1 << 2
        };

        Nidium::NML::NativeContext *m_NativeCtx;
        Nidium::NML::NativeNML *m_Nml;
        struct SDL_Window *m_Win;
        struct _ape_global *m_Gnet;
        int m_Argc = 0;
        char **m_Argv = nullptr;

        inline Nidium::NML::NativeContext *getNativeContext() const {
            return m_NativeCtx;
        }

        NativeUIInterface();
        virtual ~NativeUIInterface() {};
        virtual void stopApplication()=0;
        virtual void restartApplication(const char *path=NULL)=0;

        virtual void refreshApplication(bool clearConsole = false);
        virtual bool runJSWithoutNML(const char *path, int width = 800, int height = 600) {
            return false;
        };
        void setArguments(int argc, char **argv) {
            m_Argc = argc;
            m_Argv = argv;
        }
        virtual bool runApplication(const char *path)=0;
        virtual void setWindowTitle(const char *)=0;
        virtual const char *getWindowTitle() const=0;
        virtual void setCursor(CURSOR_TYPE)=0;
        virtual void runLoop()=0;
        virtual void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {};
        virtual void setWindowControlsOffset(double x, double y) {};
        virtual void setClipboardText(const char *text)=0;
        virtual char *getClipboardText()=0;
        virtual void openFileDialog(const char *files[],
            void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg, int flags=0)=0;
        virtual const char *getCacheDirectory() const=0;

        virtual void setWindowSize(int w, int h);
        virtual void setWindowFrame(int x, int y, int w, int h);
        virtual void getScreenSize(int *width, int *height);
        virtual void setWindowPosition(int x, int y);
        virtual void getWindowPosition(int *x, int *y);
        virtual void centerWindow();

        virtual void log(const char *buf)=0;
        virtual void logf(const char *format, ...)=0;
        virtual void vlog(const char *buf, va_list ap)=0;
        virtual void logclear()=0;

        virtual void alert(const char *message)=0;

        virtual void refresh();

        int getWidth() const { return this->m_Width; }
        int getHeight() const { return this->m_Height; }
        class NativeUIConsole
        {
            public:
            virtual void log(const char *str)=0;
            virtual void show()=0;
            virtual void hide()=0;
            virtual void clear()=0;
            virtual bool hidden()=0;
        };
        virtual NativeUIConsole *getConsole(bool create=false, bool *created=NULL)=0;

        virtual bool makeMainGLCurrent();
        virtual bool makeGLCurrent(SDL_GLContext ctx);
        virtual SDL_GLContext getCurrentGLContext();
        virtual int useOffScreenRendering(bool val);
        virtual void toggleOfflineBuffer(bool val);
        virtual void enableSysTray(const void *imgData = NULL, size_t imageDataSize = 0) {};
        virtual void disableSysTray() {};

        virtual void quit();

        uint8_t *readScreenPixel();

        void initPBOs();

        bool hasPixelBuffer() const {
            return m_ReadPixelInBuffer;
        }

        SDL_GLContext getGLContext() {
            return m_MainGLCtx;
        }

        SDL_GLContext createSharedContext(bool webgl = false);


        void deleteGLContext(SDL_GLContext ctx);

        int getFBO() const {
            return m_FBO;
        }

        uint8_t *getFrameBufferData() {
            return m_FrameBuffer;
        }

        virtual void hideWindow();
        virtual void showWindow();
        bool isWindowHidden() const {
            return m_Hidden;
        }

        NativeSystemMenu &getSystemMenu() {
            return m_SystemMenu;
        }

    protected:
        virtual void renderSystemTray() {};

        int m_Width;
        int m_Height;
        char *m_FilePath;
        bool m_Initialized;
        bool m_IsOffscreen;
        bool m_ReadPixelInBuffer;
        bool m_Hidden;
        int m_FBO;
        uint8_t *m_FrameBuffer;

#define NUM_PBOS 1

        struct {
            uint32_t pbo[NUM_PBOS];
            int vram2sys;
            int gpu2vram;
        } m_PBOs;


        NativeUIConsole *m_Console;
        SDL_GLContext m_MainGLCtx;
        NativeSystemMenu m_SystemMenu;
};

#endif

