#ifndef __APE_TIMERS_H
#define __APE_TIMERS_H

#include "common.h"

#define VTICKS_RATE 50 // 50 ms

struct _ticks_callback
{
	int ticks_need;
	int delta;
	int times;
	unsigned int identifier;
	unsigned int protect;

	void *func;
	void *params;
	
	struct _ticks_callback *next;
};

void process_tick(ape_global *ape);
struct _ticks_callback *add_timeout(unsigned int msec, void *callback, void *params, ape_global *ape);
struct _ticks_callback *add_periodical(unsigned int msec, int times, void *callback, void *params, ape_global *ape);
void del_timer_identifier(unsigned int identifier, ape_global *ape);
struct _ticks_callback *get_timer_identifier(unsigned int identifier, ape_global *ape);
int get_first_timer_ms(ape_global *ape);
void timers_free(ape_global *ape);

#define add_ticked(x, y) add_periodical(VTICKS_RATE, 0, x, y, ape)
#define ape_dispatch_async(callback, params) add_timeout(1, callback, params, ape)

#endif
