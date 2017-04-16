/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef macros_h__
#define macros_h__

#ifdef __cplusplus

#include <string.h>
#include "Interface/UIInterface.h"
#include <ape_log.h>
#ifdef __ANDROID__
#include <android/log.h>
#endif

#define __FILENAME__ \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define NDM_LOG_ERROR    APE_LOG_ERROR
#define NDM_LOG_WARN     APE_LOG_WARN
#define NDM_LOG_INFO     APE_LOG_INFO
#define NDM_LOG_DEBUG    APE_LOG_DEBUG

#define ndm_logf(level, tag, format, ...) \
    APE_logf(level, tag, "(%s:%d) " format, __FILENAME__, __LINE__, ##__VA_ARGS__)

#define ndm_log(level, tag, data) \
    APE_logf(level, tag, "(%s:%d) %s", __FILENAME__, __LINE__, data)

#define ndm_printf(format, ...) \
    APE_logf(APE_LOG_INFO, nullptr, "(%s:%d) " format, __FILENAME__, __LINE__, ##__VA_ARGS__)

#define ndm_print(data) \
    APE_logf(APE_LOG_INFO, nullptr, "(%s:%d) %s", __FILENAME__, __LINE__, data)

#define nlogf ndm_printf
#define nlog ndm_print

#endif

#endif
