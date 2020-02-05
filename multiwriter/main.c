#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "Multiwriter.h"


int main(int argc, char** argv)
{
    struct sockaddr_un servaddr;
    struct Arguments args;
    int serverfd;
    int inetfd;

    getArguments(&args, &argc, &argv);
    servaddr = randomAddr();
    serverfd = createServer(&servaddr);
    inetfd = createClient(args.port);

    for(int i; i < args.connectionsNumber; ++i)
        write(inetfd, &servaddr, sizeof(struct sockaddr_un));

    while (1) {
        if(serverfd)
        {}
    }

    _exit(EXIT_SUCCESS);
}

