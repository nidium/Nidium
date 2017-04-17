/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_windows_winuiinterface_h__
#define interface_windows_winuiinterface_h__

#include "UIInterface.h"

#include <Port/MSWindows.h>

namespace Nidium {
namespace Interface {

class UIWinInterface;

// {{{ UIWinConsole
class UIWinConsole : public UIInterface::UIConsole
{
public:
    UIWinConsole(UIWinInterface *iface) : m_Interface(iface){};
    ~UIWinConsole(){};
    void log(const char *str);
    void show();
    void hide();
    void clear(){};
    void flush(){};
    bool hidden()
    {
        return !m_IsOpen;
    };

private:
    bool m_IsOpen               = false;
    UIWinInterface *m_Interface = nullptr;

    //GtkWidget *m_Window     = nullptr;
    //GtkTextBuffer *m_Buffer = nullptr;
    //GtkWidget *m_TextView   = nullptr;
    //GtkWidget *m_Scroll     = nullptr;
};
// }}}

// {{{ UIWinInterface
class UIWinInterface : public UIInterface
{
    friend class UIWinConsole;

public:
    UIWinInterface();
    void runLoop();
    void quitApplication();
    void setTitleBarRGBAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a){};
    void setWindowControlsOffset(double x, double y){};
    void openFileDialog(const char *files[],
                        void (*cb)(void *nof, const char *lst[], uint32_t len),
                        void *arg,
                        int flags = 0);
    void enableSysTray();

    UIWinConsole *getConsole(bool create = false, bool *created = NULL)
    {
        return m_Console;
    }

    /*
    struct {
        CGRect closeFrame;
        CGRect zoomFrame;
        CGRect minFrame;
    } m_Controls;
    */

    struct
    {
        char *buf;
        size_t len;
        size_t offset;
    } m_Mainjs;

    void setArguments(int argc, char **argv, HINSTANCE hInstance)
    {
        UIInterface::setArguments(argc, argv);
        m_hInstance = hInstance;
    }


protected:
    void renderSystemTray();
    void setSystemCursor(CURSOR_TYPE cursor);
    void hitRefresh();
    void onWindowCreated();

private:
    void processWinPendingEvents();
    HINSTANCE m_hInstance;
    UIWinConsole *m_Console;
};
// }}}

} // namespace Interface
} // namespace Nidium

#endif
