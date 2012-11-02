#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <native_netlib.h>

#include <unistd.h>
#include <assert.h>

int ape_running = 1;
int x = 0;

void test_onread(ape_socket *s, ape_global *ape)
{
    assert(s->data_in.used == 4);
    assert(strncmp("foo\n", (const char *)s->data_in.data, 4) == 0);
    exit(1);
}

void test_onconnect(ape_socket *s, ape_global *ape)
{
    APE_socket_write(s, (unsigned char *)"hello\n",
                     strlen("hello\n"),
                     APE_DATA_STATIC);
}

int test_async(void *arg)
{
    ape_socket *s = (ape_socket *)arg;
    s->callbacks.on_connected = test_onconnect;
    s->callbacks.on_read = test_onread;
    
    APE_socket_connect(s, 9988, "127.0.0.1");

    return 0;
}

static void client_connect(ape_socket *server,
    ape_socket *client, ape_global *net)
{
    APE_socket_write(client, (unsigned char *)"foo\n", 4, APE_DATA_STATIC);
    APE_socket_shutdown(client);
}

static void create_server(ape_global *net)
{
    ape_socket *s = APE_socket_new(APE_SOCKET_PT_TCP, 0, net);
    s->callbacks.on_connect = client_connect;
    APE_socket_listen(s, 9988, "127.0.0.1");

}

static void *NativeRunNetworkThread(void *arg)
{
    #define NCONNECT 1
    ape_global *ape = (ape_global *)arg;
    ape_socket *s[NCONNECT];
    int i;
    
    for (i = 0; i < NCONNECT; i++) {
        s[i] = APE_socket_new(APE_SOCKET_PT_TCP, 0, ape);
        add_timer(&ape->timersng, 1, test_async, s[i]);
    }

    events_loop(ape);
    
    return NULL;
}

int main(int argc, char *argv[])
{
    ape_global *net = native_netlib_init();
    create_server(net);
    NativeRunNetworkThread(net);

    return 1;
}
