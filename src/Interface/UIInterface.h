/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_uiinterface_h__
#define interface_uiinterface_h__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NIDIUM_WINDOWPOS_UNDEFINED_MASK 0xFFFFFFF0
#define NIDIUM_WINDOWPOS_CENTER_MASK 0xFFFFFFF1

struct SDL_Window;
typedef struct _ape_global ape_global;
typedef union SDL_Event SDL_Event;

namespace Nidium {
namespace Frontend {
class Context;
class NML;
}
namespace Binding{
class JSWindow;
}

namespace Interface {

class UIInterface;

typedef void *SDL_GLContext;

// {{{ SystemMenuItem
class SystemMenuItem
{
public:
    SystemMenuItem(char *title = NULL, char *id = NULL)
        : m_Next(NULL), m_Id(NULL), m_Title(NULL), m_Enabled(false)
    {
        this->id(id);
        this->title(title);
    }

    ~SystemMenuItem()
    {
        free(m_Title);
        free(m_Id);
    };

    bool enabled(bool val)
    {
        m_Enabled = val;

        return m_Enabled;
    }

    bool enabled() const
    {
        return m_Enabled;
    }

    const char *id() const
    {
        return static_cast<const char *>(m_Id);
    }

    void id(const char *id)
    {
        if (m_Id) {
            free(m_Id);
        }
        m_Id = id ? strdup(id) : NULL;
    }

    const char *title() const
    {
        return m_Title;
    }

    void title(const char *title)
    {
        if (m_Title) {
            free(m_Title);
        }
        m_Title = title ? strdup(title) : NULL;
    }

    SystemMenuItem *m_Next;

private:
    char *m_Id;
    char *m_Title;
    bool m_Enabled;
};
// }}}

// {{{ SystemMenu
class SystemMenu
{
public:
    SystemMenu(UIInterface *ui);
    ~SystemMenu();

    void enable(bool val);
    void setIcon(const uint8_t *data, size_t width, size_t height);
    void addItem(SystemMenuItem *item);
    void deleteItems();
    const uint8_t *getIcon(size_t *len, size_t *width, size_t *height) const
    {
        *len    = m_Icon.len;
        *width  = m_Icon.width;
        *height = m_Icon.height;
        return m_Icon.data;
    }

    SystemMenuItem *items() const
    {
        return m_Items;
    }

private:
    struct
    {
        const uint8_t *data;
        size_t len;
        size_t width, height;
    } m_Icon;

    SystemMenuItem *m_Items;
    UIInterface *m_UI;
};
// }}}

// {{{ UIInterface
class UIInterface
{
public:
    friend class SystemMenu;

    enum CURSOR_TYPE
    {
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
        HIDDEN
    } m_CurrentCursor;

    enum KeyModifier
    {
        kKeyModifier_Shift   = 1 << 0,
        kKeyModifier_Alt     = 1 << 1,
        kKeyModifier_Control = 1 << 2,
        kKeyModifier_Meta    = 1 << 3
    };

    enum OPENFILE_FLAGS
    {
        kOpenFile_CanChooseDir          = 1 << 0,
        kOpenFile_CanChooseFile         = 1 << 1,
        kOpenFile_AlloMultipleSelection = 1 << 2
    };

    inline Frontend::Context *getNidiumContext() const
    {
        return m_NidiumCtx;
    }


    virtual ~UIInterface(){};

    /*
        Start the window main loop
        This call must be the last one upon initialization
        (it blocks by running the APE event loop)
    */
    virtual void runLoop() = 0;


    /*
        Set OpenGL context attribute
     */
    virtual void setGLContextAttribute();

    /*
        Create the initial window
        ::onWindowCreated() is then called on the subclass
    */
    virtual bool createWindow(int width, int height);

    /*
        Set the system window title
    */
    virtual void setWindowTitle(const char *title);

    /*
        Get the system window title
    */
    virtual const char *getWindowTitle() const;

    /*
        Shutdown the application
    */
    virtual void quitApplication() = 0;

    /*
        Run the NML at the specified path
    */
    virtual bool runApplication(const char *path);

    /*
        Stop the current running nml application
    */
    virtual void stopApplication();

    /*
        Start a new application
    */
    virtual void restartApplication(const char *path = NULL);

