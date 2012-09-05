#include "common.h"
#include "ape_events.h"
#include "ape_socket.h"

#include <stdlib.h>


int events_add(int fd, void *attach, int bitadd, ape_global *ape)
{
    struct _fdevent *ev = &ape->events;

    if (ev->add(ev, fd, bitadd, attach) == -1) {
        return -1;
    }

    return 1;
}

int events_del(int fd, ape_global *ape)
{
    struct _fdevent *ev = &ape->events;

    if (ev->del(ev, fd) == -1) {
        return -1;
    }

    return 1;
}

inline int events_poll(struct _fdevent *ev, int timeout_ms)
{
    int nfds;

    if ((nfds = ev->poll(ev, timeout_ms)) == -1) {
        return -1;
    }

    return nfds;
}


inline void *events_get_current_fd(struct _fdevent *ev, int i)
{
    return ev->get_current_fd(ev, i);
}

/*
void events_growup(struct _fdevent *ev)
{
    ev->growup(ev);
}
*/
int events_revent(struct _fdevent *ev, int i)
{
    return ev->revent(ev, i);
}

/*
int events_reload(struct _fdevent *ev)
{
    return ev->reload(ev);
}
*/

int events_init(ape_global *ape)
{
    ape->events.basemem = &ape->basemem;

    switch(ape->events.handler) {
        case EVENT_EPOLL:
            return event_epoll_init(&ape->events);
            break;
        case EVENT_KQUEUE:
            return event_kqueue_init(&ape->events);
            break;
        default:
            break;
    }

    return -1;
}

// vim: ts=4 sts=4 sw=4 et

