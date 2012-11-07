#ifndef __APE_EVENTS_H_
#define __APE_EVENTS_H_

#include "common.h"

#ifdef USE_KQUEUE_HANDLER
#include <sys/event.h>
#endif
#ifdef USE_EPOLL_HANDLER
#include <sys/epoll.h>
#endif
#ifdef USE_SELECT_HANDLER
  #include <WinSock2.h>
  #ifndef FD_SETSIZE
    #define FD_SETSIZE 1024
  #endif
#endif

/* Generics flags */
#define EVENT_READ 0x01
#define EVENT_WRITE 0x02
#define EVENT_LEVEL 0x04
#define EVENT_CANCEL 0x08

#define _APE_FD_DELEGATE_TPL  \
    ape_fds s; \
    void (*on_io)(int fd, int ev, ape_global *ape);

typedef enum {
    EVENT_UNKNOWN,
    EVENT_EPOLL,    /* Linux */
    EVENT_KQUEUE,   /* BSD */
    EVENT_DEVPOLL,  /* Solaris */
    EVENT_POLL,     /* POSIX */
    EVENT_SELECT    /* Generic (Windows) */
} fdevent_handler_t;

typedef enum {
    APE_SOCKET,
    APE_FILE,
    APE_DELEGATE
} ape_fds_t;

typedef struct { /* Do not store this. Address may changes */
    int fd;
    ape_fds_t type;
} ape_fds;

struct _ape_fd_delegate {
    _APE_FD_DELEGATE_TPL
};

#ifdef USE_SELECT_HANDLER
typedef struct {
  int  fd;
  char read:4;          /* bitmask */
  char write:4;         /* bitmask */
  void *ptr;
} select_fd_t;
#endif

struct _fdevent {
    /* Common values */
    int *basemem;

    /* Interface */
    int (*add)      (struct _fdevent *, int, int, void *);
    int (*del)      (struct _fdevent *, int);
    int (*poll)     (struct _fdevent *, int);
    int (*revent)   (struct _fdevent *, int);
    int (*reload)   (struct _fdevent *);
    void (*growup)  (struct _fdevent *);
    void *(*get_current_fd) (struct _fdevent *, int);

    /* Specifics values */
#ifdef USE_KQUEUE_HANDLER
    struct kevent *events;
    int kq_fd;
#endif
#ifdef USE_EPOLL_HANDLER
    struct epoll_event *events;
    int epoll_fd;
#endif
#ifdef USE_SELECT_HANDLER
	select_fd_t fds[FD_SETSIZE];
	select_fd_t **events; /* Pointers into fds */
#endif
    fdevent_handler_t handler;
};

int events_init(ape_global *ape);
int events_add(int fd, void *attach, int bitadd, ape_global *ape);
int events_del(int fd, ape_global *ape);
void *events_get_current_fd(struct _fdevent *ev, int i);
int events_poll(struct _fdevent *ev, int timeout_ms);

int event_kqueue_init(struct _fdevent *ev);
int event_epoll_init(struct _fdevent *ev);
int event_select_init(struct _fdevent *ev);
int events_revent(struct _fdevent *ev, int i);
#endif

// vim: ts=4 sts=4 sw=4 et

