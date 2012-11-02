/* event_kqueue.c */

#include "common.h"
#include "ape_events.h"
#include "ape_socket.h"
#ifndef __WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <time.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef USE_KQUEUE_HANDLER
static int event_kqueue_add(struct _fdevent *ev, int fd, int bitadd, void *attach)
{
	struct kevent kev;
	struct timespec ts;
	int filter = 0;
		
	memset(&kev, 0, sizeof(kev));

//	printf("FD attached : %d %p\n", fd, attach);
	
	if (bitadd & EVENT_READ) {
		filter = EVFILT_READ; 

		ts.tv_sec = 0;
		ts.tv_nsec = 0;
	
		EV_SET(&kev, fd, filter, EV_ADD|EV_CLEAR, 0, 0, attach);
		if (kevent(ev->kq_fd, &kev, 1, NULL, 0, &ts) == -1) {
			return -1;
		}
	
	}

	if (bitadd & EVENT_WRITE) {
		filter = EVFILT_WRITE; 
	
		memset(&kev, 0, sizeof(kev));

		ts.tv_sec = 0;
		ts.tv_nsec = 0;
	
		EV_SET(&kev, fd, filter, EV_ADD|EV_CLEAR, 0, 0, attach);
		if (kevent(ev->kq_fd, &kev, 1, NULL, 0, &ts) == -1) {
			return -1;
		}
	
	}
//	printf("Succeed new socket\n");
	return 1;
}

static int event_kqueue_poll(struct _fdevent *ev, int timeout_ms)
{
	int nfds;
	struct timespec ts;
	
	ts.tv_sec = timeout_ms / 1000;
	ts.tv_nsec = (timeout_ms % 1000) * 1000000;

	if ((nfds = kevent(ev->kq_fd, NULL, 0, ev->events, *ev->basemem * 2, &ts)) == -1) {
		return -1;
	}

	return nfds;
}

static void *event_kqueue_get_fd(struct _fdevent *ev, int i)
{
	if (((ape_socket *)ev->events[i].udata)->states.state == APE_SOCKET_ST_OFFLINE) {
		return NULL;
	}
	return ev->events[i].udata;
}

static void event_kqueue_growup(struct _fdevent *ev)
{
	//ev->events = realloc(ev->events, sizeof(struct epoll_event) * (*ev->basemem));
}

static int event_kqueue_revent(struct _fdevent *ev, int i)
{
	int bitret = 0;
	
	if (ev->events[i].filter == EVFILT_READ) {
		bitret = EVENT_READ;
	} else if (ev->events[i].filter == EVFILT_WRITE) {
		bitret = EVENT_WRITE;
	}
	
	return bitret;
}


int event_kqueue_reload(struct _fdevent *ev)
{
	int nfd;
	if ((nfd = dup(ev->kq_fd)) != -1) {
		close(nfd);
		close(ev->kq_fd);
	}

	if ((ev->kq_fd = kqueue()) == -1) {
		return 0;
	}
	return 1;
}

int event_kqueue_init(struct _fdevent *ev)
{
	if ((ev->kq_fd = kqueue()) == -1) {
		printf("Kqueue failed\n");
		return 0;
	}

	ev->events = malloc(sizeof(struct kevent) * (*ev->basemem * 2));
	memset(ev->events, 0, sizeof(struct kevent) * (*ev->basemem * 2));
	
	ev->add = event_kqueue_add;
	ev->poll = event_kqueue_poll;
	ev->get_current_fd = event_kqueue_get_fd;
	ev->growup = event_kqueue_growup;
	ev->revent = event_kqueue_revent;
	ev->reload = event_kqueue_reload;
	
	printf("Kqueue succeed\n");
	
	return 1;
}

#else
int event_kqueue_init(struct _fdevent *ev)
{
	return 0;
}
#endif


