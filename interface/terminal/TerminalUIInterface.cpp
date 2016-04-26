#include "TerminalUIInterface.h"

#include <string.h>
#include <stdio.h>

#include <native_netlib.h>

#include <Binding/Nidium.JS.h>

#include "Graphics/SkiaContext.h"
#include "Frontend/App.h"
#include "Frontend/NML.h"

namespace Nidium {
namespace Interface {

uint32_t ttfps = 0;

static int NativeProcessUI(void *arg)
{
    return 16;
}

// {{{ NativeTerminalUIInterface
bool NativeTerminalUIInterface::runApplication(const char *path)
{
    this->m_Console = new NativeUITerminalConsole();
    this->m_Gnet = native_netlib_init();
    this->NJS = new Nidium::Binding::NidiumJS(1, 1, this, m_Gnet);

    this->m_Nml = new Nidium::Frontend::NML(this->m_Gnet);
    this->m_Nml->setNJS(this->NJS);
    this->m_Nml->loadFile("index.nml", NULL, NULL);

    return true;
}

NativeTerminalUIInterface::NativeTerminalUIInterface(): m_Console(NULL)
{
    this->m_Width = 0;
    this->m_Height = 0;
    this->m_Nml = NULL;

    this->m_CurrentCursor = NOCHANGE;
    this->NJS = NULL;
}


void NativeTerminalUIInterface::setCursor(CURSOR_TYPE type)
{
}

void NativeTerminalUIInterface::setWindowTitle(const char *name)
{
}

void NativeTerminalUIInterface::openFileDialog(const char const *files[],
    void (*cb)(void *nof, const char *lst[], uint32_t len), void *arg)
{
    fprintf(stdout, "openFileDialog not implemented\n");
}

void NativeTerminalUIInterface::setTitleBarRGBAColor(uint8_t r, uint8_t g,
    uint8_t b, uint8_t a)
{
}

void NativeTerminalUIInterface::initControls()
{
}

void NativeTerminalUIInterface::setWindowControlsOffset(double x, double y)
{
}

void NativeTerminalUIInterface::runLoop()
{
    APE_timer_create(m_Gnet, 1, NativeProcessUI, (void *)this);

    APE_loop_run(m_Gnet);
}
// }}}

// {{{ NativeUITerminalConsole
NativeUITerminalConsole::NativeUITerminalConsole ()
    : m_IsHidden(false), m_NeedFlush(false)
{
}

void NativeUITerminalConsole::log(const char *str)
{
    if (strcmp("\n", str) == 0) {
        fprintf(stdout, "\n");
    } else {
        fprintf(stdout, "[CONSOLE] %s", str);
    }
}

void NativeUITerminalConsole::show()
{
}

void NativeUITerminalConsole::hide()
{
}

bool NativeUITerminalConsole::hidden()
{
    return m_IsHidden;
}

void NativeUITerminalConsole::clear()
{
}

void NativeUITerminalConsole::flush()
{
}

NativeUITerminalConsole::~NativeUITerminalConsole()
{
}
// }}}

} // namespace Nidium
} // namespace Interface

