#include "YGEnums.h"
#include <stdlib.h>
#include <string.h>

YGFlexDirection YGFlexDirectionFromString(const char *value)
{
    static struct ygvalstr {
        const char *name;
        YGFlexDirection direction;
    } enums[] = {
        {"column",           YGFlexDirectionColumn},
        {"row",              YGFlexDirectionRow},
        {"column-reverse",   YGFlexDirectionColumnReverse},
        {"row-reverse",      YGFlexDirectionRowReverse},
        {NULL,               YGFlexDirectionColumn}
    };

    for (int i = 0; enums[i].name != NULL; i++) {
        if (strcmp(value, enums[i].name) == 0) {
            return enums[i].direction;
        }
    }

    return YGFlexDirectionColumn;
}

YGJustify YGJustifyFromString(const char *value)
{
    static struct ygvalstr {
        const char *name;
        YGJustify direction;
    } enums[] = {
        {"flex-start",          YGJustifyFlexStart},
        {"center",              YGJustifyCenter},
        {"flex-end",            YGJustifyFlexEnd},
        {"space-between",       YGJustifySpaceBetween},
        {"space-around",        YGJustifySpaceAround},
        {NULL,                  YGJustifyFlexStart}
    };

    for (int i = 0; enums[i].name != NULL; i++) {
        if (strcmp(value, enums[i].name) == 0) {
            return enums[i].direction;
        }
    }

    return YGJustifyFlexStart;
}

YGWrap YGWrapFromString(const char *value)
{
    static struct ygvalstr {
        const char *name;
        YGWrap direction;
    } enums[] = {
        {"no-wrap",          YGWrapNoWrap},
        {"wrap",             YGWrapWrap},
        {"wrap-reverse",     YGWrapWrapReverse},
        {NULL,               YGWrapNoWrap}
    };

    for (int i = 0; enums[i].name != NULL; i++) {
        if (strcmp(value, enums[i].name) == 0) {
            return enums[i].direction;
        }
    }

    return YGWrapNoWrap;
}

YGAlign YGAlignFromString(const char *value)
{
    static struct ygvalstr {
        const char *name;
        YGAlign direction;
    } enums[] = {
        {"auto",           YGAlignAuto},
        {"flex-start",     YGAlignFlexStart},
        {"center",         YGAlignCenter},
        {"flex-end",       YGAlignFlexEnd},
        {"stretch",        YGAlignStretch},
        {"baseline",       YGAlignBaseline},
        {"space-between",  YGAlignSpaceBetween},
        {"space-around",   YGAlignSpaceAround},
        {NULL,             YGAlignFlexStart}
    };

    for (int i = 0; enums[i].name != NULL; i++) {
        if (strcmp(value, enums[i].name) == 0) {
            return enums[i].direction;
        }
    }

    return YGAlignFlexStart;
}
