#ifndef nidium_macros_h__
#define nidium_macros_h__

#include <string.h>

#include <UIInterface.h>

extern NativeUIInterface *__NativeUI;

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define NLOG(format, ...) \
    __NativeUI->logf("[%s:%d] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)

#endif

