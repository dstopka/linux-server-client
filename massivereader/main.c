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
    struct Arguments args = {0, "", 0};
    int epfd;
    int serverfd;
    int logfd;
    struct epoll_event evlist[MAX_EVENTS];

    getArguments(&args, argc, argv);
    makeLog(&args, &logfd);
    serverfd = createServer(args.port); 
    if((epfd = epoll_create1(0)) < 0)
        onError("epoll");
    struct SocketData sockData;
    sockData.fd = serverfd;
    sockData.local = 0;
    epollPush(epfd, EPOLLIN | EPOLLET, &sockData);
    setHandler();

    while(1) 
    {
		int readyEventsNumber;
		readyEventsNumber = epoll_wait(epfd, evlist, MAX_EVENTS, -1);
		for (int i = 0; i < readyEventsNumber; ++i) 
        {
			if (evlist[i].events & EPOLLERR || evlist[i].events & EPOLLHUP || !(evlist[i].events & EPOLLIN))
            {
				if (epoll_ctl(epfd, EPOLL_CTL_DEL, ((struct SocketData*) (evlist[i].data.ptr))->fd, NULL) < 0)
                    onError("epoll_ctl delete");
				if(close(((struct SocketData*) (evlist[i].data.ptr))->fd) < 0)
                    onError("close");
			} 
            else if (((struct SocketData*) (evlist[i].data.ptr))->fd == serverfd) 
				acceptAddConnection(serverfd, epfd);
            else if(((struct SocketData*) (evlist[i].data.ptr))->local == 1)
				readLocalData(((struct SocketData*) (evlist[i].data.ptr)), logfd);
            else
                onIncomingData(((struct SocketData*) (evlist[i].data.ptr))->fd, epfd);
		}
        if(flag)
        {
            makeLog(&args, &logfd);
            flag = 0;
        }
	}

    free(args.filePrefix);
    if(close(serverfd) < 0)
        onError("close");
    if(close(epfd) < 0)
        onError("close");
    if(close(logfd) < 0)
        onError("close");
    _exit(EXIT_SUCCESS);
}
