#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Server/Server.h"

int main(int argc, char **argv)
{
    return Nidium::Server::Server::Start(argc, argv);
}

