#ifndef macros_h__
#define macros_h__

#include <string.h>

#include <UIInterface.h>

namespace Nidium {
    namespace Interface {
        extern UIInterface *__NativeUI;
    }
namespace Frontend {

#define NLOG(format, ...) \
    Nidium::Interface::__NativeUI->logf("[%s:%d] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)

} // namespace Frontend
} // namespace Nidium

#endif

