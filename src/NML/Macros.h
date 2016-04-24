#ifndef nml_macros_h__
#define nml_macros_h__

#include <string.h>

#include <UIInterface.h>

namespace Nidium {
    namespace Interface {
        extern NativeUIInterface *__NativeUI;
    }
namespace NML {

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define NLOG(format, ...) \
    Nidium::Interface::__NativeUI->logf("[%s:%d] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)

} // namespace NML
} // namespace Nidium

#endif

