#define _HAVE_SSL_SUPPORT 1

#include <native_netlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include "NativeServer.h"
#include "NativeContext.h"
#include "NativeMacros.h"
#include "NativeREPL.h"

int ape_running = 1;
unsigned long _ape_seed;

static void signal_handler(int sign)
{
    ape_running = 0;
    NLOG("[Quit] Shutting down...");
}

static int inc_rlimit(int nofile)
{
    struct rlimit rl;

    rl.rlim_cur = nofile;
    rl.rlim_max = nofile;

    return setrlimit(RLIMIT_NOFILE, &rl);
}

void NativeServer::Daemonize(int pidfile)
{
    if (0 != fork()) { 
        exit(0);
    }
    if (-1 == setsid()) {
        exit(0);
    }
    signal(SIGHUP, SIG_IGN);

    if (0 != fork()) {
        exit(0);
    }
    if (pidfile > 0) {
        char pidstring[32];
        int len;
        len = sprintf(pidstring, "%i", (int)getpid());
        write(pidfile, pidstring, len);
        close(pidfile);
    }
}

int NativeServer::Start(int argc, char *argv[])
{
    bool daemon = false;
    NativeREPL *repl = NULL;

    static struct option long_options[] =
    {
        {"daemon",     no_argument,       0, 'd'},
        {0, 0, 0, 0}
    };

    _ape_seed = time(NULL) ^ (getpid() << 16);
    ape_global *net = native_netlib_init();

    inc_rlimit(64000);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, &signal_handler);
    signal(SIGTERM, &signal_handler);

    int ch;

    while ((ch = getopt_long(argc, argv, "d", long_options, NULL)) != -1) {
        switch (ch) {
            case 'd':
                daemon = true;
                break;
            default:
                break;            
        }
    }

    NativeContext ctx(net);
    /*
        Daemon requires a .js to load
    */
    if (daemon) {
        if (!ctx.getNJS()->LoadScript(argv[argc-1])) {
            return 1;
        }
        NativeServer::Daemonize();
    } else {
        if (argc > 1) {
            ctx.getNJS()->LoadScript(argv[argc-1]);
        }

        /* Heap allocated because we need to be
        sure that it's deleted before NativeJS */
        repl = new NativeREPL(ctx.getNJS());
    }

    events_loop(net);

    delete repl;
    return 1;
}
