#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "NativeServer.h"

int main(int argc, char **argv)
{
    return NativeServer::Start(argc, argv);
}
