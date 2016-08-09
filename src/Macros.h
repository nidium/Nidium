#ifndef macros_h__
#define macros_h__

#include <string.h>

#include <UIInterface.h>
#include "Macros.h"

namespace Nidium {
namespace Interface {
extern UIInterface *__NidiumUI;
}
}

#define NUI_LOG(format, ...)                                 \
    Nidium::Interface::__NidiumUI->getNidiumContext()->vlog( \
        "[%s:%d] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)
#endif
