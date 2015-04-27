#ifndef nativeserver_h__
#define nativeserver_h__

#include <stdlib.h>
#include <map>

class NativeServer
{
public:
    static int Start(int argc, char **argv);
private:
    NativeServer(int argc, char **argv);
    int init();

    void daemonize(int pidfile = 0);
    int initWorker(int *idx);
    void wait();

    struct {
        int argc;
        char **argv;
    } m_Args;

    int m_WorkerIdx;

    std::map<pid_t, int> m_PidIdxMapper;
};

class NativeWorker
{
public:
     NativeWorker(int idx);
    ~NativeWorker();
    int run(int argc, char **argv);

    int getIdentifier() const {
        return m_Idx;
    }
private:
    int m_Idx;
};

#endif