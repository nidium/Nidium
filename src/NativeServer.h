#ifndef nativeserver_h__
#define nativeserver_h__

#include <stdlib.h>
#include <map>

/* Check if we can use setproctitle().
 * BSD systems have support for it, we provide an implementation for
 * Linux and osx. */
#if (defined __NetBSD__ || defined __FreeBSD__ || defined __OpenBSD__)
#define USE_SETPROCTITLE
#endif

#if ((defined __linux && defined(__GLIBC__)) || defined __APPLE__)
#define USE_SETPROCTITLE
#define INIT_SETPROCTITLE_REPLACEMENT
#ifdef __cplusplus
extern "C" {
#endif
void spt_init(int argc, char *argv[]);
void setproctitle(const char *fmt, ...);
#ifdef __cplusplus
}
#endif
#endif

class NativeServer
{
public:
    static int Start(int argc, char **argv);
private:
    NativeServer(int argc, char **argv);
    int init();
    void Usage(struct option * long_options, const char ** text_blocks);

    void daemonize(int pidfile = 0);
    int initWorker(int *idx);
    void wait();
    void displayVersion();

    struct {
        int argc;
        char **argv;
    } m_Args;

    int m_WorkerIdx;

    std::map<pid_t, int> m_PidIdxMapper;

    char *m_InstanceName;
    bool m_HasREPL;
    bool m_JSStrictMode;
    int m_NWorkers;
};

class NativeWorker
{
public:
     NativeWorker(int idx, bool repl = false);
    ~NativeWorker();
    int run(int argc, char **argv, bool jsstrict = false);

    int getIdentifier() const {
        return m_Idx;
    }
private:
    int m_Idx;
    bool m_RunREPL;
};

#endif

