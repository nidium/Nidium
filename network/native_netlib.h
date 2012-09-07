#ifndef _NATIVE_NETLIB_H_
#define _NATIVE_NETLIB_H_

#include "common.h"
#include "ape_socket.h"
#include "ape_events_loop.h"
#include "ape_timers.h"
#include "ape_timers_next.h"

#ifdef __cplusplus
extern "C" {
#endif

ape_global *native_netlib_init();	

#ifdef __cplusplus
}
#endif

#endif
