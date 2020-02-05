#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include "Massivereader.h"


void onError(char* message)
{
    printf("%s error\n", message);
    _exit(EXIT_FAILURE);
}

//---------------------------------------

void getArguments(struct Arguments* args, int argc, char** argv)
{
    int readArgs = 0;
    int opt;
	while((opt = getopt(argc, argv, "O:")) != -1)
	{
		switch(opt)
		{
			case 'O':
            {
                args->filePrefix = (char*)malloc(strlen(optarg)+1);
                if(args->filePrefix == NULL)
                    onError("Memory allocation");
                strcpy(args->filePrefix, optarg);
                readArgs++;
                break;
            }
			default: 
				printf("WRONG ARGUMENTS!\n");
				_exit(EXIT_FAILURE);
		}
	}
    if(optind >= argc || !readArgs)
    {
        printf("To few arguments!\n");
        _exit(EXIT_FAILURE);
    }
    char* p;
    args->port = strtol(argv[optind], &p, 10);    
    if(*p != '\0')
    {
        printf("Port argument must be an integer!\n");
        _exit(EXIT_FAILURE);
    }
    if(optind+1 < argc)
    {
        printf("Unrecognised arguments were specified!\n");
        _exit(EXIT_FAILURE);
    }
}

//---------------------------------------

int createServer(int port)
{
    struct sockaddr_in servaddr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        onError("socket");

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port=htons(port);
 
    if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
        onError("bind");

    makeNonBlock(sockfd);

    return sockfd;
}

//---------------------------------------

void epollPush(int epollfd, int socketfd, int flags)
{
    struct epoll_event event;
    event.data.fd = socketfd;
	event.events = flags;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &event) < 0)
        onError("epoll_ctl");
}

//---------------------------------------

void acceptAddConnection(int sockfd, int epfd)
{
	struct sockaddr newAddr;
	socklen_t newAddrlen = sizeof(newAddr);
	int infd;
	if((infd = accept(sockfd, &newAddr, &newAddrlen)) < 0)
        onError("accept");

    makeNonBlock(infd);

	epollPush(epfd, infd, EPOLLIN | EPOLLET);    
}

//---------------------------------------

void onIncomingData(int fd, int epollfd)
{
    while(1)
    {
        struct sockaddr_un addr;
	    if(read(fd, &addr, sizeof(struct sockaddr_un)) != sizeof(struct sockaddr_un))
            break;
        int newSockfd;
        if((newSockfd = connectSocket(addr)))
        {
            epollPush(epollfd, newSockfd, EPOLLIN | EPOLLET);
            write(fd, &addr, sizeof(struct sockaddr_un));
            printf("connected to local server\n");
        }
        else 
        {
            addr.sun_family = -1;
            write(fd, &addr, sizeof(struct sockaddr_un));
            printf("couldn't connect to local server \n%s\n", addr.sun_path + 1);
        }
    }
}

//---------------------------------------

void makeNonBlock(int sockfd)
{
    int flags;
    if ((flags = fcntl(sockfd, F_GETFL, 0)) < 0)
        onError("fcntl getfl");

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
        onError("fcntl setfl");
}

//---------------------------------------

int connectSocket(struct sockaddr_un addr)
{
    int socketfd;
    if((socketfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
        onError("socket");

    if(connect(socketfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) < 0)
    {
        printf("%d\n", errno);
        return 0;
    }

    makeNonBlock(socketfd);
    return socketfd;
}
