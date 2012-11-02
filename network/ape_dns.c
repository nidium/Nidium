#ifdef _MSC_VER
  #include <ares.h>
  #include <WinSock2.h>
  #include <io.h>
#else
  #include "../c-ares/ares.h"
  #include <netdb.h>
  #include <unistd.h>
  #include <arpa/inet.h>
#endif

#include <stdlib.h>
#include "common.h"
#include "ape_dns.h"
#include "ape_events.h"

#include <stdio.h>

#include <fcntl.h>

/* gcc *.c -I../deps/ ../deps/c-ares/.libs/libcares.a -lrt */

#ifdef FIONBIO
static __inline int setnonblocking(int fd)
{
    int  ret = 1;
#ifdef _MSC_VER
	return ioctlsocket(fd, FIONBIO, &ret);
    #define close _close
#else
    return ioctl(fd, FIONBIO, &ret);
#endif
}
#else
#define setnonblocking(fd) fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)
#endif

struct _ape_dns_cb_argv {
    ape_global *ape;
    ape_gethostbyname_callback callback;
    const char *origin;
    void *arg;
};

static void ares_io(int fd, int ev, ape_global *ape)
{
    ares_process_fd(ape->dns.channel,
        (ev & EVENT_READ ? fd : ARES_SOCKET_BAD),
        (ev & EVENT_WRITE ? fd : ARES_SOCKET_BAD));
}

static void ares_socket_cb(void *data, int s, int read, int write)
{
    ape_global *ape = data;
    int i, f = 0;

    for (i = 0; i < ape->dns.sockets.size; i++) {
        if (!f && ape->dns.sockets.list[i].s.fd == 0) {
            f = i;
        } else if (ape->dns.sockets.list[i].s.fd == s) {
            /* Modify or delete the object (+ return) */
            if (read == 0 && write == 0) {
                ape->dns.sockets.list[i].s.fd = 0;
                close(s);
            }
            return;
        }
    }
    setnonblocking(s);
    ape->dns.sockets.list[f].s.fd   = s;
    ape->dns.sockets.list[f].s.type = APE_DELEGATE;
    ape->dns.sockets.list[f].on_io  = ares_io;

    events_add(s, &ape->dns.sockets.list[f], EVENT_READ|EVENT_WRITE|EVENT_LEVEL, ape);
}

int ape_dns_init(ape_global *ape)
{
    struct ares_options opt;
    int ret;

    if (ares_library_init(ARES_LIB_INIT_ALL) != 0) {
        return -1;
    }

    opt.sock_state_cb   = ares_socket_cb;
    opt.sock_state_cb_data  = ape;

    /* At the moment we only use one dns channel */
    if ((ret = ares_init_options(&ape->dns.channel, &opt,
                    0x00 | ARES_OPT_SOCK_STATE_CB)) != ARES_SUCCESS) {

        return -1;
    }
    ape->dns.sockets.list   = calloc(32, sizeof(struct _ares_sockets) * 32);
    ape->dns.sockets.size   = 32;
    ape->dns.sockets.used   = 0;

    return 0;
}

void ares_gethostbyname_cb(void *arg, int status,
        int timeout, struct hostent *host)
{
    struct _ape_dns_cb_argv *params = arg;
    char ret[46];

    if (status == ARES_SUCCESS) {
        /* only return the first h_addr_list element */
        inet_ntop(host->h_addrtype, *host->h_addr_list, ret, sizeof(ret));
        params->callback(ret, params->arg, status);
    } else {
        params->callback(NULL, params->arg, status);
    }

    free(params);
}

void ape_gethostbyname(const char *host, ape_gethostbyname_callback callback,
        void *arg, ape_global *ape)
{
    struct in_addr addr4;

    if (inet_pton(AF_INET, host, &addr4) == 1) {
        callback(host, arg, ARES_SUCCESS);
    } else {
        struct _ape_dns_cb_argv *cb     = malloc(sizeof(*cb));
        cb->ape             = ape;
        cb->callback            = callback;
        cb->origin          = host;
        cb->arg             = arg;

        ares_gethostbyname(ape->dns.channel, host,
                AF_INET, ares_gethostbyname_cb, cb);

    }
}

// vim: ts=4 sts=4 sw=4 et

