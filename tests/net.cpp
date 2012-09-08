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
    APE_socket_write(s, (unsigned char *)"GET / HTTP/1.0\nHost: www.google.fr\n\n",
                     strlen("GET / HTTP/1.0\nHost: www.google.fr\n\n"),
                     APE_DATA_COPY);
}

int test_async(void *arg)
{
    ape_socket *s = (ape_socket *)arg;
    s->callbacks.on_connected = test_onconnect;
    s->callbacks.on_read = test_onread;
    
    APE_socket_connect(s, 80, "173.194.67.94");

    return 0;
}

int test_timer(void *arg)
{
    ape_global *ape = (ape_global *)arg;
    tend = mach_absolute_time();
    //usleep(500000);
    printf("Exec timer %lld\n", (tend-tstart)/1000000);
    tstart = tend;
    if (++x%10 == 0) timers_stats_print(&ape->timersng);

    return 5000;
}

int test_timer2(void *arg)
{
    ape_global *ape = (ape_global *)arg;
    tend2 = mach_absolute_time();

    printf("Exec timer 2 %lld\n", (tend2-tstart2)/1000000);
    tstart2 = tend2;
    //if (++x%50 == 0) timers_stats_print(&ape->timersng);

    return 356;
}

int test_async_msg(void *arg)
{
    printf("test 2\n");
    return 100;
}

static void *NativeRunNetworkThread(void *arg)
{
	#define NCONNECT 0
    ape_global *ape = (ape_global *)arg;
    ape_socket *s[NCONNECT];
    int i;
    
    for (i = 0; i < NCONNECT; i++) {
        s[i] = APE_socket_new(APE_SOCKET_PT_TCP, 0, ape);
        add_timer(&ape->timersng, 1, test_async, s[i]);
    }
    tstart = mach_absolute_time();
    tstart2 = tstart;
    //add_timer(&ape->timersng, 277, test_timer2, ape);
    add_timer(&ape->timersng, 500, test_timer, ape);
    //timer_dispatch_async(test_timer, NULL);
    printf("Test 1\n");
    events_loop(ape);
    
    return NULL;
}

int main(int argc, char *argv[])
{
	ape_global *net = native_netlib_init();
	NativeRunNetworkThread(net);

	return 1;
}
