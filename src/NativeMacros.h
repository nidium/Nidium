#ifndef nativemacros_h__
#define nativemacros_h__

#include <string.h>

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define NLOG(format, ...) \
    printf("[%s:%d] " format "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)

#endif