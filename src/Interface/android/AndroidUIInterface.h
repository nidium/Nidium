/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_linux_x11uiinterface_h__
#define interface_linux_x11uiinterface_h__

#include "UIInterface.h"
#include "Core/Messages.h"
#include "Frontend/InputHandler.h"
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
class AndroidUIInterface : public UIInterface,
                           public Nidium::Core::Messages
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
    void handleEvent(const SDL_Event *ev) override;

    DummyConsole *getConsole(bool create = false, bool *created = NULL) override
    {
        return m_Console;
    }

    void initControls() override {};
    void onScroll(float x, float y, float velocityX, float velocityY, int state);
    void onFlingUpdate(int scrollX, int scrollY, bool finished);

    void onMessage(const Core::SharedMessages::Message &msg) override;

protected:
    void renderSystemTray() {};
    void setSystemCursor(CURSOR_TYPE cursor) override {};
    void hitRefresh() override;
    void onWindowCreated() override;

private:
    enum AndroidMessage
    {
        kAndroidMessage_scroll,
        kAndroidMessage_fling,
    };

    struct AndroidScrollMessage {
        int x;
        int y;
        int velocityX;
        int velocityY;
        Frontend::InputEvent::ScrollState state;

        AndroidScrollMessage(int x, int y,
                             int velocityX, int velocityY,
                             Frontend::InputEvent::ScrollState state)
            : x(x), y(y), velocityX(velocityX), velocityY(velocityY), state(state) { };
    };

    DummyConsole *m_Console;
    int toLogicalSize(int physicalSize);
};
// }}}

} // namespace Interface
} // namespace Nidium

#endif
