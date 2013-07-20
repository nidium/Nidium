
#include "NativeTerminalUIInterface.h"
#include <NativeJS.h>
#include <NativeSkia.h>
#include <NativeApp.h>
#include <native_netlib.h>
#include <NativeNML.h>
#include <string.h>
#include <stdio.h>

uint32_t ttfps = 0;

static int NativeProcessUI(void *arg)
{
    return 16; 
}

bool NativeTerminalUIInterface::runApplication(const char *path)
{
    this->console = new NativeUITerminalConsole();
    this->gnet = native_netlib_init();
    this->NJS = new NativeJS(1, 1, this, gnet);

    this->nml = new NativeNML(this->gnet);
    this->nml->setNJS(this->NJS);
    this->nml->loadFile("index.nml");

    return true;
}

NativeTerminalUIInterface::NativeTerminalUIInterface()
{
    this->width = 0;
    this->height = 0;
    this->nml = NULL;

    this->currentCursor = NOCHANGE;
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
    printf("openFileDialog not implemented\n");
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
    add_timer(&gnet->timersng, 1, NativeProcessUI, (void *)this);
    
    events_loop(gnet);  
}

NativeUITerminalConsole::NativeUITerminalConsole () 
    : isHidden(false), needFlush(false)
{
}

void NativeUITerminalConsole::log(const char *str)
{
    if (strcmp("\n", str) == 0) {
        printf("\n");
    } else {
        printf("[CONSOLE] %s", str);
    }
}

void NativeUITerminalConsole::show()
{
}

void NativeUITerminalConsole::hide()
{
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
