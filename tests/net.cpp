#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <native_netlib.h>
#include <mach/mach_time.h>
#include <unistd.h>

int ape_running = 1;
uint64_t tstart, tend;
uint64_t tstart2, tend2;
int x = 0;

void test_onread(ape_socket *s, ape_global *ape)
{
    fwrite(s->data_in.data, 1, s->data_in.size-1, stdout);
    printf("\n");
}

void test_onconnect(ape_socket *s, ape_global *ape)
{
    printf("It's connected\n");
    APE_socket_write(s, (unsigned char *)"GET / HTTP/1.1\nHost: google.com\n\n",
                     strlen("GET / HTTP/1.1\nHost: google.com\n\n"),
                     APE_DATA_COPY);
}

void test_async(ape_socket *s, int *last)
{
    s->callbacks.on_connected = test_onconnect;
    s->callbacks.on_read = test_onread;
    
    APE_socket_connect(s, 80, "91.121.5.68");
}

int test_timer(void *arg)
{
    ape_global *ape = (ape_global *)arg;
    tend = mach_absolute_time();
    sleep(1);
    printf("Exec timer %lld\n", (tend-tstart)/1000000);
    tstart = tend;
    if (++x%50 == 0) timers_stats_print(&ape->timersng);

    return 500;
}

int test_timer2(void *arg)
{
    ape_global *ape = (ape_global *)arg;
    tend2 = mach_absolute_time();

    printf("Exec timer 2 %lld\n", (tend2-tstart2)/1000000);
    tstart2 = tend2;
    if (++x%50 == 0) timers_stats_print(&ape->timersng);

    return 356;
}


static void *NativeRunNetworkThread(void *arg)
{
	#define NCONNECT 5
    ape_global *ape = (ape_global *)arg;
    ape_socket *s[NCONNECT];
    int i;
    
    for (i = 0; i < NCONNECT; i++) {
        s[i] = APE_socket_new(APE_SOCKET_PT_TCP, 0, ape);
        add_timeout(1, (void *)test_async, s[i], ape);
    }
    tstart = mach_absolute_time();
    tstart2 = tstart;
    add_timer(&ape->timersng, 500, test_timer2, ape);
    add_timer(&ape->timersng, 500, test_timer, ape);
    
    events_loop(ape);
    
    return NULL;
}

int main(int argc, char *argv[])
{
	ape_global *net = native_netlib_init();
	NativeRunNetworkThread(net);

	return 1;
}
