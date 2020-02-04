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
    int sockfd;
    //struct epoll_event ev;
    struct epoll_event evlist[MAX_EVENTS];

    getArguments(&args, argc, argv);
        
    if((epfd = epoll_create1(0)) < 0)
        onError("epoll");

    sockfd = createSocket(args.port);

    epollPush(epfd, sockfd, EPOLLIN | EPOLLET);
    if ((listen(sockfd, 5)) < 0) 
        onError("listen");

    while(1) {
		int n, i;
		n = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
		for (i = 0; i < n; i++) {
			if (evlist[i].events & EPOLLERR || evlist[i].events & EPOLLHUP || !(evlist[i].events & EPOLLIN))
            {
				/* An error on this fd or socket not ready */
				perror("epoll error");
				close(evlist[i].data.fd);
			} 
            else if (evlist[i].data.fd == sockfd) 
            {
				/* New incoming connection */
				acceptAddConnection(sockfd, epfd);
			} 
            else 
            {
				/* Data incoming on fd */
				onIncomingData(evlist[i].data.fd);
			}
		}
	}

    free(args.filePrefix);
    close(sockfd);
    _exit(EXIT_SUCCESS);
}
