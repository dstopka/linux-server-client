#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "Massivereader.h"

int main(int argc, char** argv)
{
    struct Arguments args;
    int epfd;
    int sockfd;//, len, n;

    //struct epoll_event ev;
    //struct epoll_event evlist[MAX_EVENTS];
    getArguments(&args, argc, argv);
    createSocket(&sockfd, args.port);
    if ((listen(sockfd, 5)) < 0) 
    { 
       printf("listen error\t%d\n", errno);
        _exit(EXIT_FAILURE);
    }
    if((epfd = epoll_create1(0)) < 0)
    {
        printf("epoll error\n");
        _exit(EXIT_FAILURE);
    } 

    while(1){}

    _exit(EXIT_SUCCESS);
}
