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
    int serverfd;
    struct epoll_event evlist[MAX_EVENTS];

    getArguments(&args, argc, argv);

    serverfd = createServer(args.port); 
    if((epfd = epoll_create1(0)) < 0)
        onError("epoll");
    epollPush(epfd, serverfd, EPOLLIN | EPOLLET);
    if ((listen(serverfd, 5)) < 0) 
        onError("listen");

    while(1) {
		int readyEventsNumber;
		readyEventsNumber = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
		for (int i = 0; i < readyEventsNumber; ++i) {
			if (evlist[i].events & EPOLLERR || evlist[i].events & EPOLLHUP || !(evlist[i].events & EPOLLIN))
            {
				/* An error on this fd or socket not ready */
				perror("epoll error");
				close(evlist[i].data.fd);
			} 
            else if (evlist[i].data.fd == serverfd) 
            {
				/* New incoming connection */
				acceptAddConnection(serverfd, epfd);
			} 
            else 
            {
				/* Data incoming on fd */
				onIncomingData(evlist[i].data.fd, epfd);
			}
		}
	}

    free(args.filePrefix);
    close(serverfd);
    close(epfd);
    _exit(EXIT_SUCCESS);
}
