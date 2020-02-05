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
    struct epoll_event evlist[MAX_EVENTS];
    struct Arguments args;
    int serverfd;
    int inetfd;
    int epfd;
    int* connectedSockets;

    getArguments(&args, &argc, &argv);
    connectedSockets = (int*)malloc(args.connectionsNumber*sizeof(int));
    int* nextSocket = connectedSockets;
    servaddr = randomAddr();
    serverfd = createServer(&servaddr);
    inetfd = createClient(args.port);
    if((epfd = epoll_create1(0)) < 0)
        onError("epoll");
    epollPush(epfd, serverfd, EPOLLIN | EPOLLET);
    epollPush(epfd, inetfd, EPOLLIN | EPOLLET);

    for(int i; i < args.connectionsNumber; ++i)
        write(inetfd, &servaddr, sizeof(struct sockaddr_un));

    while(1) 
    {
		int readyEventsNumber;
		readyEventsNumber = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
		for (int i = 0; i < readyEventsNumber; ++i) {
			if (evlist[i].events & EPOLLERR || evlist[i].events & EPOLLHUP || !(evlist[i].events & EPOLLIN))
            {
				perror("epoll error");
				close(evlist[i].data.fd);
			} 
            else if (evlist[i].data.fd == serverfd) 
            {
				acceptConnection(serverfd, nextSocket);
			} 
            else if (evlist[i].data.fd == inetfd)
            {
				onIncomingData(inetfd);
			}
		}
	}

    _exit(EXIT_SUCCESS);
}

