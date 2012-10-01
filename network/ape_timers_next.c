#include "ape_timers_next.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>

#if defined(__APPLE__)
  #include <mach/mach_time.h>
#else
  #include <time.h>
static inline uint64_t mach_absolute_time()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);

	return t.tv_sec * 1000000000 + t.tv_nsec;
}
#endif

int process_timers(ape_timers *timers)
{
	ape_timer *cur = timers->head;

	/* TODO: paused timer */

	while (cur != NULL) {
		uint64_t start;

		if (cur->flags & APE_TIMER_IS_CLEARED) {
			cur = del_timer(timers, cur);
			continue;
		}

		if ((start = mach_absolute_time()) >= cur->schedule-150000) {
			uint64_t ret;
			unsigned int duration;
			
			ret = cur->callback(cur->arg);

			if (ret == -1) {
				cur->schedule = start + cur->ticks_needs;
			} else if (ret == 0) {
				cur = del_timer(timers, cur);
				continue;
			} else {
				cur->ticks_needs = ret * 1000000;
				cur->schedule = start + cur->ticks_needs;
			}
			duration = mach_absolute_time() - start;

			if (cur->stats.max < duration / 1000000) {
				cur->stats.max = duration / 1000000;
			}
			if (cur->stats.min == 0 || duration / 1000000 < cur->stats.min) {
				cur->stats.min = duration / 1000000;
			}
			cur->stats.nexec++;
			cur->stats.totaltime += duration / 1000000;

		}

		cur = cur->next;
	}

	return 0;
}

ape_timer *get_timer_by_id(ape_timers *timers, int identifier)
{
	ape_timer *cur;
	for (cur = timers->head; cur != NULL; cur = cur->next) {
		if (cur->identifier == identifier) {
			return cur;
		}
	}
	return NULL;
}

void clear_timer_by_id(ape_timers *timers, int identifier, int force)
{
	ape_timer *cur;
	for (cur = timers->head; cur != NULL; cur = cur->next) {
		if (cur->identifier == identifier) {
			if (!(cur->flags & APE_TIMER_IS_PROTECTED) ||
				(cur->flags & APE_TIMER_IS_PROTECTED && force)) {
				
				cur->flags |= APE_TIMER_IS_CLEARED;
			}
			return;
		}
	}	
}

void del_timers_unprotected(ape_timers *timers)
{
	ape_timer *cur = timers->head;

	while (cur != NULL) {
		if (!(cur->flags & APE_TIMER_IS_PROTECTED)) {
			cur = del_timer(timers, cur);
			continue;
		}

		cur = cur->next;
	}
}

/* delete *timer* and returns *timer->next* */
ape_timer *del_timer(ape_timers *timers, ape_timer *timer)
{
	ape_timer *ret;

	if (timer->prev == NULL) {
		timers->head = timer->next;
	} else {
		timer->prev->next = timer->next;
	}
	if (timer->next != NULL) {
		timer->next->prev = timer->prev;
	}
	ret = timer->next;

	if (timer->clearfunc) {
		timer->clearfunc(timer->arg);
	}
	
	free(timer);

	return ret;
}

void timer_stats_print(ape_timer *timer)
{
	printf("%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", timer->identifier,
		timer->stats.nexec,
		timer->stats.totaltime,
		timer->stats.max,
		timer->stats.min,
		(timer->stats.nexec == 0 ? timer->stats.totaltime :
			timer->stats.totaltime/timer->stats.nexec));
}

void timers_stats_print(ape_timers *timers)
{
	ape_timer *cur;
	printf("=======================\n");
	printf("Id\t\ttimes\texec\t\tmax\t\tmin\t\tavg\n");
	for (cur = timers->head; cur != NULL; cur = cur->next) {
		timer_stats_print(cur);
	}
	printf("=======================\n");
}

ape_timer *add_timer(ape_timers *timers, int ms, timer_callback cb, void *arg)
{
	ape_timer *timer = malloc(sizeof(ape_timer));

	timers->last_identifier++;
	timer->callback = cb;
	timer->ticks_needs = (uint64_t)ms * 1000000;
	timer->schedule = mach_absolute_time() + timer->ticks_needs;
	timer->arg = arg;
	timer->flags = APE_TIMER_IS_PROTECTED;
	timer->prev = NULL;
	timer->identifier = timers->last_identifier;
	timer->next = timers->head;
	timer->clearfunc = NULL;

	timer->stats.nexec = 0;
	timer->stats.totaltime = 0;
	timer->stats.max = 0;
	timer->stats.min = 0;

	if (timers->head != NULL) {
		timers->head->prev = timer;
	}

	timers->head = timer;

	return timer;
}

