#ifndef nativeserver_h__
#define nativeserver_h__

#include <stdlib.h>

class NativeServer
{
public:
    static int Start(int argc, char **argv);
    static void Daemonize(int pidfile = 0);
    static int initWorker(int argc, char **argv, int idx);
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