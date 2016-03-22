#include "NativeTerminalUIInterface.h"

#include <string.h>
#include <stdio.h>

#include <native_netlib.h>

#include <NativeJS.h>

#include "NativeSkia.h"
#include "NativeApp.h"
#include "NativeNML.h"

uint32_t ttfps = 0;

static int NativeProcessUI(void *arg)
{
    return 16;
}

bool NativeTerminalUIInterface::runApplication(const char *path)
{
    this->m_Console = new NativeUITerminalConsole();
    this->m_Gnet = native_netlib_init();
    this->NJS = new NativeJS(1, 1, this, m_Gnet);

    this->m_Nml = new NativeNML(this->m_Gnet);
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
    this->setTitleBarRGBAColor(0, 0, 0, 0);
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
    m_TitleBarRGBAColor.r = r;
    m_TitleBarRGBAColor.g = g;
    m_TitleBarRGBAColor.b = b;
    m_TitleBarRGBAColor.a = a;
}

void NativeTerminalUIInterface::initControls()
{
}

void NativeTerminalUIInterface::setWindowControlsOffset(double x, double y)
{
    m_WindowControlsOffset.x = x;
    m_WindowControlsOffset.y = y;
}

void NativeTerminalUIInterface::runLoop()
{
    add_timer(&m_Gnet->timersng, 1, NativeProcessUI, (void *)this);

    events_loop(m_Gnet);
}

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

