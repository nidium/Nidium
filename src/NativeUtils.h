#ifndef nativeutils_h__
#define nativeutils_h__

#include <stdint.h>

class NativeUtils
{
    public:
    static uint64_t getTick();
};

#define native_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define native_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))
#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1 : 0

#endif
