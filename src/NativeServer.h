#ifndef nativeserver_h__
#define nativeserver_h__

#include <stdlib.h>

class NativeServer
{
public:
    static int Start(int argc, char **argv);
    static void Daemonize(int pidfile = 0);
};

#endif