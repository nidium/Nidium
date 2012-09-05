#ifndef _APE_COMMON_H_
#define _APE_COMMON_H_

#define USE_KQUEUE_HANDLER

#define APE_BASEMEM 4096

#define ape_min(val1, val2)  ((val1 > val2) ? (val2) : (val1))
#define ape_max(val1, val2)  ((val1 < val2) ? (val2) : (val1))

#define CONST_STR_LEN(x) x, x ? sizeof(x) - 1 : 0
#define CONST_STR_LEN2(x) x ? sizeof(x) - 1 : 0, x

#define _APE_ABS_MASK(val) (val >> sizeof(int) * 8 - 1)
#define APE_ABS(val) (val + _APE_ABS_MASK(val)) ^ _APE_ABS_MASK(val)

typedef struct _ape_global ape_global;

#include "ape_events.h"

struct _ape_global {
    int basemem;
    void *ctx; /* public */

    unsigned int seed;
    struct _fdevent events;
    
	struct {
		struct _ticks_callback *timers;
		unsigned int ntimers;
	} timers;
	
    int is_running;
};

#endif