    /*
        Refresh the current application
        Internally calls restartApplication with the current NML Path
    */
    virtual void refreshApplication(bool clearConsole = false);

    /*
        Schedule a cursor change
    */
    virtual void setCursor(CURSOR_TYPE);

    virtual void hideCursor(bool state);

    /*
        Change the window width and height
    */
    virtual void setWindowSize(int w, int h);

    /*
        Change the window position and size
    */
    virtual void setWindowFrame(int x, int y, int w, int h);

    virtual void
    setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a){};
    virtual void setWindowControlsOffset(double x, double y){};
    virtual void setClipboardText(const char *text);
    virtual char *getClipboardText();
    virtual void
    openFileDialog(const char *files[],
                   void (*cb)(void *nof, const char *lst[], uint32_t len),
                   void *arg,
                   int flags = 0)
        = 0;

    virtual void getScreenSize(int *width, int *height);
    virtual void setWindowPosition(int x, int y);
    virtual void getWindowPosition(int *x, int *y);
    virtual void centerWindow();

    virtual void refresh();

    virtual void setSignalHandler();
    virtual void signalHandler(int sign);

    void setArguments(int argc, char **argv)
    {
        m_Argc = argc;
        m_Argv = argv;
    }

    int getWidth() const
    {
        return this->m_Width;
    }

    int getHeight() const
    {
        return this->m_Height;
    }

    virtual void hideWindow();
    virtual void showWindow();

    virtual bool makeMainGLCurrent();
    virtual bool makeGLCurrent(SDL_GLContext ctx);
    virtual SDL_GLContext getCurrentGLContext();
    virtual int useOffScreenRendering(bool val);
    virtual void toggleOfflineBuffer(bool val);
    virtual void enableSysTray(){};
    virtual void disableSysTray(){};

    virtual void handleEvent(const SDL_Event *ev);

    virtual void quit();

    uint8_t *readScreenPixel();

    void initPBOs();

    bool hasPixelBuffer() const
    {
        return m_ReadPixelInBuffer;
    }

    SDL_GLContext getGLContext()
    {
        return m_MainGLCtx;
    }

    SDL_GLContext createSharedContext(bool webgl = false);


    void deleteGLContext(SDL_GLContext ctx);

    int getFBO() const
    {
        return m_FBO;
    }

    uint8_t *getFrameBufferData()
    {
        return m_FrameBuffer;
    }

    bool isWindowHidden() const
    {
        return m_Hidden;
    }

    SystemMenu &getSystemMenu()
    {
        return m_SystemMenu;
    }

    static int HandleEvents(void *arg);

    static void OnNMLLoaded(void *arg);

    class UIConsole
    {
    public:
        virtual void log(const char *str) = 0;
        virtual void show() = 0;
        virtual void hide() = 0;
        virtual void clear() = 0;
        virtual void flush() = 0;
        virtual bool hidden() = 0;
    };
    virtual UIConsole *getConsole(bool create = false, bool *created = NULL)
        = 0;

    Frontend::Context *m_NidiumCtx;
    Frontend::NML *m_Nml;
    SDL_Window *m_Win;
    ape_global *m_Gnet;
    int m_Argc    = 0;
    char **m_Argv = nullptr;

protected:
    UIInterface();
    bool isContextReady();
    virtual void initControls();
    virtual void setupWindow(){};
    virtual void onWindowCreated(){};
    virtual void onNMLLoaded();
    virtual void setSystemCursor(CURSOR_TYPE cursor)=0;

    /*
        Ctrl+R action
        OSX: does nothing, this is handled by the menu action
    */
    virtual void hitRefresh()
    {
    }

    int m_Width;
    int m_Height;
    char *m_FilePath;
    bool m_Initialized;
    bool m_IsOffscreen;
    bool m_ReadPixelInBuffer;
    bool m_Hidden;
    int m_FBO;
    uint8_t *m_FrameBuffer;
    bool m_PendingRefresh = false;

#define NUM_PBOS 1

    struct
    {
        uint32_t pbo[NUM_PBOS];
        int vram2sys;
        int gpu2vram;
    } m_PBOs;


    UIConsole *m_Console;
    SDL_GLContext m_MainGLCtx;
    SystemMenu m_SystemMenu;

    bool m_CursorNeedsUpdate = false;
};
// }}}

} // namespace Interface
} // namespace Nidium

#endif
