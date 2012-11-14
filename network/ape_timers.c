#ifndef _MSC_VER
  #include <sys/time.h>
#endif
#include <time.h>
#include "ape_timers.h"
#include <stdlib.h>

__inline void process_tick(ape_global *ape)
{
	struct _ticks_callback *timers = ape->timers.timers;

	
	while (timers != NULL && --timers->delta <= 0) {
		int lastcall = (timers->times > 0 && --timers->times == 0);
		void (*func_timer)(void *param, int *) = timers->func;
		int (*func_timer_resample)(void *param, int *) = timers->func;
		int resample = 0;

		if (timers->flag & APE_TIMER_RESAMPLE) {
			resample = func_timer_resample(timers->params, &lastcall);
			if (resample == 0) {
				resample = timers->ticks_need;
			}
		} else {
			resample = timers->ticks_need;
			func_timer(timers->params, &lastcall);
		}
		ape->timers.timers = timers->next;

		if (!lastcall) {
			struct _ticks_callback *new_timer = add_timeout(resample, 
													timers->func, 
													timers->params, ape);
			
			ape->timers.ntimers--;
			new_timer->identifier = timers->identifier;
			new_timer->times = timers->times;
			new_timer->flag = timers->flag;
		}

		free(timers);

		if ((timers = ape->timers.timers) != NULL) {
			timers->delta++;
		}
	}
}

struct _ticks_callback *add_timeout(unsigned int msec, 
		void *callback, void *params, ape_global *ape)
{
	struct _ticks_callback *timers = ape->timers.timers;
	struct _ticks_callback *prev = NULL;

	struct _ticks_callback *new_timer;
	new_timer = malloc(sizeof(*new_timer));
	
	new_timer->ticks_need = msec;
	new_timer->delta = msec;
	new_timer->times = 1;
	new_timer->identifier = ape->timers.ntimers;
	new_timer->flag = APE_TIMER_PROTECTED;
	new_timer->func = callback;
	new_timer->params = params;
	new_timer->next = NULL;

	while (timers != NULL) {
		if (new_timer->delta <= timers->delta) {
			new_timer->next = timers;
			timers->delta -= new_timer->delta;
			break;
		}

		new_timer->delta -= timers->delta;
		prev = timers;
		timers = timers->next;
	}

	if (prev != NULL) {
		prev->next = new_timer;
	} else {
		ape->timers.timers = new_timer;
	}
	
	ape->timers.ntimers++;

	return new_timer;
}

/* Exec callback "times"x each "sec" */
/* If "times" is 0, the function is executed infinitely */

struct _ticks_callback *add_periodical(unsigned int msec,
	int times, void *callback, void *params, int resamp, ape_global *ape)
{
	struct _ticks_callback *new_timer = add_timeout(msec, callback, params, ape);

	if (resamp)
		new_timer->flag |= APE_TIMER_RESAMPLE;

	new_timer->times = times;
	
	return new_timer;
}

struct _ticks_callback *get_timer_identifier(unsigned int identifier,
	ape_global *ape)
{
	struct _ticks_callback *timers = ape->timers.timers;
	
	while (timers != NULL) {
		if (timers->identifier == identifier) {
			return timers;
		}
		timers = timers->next;
	}
	
	return NULL;
}

void del_timer_identifier(unsigned int identifier, ape_global *ape)
{
	struct _ticks_callback *timers = ape->timers.timers;
	struct _ticks_callback *prev = NULL;
	
	while (timers != NULL) {
		if (timers->identifier == identifier) {
			if (prev != NULL) {
				prev->next = timers->next;
			} else {
				ape->timers.timers = timers->next;
			}

			free(timers);
			break;
		}

		prev = timers;
		timers = timers->next;
	}
}

/* Returns closest timer execution time (in ms) */
int get_first_timer_ms(ape_global *ape)
{
	struct _ticks_callback *timers = ape->timers.timers;

	if (timers != NULL) {
		return timers->delta;
	}

	return -1;
}

/* Delete all timers and deallocate memory */
void timers_free(ape_global *ape)
{
	struct _ticks_callback *timers = ape->timers.timers;
	struct _ticks_callback *prev;

	while (timers != NULL) {
		prev = timers;
		timers = timers->next;
		free(prev);
	}
}


