/* event_kqueue.c */

#include "common.h"
#include "ape_events.h"
#ifndef __WIN32
#include <sys/time.h>
#include <unistd.h>
#endif
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "ape_socket.h"

#ifdef USE_SELECT_HANDLER

#ifndef MIN_TIMEOUT_MS
# ifdef DEBUG_EVENT_SELECT
#  define MIN_TIMEOUT_MS        1
# else
#  define MIN_TIMEOUT_MS        150
# endif
#endif

typedef enum
{
  evsb_none             = 0,
  evsb_added            = 1,    /**< descriptor has been added to list */
  evsb_ready            = 2,    /**< descriptor is ready to read or write */
  evsb_writeWatch       = 4     /**< descriptor should be watched for writes */
} evsb_bit_t;   /**< event status bits; must fit in a nibble */

static int event_select_add(struct _fdevent *ev, int fd, int bitadd,
        void *attach)
{
	printf("Adding %d to list %d\n", fd, FD_SETSIZE);
	if (fd < 0 || fd > FD_SETSIZE) {
		printf("cant add event\n");
		return -1;
	}
  
	if (bitadd & EVENT_READ) 
		ev->fds[fd].read |= evsb_added;

	if (bitadd & EVENT_WRITE) 
		ev->fds[fd].write |= evsb_added;

	ev->fds[fd].fd = fd;
	ev->fds[fd].ptr = attach;
	printf("[++++] added %d\n", fd);


	return 1;
}

static int event_select_del(struct _fdevent *ev, int fd)
{

	ev->fds[fd].read = 0;
	ev->fds[fd].write = 0;

    return 1;
}

static int event_select_poll(struct _fdevent *ev, int timeout_ms)
{
  struct timeval        tv;
  int                   fd, i, maxfd, numfds;
  fd_set                rfds, wfds;

  if (timeout_ms < MIN_TIMEOUT_MS)
    timeout_ms = MIN_TIMEOUT_MS;

  tv.tv_sec = timeout_ms / 1000;
  tv.tv_usec = (timeout_ms % 1000) * 1000;

  FD_ZERO(&rfds);
  FD_ZERO(&wfds);

  for (fd=0, maxfd=0; fd < FD_SETSIZE; fd++)
  {
    if (ev->fds[fd].read) {
      FD_SET(fd, &rfds);
      printf("[----] WATCH read on %d\n", fd);
    }
    if (ev->fds[fd].write & evsb_writeWatch) {
      printf("[----] WATCH WRITE on %d\n", fd);
      FD_SET(fd, &wfds);
    }
    if (ev->fds[fd].read || ev->fds[fd].write & evsb_writeWatch)
    {
      if (fd > maxfd)
        maxfd = fd;
    }

  }

  errno = 0;
  numfds = select(maxfd + 1, &rfds, &wfds, NULL, &tv);
  switch(numfds)
  {
    case -1:
      //fprintf(stderr, "Error calling select: %s\n", strerror(errno));
    case 0:
      return numfds;
  }
  printf("passed\n");
  /* Mark pending data */
  for (fd=0; fd <= maxfd; fd++)
  {
    if (FD_ISSET(fd, &rfds)) {
      ev->fds[fd].read |= evsb_ready;
	  printf("ready for read\n");
	}
    else
      ev->fds[fd].read &= ~evsb_ready;

    if (FD_ISSET(fd, &wfds)) {
      ev->fds[fd].write |= evsb_ready;
      printf("[++] Write is ready in select()\n\n");
    }
    else
    {
      ev->fds[fd].write &= ~evsb_ready;
    }
  }
  
  /* Create the events array for event_select_revent et al */
  for (fd=0, i=0; fd <= maxfd; fd++)
  {
    if (FD_ISSET(fd, &rfds) || FD_ISSET(fd, &wfds))
    {
      ev->events[i++] = &ev->fds[fd];
    }
  }
  if (i > 0) printf("got something %d\n", i);
  return i;
}

static void *event_select_get_fd(struct _fdevent *ev, int i)
{
	return ev->events[i]->ptr;
}

static void event_select_growup(struct _fdevent *ev)
{

}

static int event_select_revent(struct _fdevent *ev, int i)
{
	int bitret = 0;
	int fd = ev->events[i]->fd;

	if (ev->fds[fd].read & evsb_ready)
	bitret |= EVENT_READ;

	if (ev->fds[fd].write & evsb_ready)
	bitret |= EVENT_WRITE;

	ev->fds[fd].read &= evsb_added;       /* clear ready */
	ev->fds[fd].write &= evsb_added | evsb_writeWatch;    /* clear ready */

	return bitret;
}


int event_select_reload(struct _fdevent *ev)
{


    return 1;
}

int event_select_init(struct _fdevent *ev)
{

	ev->events = malloc(sizeof(*ev->events) * (*ev->basemem));
	memset(ev->fds, 0, sizeof(ev->fds));

	ev->add               = event_select_add;
	ev->poll              = event_select_poll;
	ev->get_current_fd    = event_select_get_fd;
	ev->growup            = event_select_growup;
	ev->revent            = event_select_revent;
	ev->reload            = event_select_reload;

    return 1;
}

#else
int event_epoll_init(struct _fdevent *ev)
{
    return 0;
}
#endif

// vim: ts=4 sts=4 sw=4 et

