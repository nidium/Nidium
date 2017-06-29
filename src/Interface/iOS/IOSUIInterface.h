/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_ios_iosuiinterface_h__
#define interface_ios_iosuiinterface_h__

#include "UIInterface.h"
#include "Frontend/InputHandler.h"

namespace Nidium {
namespace Interface {

class IOSUIInterface;

// {{{ DummyConsole
class DummyConsole : public UIInterface::UIConsole
{
public:
    DummyConsole(IOSUIInterface *interface) {};
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

// {{{ IOSUIInterface
class IOSUIInterface : public UIInterface
{
    friend class DummyConsole;

public:
    IOSUIInterface();
    void runLoop() override;
    void quitApplication() override;
    void openFileDialog(const char *files[],
                        void (*cb)(void *nof, const char *lst[], uint32_t len),
                        void *arg,
                        int flags = 0) override;
    void setGLContextAttribute() override;
    bool createWindow(int width, int height) override;
    void handleEvent(const SDL_Event *ev) override;

    DummyConsole *getConsole(bool create = false, bool *created = NULL) override
    {
        return m_Console;
    }

    void bindFramebuffer() override;

    void bridge(const char *data) override;

    void initControls() override {};
protected:
    void renderSystemTray() {};
    void setSystemCursor(CURSOR_TYPE cursor) override {};
    void hitRefresh() override;
    void onWindowCreated() override;

private:
    DummyConsole *m_Console;
    int toLogicalSize(int physicalSize);
    id m_NidiumWindow = nullptr;
};
// }}}

} // namespace Interface
} // namespace Nidium

#endif
