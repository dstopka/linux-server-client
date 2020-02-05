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
    struct Connections connections = {NULL, 0, 0};
    struct Arguments args;
    int serverfd;
    int inetfd;
    int epfd;

    getArguments(&args, argc, argv);
    connections.connectedSockets = (int*)malloc(args.connectionsNumber*sizeof(int));
    if(connections.connectedSockets == NULL)
        onError("memory allocation");
    int* nextSocket = connections.connectedSockets;
    servaddr = randomAddr();
    serverfd = createServer(&servaddr);
    inetfd = createClient(args.port);
    if((epfd = epoll_create1(0)) < 0)
        onError("epoll");
    epollPush(epfd, serverfd, EPOLLIN | EPOLLET);
    epollPush(epfd, inetfd, EPOLLIN | EPOLLET);

    for(int i; i < args.connectionsNumber; ++i)
        write(inetfd, &servaddr, sizeof(struct sockaddr_un));

    while(connections.connectedNo + connections.rejectedNo < args.connectionsNumber) 
    {
		int readyEventsNumber;
		readyEventsNumber = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
		for (int i = 0; i < readyEventsNumber; ++i) {
			if (evlist[i].events & EPOLLERR || evlist[i].events & EPOLLHUP || !(evlist[i].events & EPOLLIN))
            {
                if (epoll_ctl(epfd, EPOLL_CTL_DEL, evlist[i].data.fd, NULL) < 0)
                    onError("epoll_ctl delete");
                for(int i = 0; i < args.connectionsNumber; ++i)
                    if(connections.connectedSockets[i] == evlist[i].data.fd)
                        connections.connectedSockets[i] = 0;
				close(evlist[i].data.fd);
			} 
            else if (evlist[i].data.fd == serverfd) 
            {
				acceptConnection(serverfd, &nextSocket, epfd);
			} 
            else if (evlist[i].data.fd == inetfd)
            {
				onIncomingData(inetfd, &connections);
			}
		}
	}
    if (epoll_ctl(epfd, EPOLL_CTL_DEL, inetfd, NULL) < 0)
        onError("epoll_ctl delete");
    close(inetfd);
    printf("%s\n", timeToStr());

    free(connections.connectedSockets);
    if(close(serverfd) < 0)
        onError("close");
    if(close(epfd) < 0)
        onError("close");
    _exit(EXIT_SUCCESS);
}

