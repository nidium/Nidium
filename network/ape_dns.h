#ifndef __APE_DNS_H_
#define __APE_DNS_H_

#define _HAS_ARES_SUPPORT

#include "common.h"

typedef int (*ape_gethostbyname_callback)(const char *ip, void *arg,
        int status);


struct _ares_sockets {
    _APE_FD_DELEGATE_TPL
};

typedef struct _ape_dns_cb_argv {
    ape_global *ape;
    ape_gethostbyname_callback callback;
    const char *origin;
    void *arg;
    int invalidate;
} ape_dns_state;

void ape_dns_invalidate(ape_dns_state *state);
int ape_dns_init(ape_global *ape);

ape_dns_state *ape_gethostbyname(const char *host, ape_gethostbyname_callback callback,
        void *arg, ape_global *ape);

#endif

// vim: ts=4 sts=4 sw=4 et

