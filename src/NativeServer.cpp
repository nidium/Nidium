#define _HAVE_SSL_SUPPORT 1
#include <native_netlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#ifdef __linux__
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "NativeServer.h"
#include "NativeContext.h"
#include "NativeMacros.h"
#include "NativeREPL.h"

#include <NativeJSProcess.h>

#define NATIVE_MAX_WORKERS 64

int ape_running = 1;
unsigned long _ape_seed;

static void signal_handler(int sign)
{
    ape_running = 0;
    NLOG("Signal %d received, shutting down...", sign);
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

int NativeServer::initWorker(int argc, char **argv, int idx)
{
    if (fork() != 0) {
        NativeWorker worker(idx);
        worker.run(argc, argv);

        return 1;
    }

    return 0;
}

int NativeServer::Start(int argc, char *argv[])
{
    bool daemon = false;
    int workers = 1;

    static struct option long_options[] =
    {
        {"daemon",     no_argument,       0, 'd'},
        {"workers",    required_argument, 0, 'w'},
        {0, 0, 0, 0}
    };

    _ape_seed = time(NULL) ^ (getpid() << 16);

    signal(SIGINT, &signal_handler);
    signal(SIGTERM, &signal_handler);
    signal(SIGQUIT, &signal_handler);

    int ch;
    //opterr = 0;

    /*
        Needed on macosx so that arguments doesn't fail after the .js file
    */
    setenv("POSIXLY_CORRECT", "1", 1);

    while ((ch = getopt_long(argc, argv, "dw:", long_options, NULL)) != -1) {
        //printf("Got %c (%s)\n", ch, optarg);
        switch (ch) {
            case 'd':
                daemon = true;
                break;
            case '?':
                exit(1);
                break;
            case 'w':
                workers = atoi(optarg);
                break;
            default:
                break;            
        }
    }

    if (workers > NATIVE_MAX_WORKERS) {
        fprintf(stderr, "[Error] Too many worker requested : max %d\n", NATIVE_MAX_WORKERS);
        exit(1);
    }

    if (workers) {
        for (int i = 0; i < workers; i++) {
            if (NativeServer::initWorker(argc, argv, i+1) == 1) {
                break;
            }
        }
    }

    return 1;
}

NativeWorker::NativeWorker(int idx) : 
    m_Idx(idx)
{

}

NativeWorker::~NativeWorker()
{

}

int NativeWorker::run(int argc, char **argv)
{
    NativeREPL *repl = NULL;
    ape_global *net = native_netlib_init();

    inc_rlimit(64000);

    signal(SIGPIPE, SIG_IGN);

    NativeContext ctx(net, this);
    const NativeJS *js = ctx.getNJS();
    NativeJSProcess::registerObject(js->getJSContext(), argv, argc);

    /*
        Daemon requires a .js to load
    */
    if (0) {
        if (!ctx.getNJS()->LoadScript(argv[argc-1])) {
            return 1;
        }
        NativeServer::Daemonize();
    } else {
#ifdef DEBUG
        printf("[Warn] Running in Debug mode\n");
#endif
        if (argc > 1) {
            ctx.getNJS()->LoadScript(argv[argc-1]);
        }

        /* Heap allocated because we need to be
        sure that it's deleted before NativeJS */
        //repl = new NativeREPL(ctx.getNJS());
    }

    events_loop(net);

    if (repl) {
        delete repl;
    }

    return 0;
}