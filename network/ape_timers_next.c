#include "ape_timers_next.h"
#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <mach/mach_time.h>


int process_timers(ape_timers *timers, int elapsed)
{
	ape_timer *cur = timers->head;

	while(cur != NULL) {
		if ((cur->ticks_left = ape_max(cur->ticks_left - elapsed, 0)) == 0) {
			int ret;
			uint64_t start = mach_absolute_time();
			unsigned int duration;
			ret = cur->callback(cur->arg);
			duration = (int)(mach_absolute_time() - start) / 100;
			elapsed += duration;

			if (ret == -1) {
				cur->ticks_left = cur->ticks_needs;
			} else if (ret == 0) {
				cur = del_timer(timers, cur);
				continue;
			} else {
				cur->ticks_left = cur->ticks_needs = ret*10000;
			}
			if (cur->stats.max < duration) {
				cur->stats.max = duration;
			}
			if (cur->stats.min == 0 || duration < cur->stats.min) {
				cur->stats.min = duration;
			}
			cur->stats.nexec++;
			cur->stats.totaltime += duration;

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
	free(timer);

	return ret;
}

void timer_stats_print(ape_timer *timer)
{
	printf("%d\t%d\t%d\t%d\t%d\t%d\n", timer->identifier,
		timer->stats.nexec,
		timer->stats.totaltime,
		timer->stats.max,
		timer->stats.min,
		timer->stats.totaltime/timer->stats.nexec);
}

void timers_stats_print(ape_timers *timers)
{
	ape_timer *cur;
	printf("=======================\n");
	printf("Id\ttimes\texec\tmax\tmin\tavg\n");
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
	timer->ticks_needs = timer->ticks_left = ms*10000;

	timer->arg = arg;
	timer->flags = 0;
	timer->prev = NULL;
	timer->identifier = timers->last_identifier;
	timer->next = timers->head;

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

