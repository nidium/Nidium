#ifndef _APE_COMMON_H_
#define _APE_COMMON_H_

#ifdef __linux__
  #define USE_EPOLL_HANDLER
#elif defined(__APPLE__)
  #define USE_KQUEUE_HANDLER
#elif defined(_MSC_VER)
  #define USE_SELECT_HANDLER
  #define __WIN32
#else
  #error "No suitable IO handler found"
#endif


#ifdef _MSC_VER
  #include <ares.h>
#else
  #include "ares.h"
#endif
#define APE_BASEMEM 4096

#define ape_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define ape_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))

#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1 : 0
#define CONST_STR_LEN2(x) x ? sizeof(x) - 1 : 0, x

#define _APE_ABS_MASK(val) (val >> sizeof(int) * 8 - 1)
#define APE_ABS(val) (val + _APE_ABS_MASK(val)) ^ _APE_ABS_MASK(val)

typedef struct _ape_global ape_global;

#include "ape_events.h"
#include "ape_timers_next.h"

struct _ape_global {
    int basemem;
    void *ctx; /* public */

    unsigned int seed;
    struct _fdevent events;

    struct {
        ares_channel channel;
        struct {
            struct _ares_sockets *list;
            size_t size;
            size_t used;
        } sockets;

    } dns;

	struct {
		struct _ticks_callback *timers;
		unsigned int ntimers;
	} timers;

	ape_timers timersng;
	
    int is_running;
};


#endif
