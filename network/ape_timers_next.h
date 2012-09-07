#ifndef __APE_TIMERS_NEXT_H
#define __APE_TIMERS_NEXT_H

typedef int (*timer_callback)(void *arg);

typedef struct _ape_timer
{
	int identifier;
	int flags;
	int ticks_left;
	int ticks_needs;
	int nexec;
	timer_callback callback;
	void *arg;

	struct {
		unsigned int nexec;
		unsigned int max;
		unsigned int min;
		unsigned int totaltime;
	} stats;

	struct _ape_timer *next;
	struct _ape_timer *prev;
} ape_timer;


typedef struct _ape_timers
{
	ape_timer *head;
	int last_identifier;
} ape_timers;

#ifdef __cplusplus
extern "C" {
#endif

int process_timers(ape_timers *timers, int elapsed);
ape_timer *del_timer(ape_timers *timers, ape_timer *timer);
ape_timer *add_timer(ape_timers *timers, int ms, timer_callback cb, void *arg);
ape_timer *get_timer_by_id(ape_timers *timers, int identifier);
void timer_stats_print(ape_timer *timer);
void timers_stats_print(ape_timers *timers);

#ifdef __cplusplus
}
#endif

#define timer_dispatch_async(callback, params) add_timer(&ape->timersng, 1, callback, params)


#endif
