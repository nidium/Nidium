#pragma once

#include "YGMacros.h"
#include "YGEnums.h"

YG_EXTERN_C_BEGIN

WIN_EXPORT YGFlexDirection YGFlexDirectionFromString(const char *value);
WIN_EXPORT YGJustify YGJustifyFromString(const char *value);
WIN_EXPORT YGWrap YGWrapFromString(const char *value);
WIN_EXPORT YGAlign YGAlignFromString(const char *value);

YG_EXTERN_C_END
