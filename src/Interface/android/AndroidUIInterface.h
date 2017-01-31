/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_linux_x11uiinterface_h__
#define interface_linux_x11uiinterface_h__

#include "UIInterface.h"
#include <jnipp.h>

namespace Nidium {
namespace Interface {

class AndroidUIInterface;

// {{{ DummyConsole
class DummyConsole : public UIInterface::UIConsole
{
public:
    DummyConsole(AndroidUIInterface *interface) {};
    ~DummyConsole() {};
    void log(const char *str) {};
    void show() {};
    void hide() {};
    void clear() {};
    void flush() {};
    bool hidden()
    {
        return true;
    };
};
// }}}

// {{{ AndroidUIInterface
class AndroidUIInterface : public UIInterface
{
    friend class DummyConsole;

public:
    AndroidUIInterface();
    void runLoop() override;
    void quitApplication() override;
    void openFileDialog(const char *files[],
                        void (*cb)(void *nof, const char *lst[], uint32_t len),
                        void *arg,
                        int flags = 0) override;
    void setGLContextAttribute() override;
    bool createWindow(int width, int height) override;

    DummyConsole *getConsole(bool create = false, bool *created = NULL) override
    {
        return m_Console;
    }

protected:
    void renderSystemTray() {};
    void setSystemCursor(CURSOR_TYPE cursor) override {};
    void hitRefresh() override;
    void onWindowCreated() override;

private:
    DummyConsole *m_Console;
};
// }}}

} // namespace Interface
} // namespace Nidium

#endif
