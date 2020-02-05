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
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include "Massivereader.h"

int flag = 0;

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

    if ((listen(sockfd, 5)) < 0) 
        onError("listen");

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
            if(write(fd, &addr, sizeof(struct sockaddr_un)) < 0)
                onError("write +");
            printf("connected to local server\n");
        }
        else 
        {
            addr.sun_family = -1;
            if(write(fd, &addr, sizeof(struct sockaddr_un)) < 0)
                onError("write +");
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

//---------------------------------------

void makeLog(struct Arguments* args, int* logfd)
{
    int fd = -1;
    char new_file_path[1024];
    
    while(fd == -1)
    {
        sprintf(new_file_path,"%s%03d", args->filePrefix, args->filesNo++);
        fd = open(new_file_path, O_WRONLY | O_CREAT | O_TRUNC);
    }

    if(close(*logfd) < 0)
        onError("close");
    *logfd = fd;
    flag = 0;
}

//---------------------------------------

void sigUsr1Handler()
{
    flag = 1;
}

//---------------------------------------

void setHandler()
{
    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = sigUsr1Handler;
    if(sigaction(SIGUSR1, &sa, NULL) == -1)
        onError("sigaction");
}

//---------------------------------------

char* timeToStr()
{
    char* buff=(char*)malloc(20);
    if(buff == NULL)
        onError("memory allocation");
    buff[19] = 0;
    struct timespec t;
    struct tm* tm;
    long nanoseconds;

    if(clock_gettime(CLOCK_REALTIME, &t)<0)
        onError("gettime");
    if((tm = localtime(&t.tv_sec)) == NULL)
        onError("localtime");

    strftime(buff, sizeof buff, "%M*:%S,", tm);

    nanoseconds = t.tv_nsec;
    char nsec[10];
    nsec[9] = 0;
    for (int i = 0; i < 9; ++i) {
        nsec[8-i] = '0' + (char)(nanoseconds%10);
        nanoseconds /= 10;
    }

    strncpy(&buff[7], nsec, 2);
    strcat(buff, ".");
    strncpy(&buff[10], &nsec[2], 2);
    strcat(buff, ".");
    strncpy(&buff[13], &nsec[4], 2);
    strcat(buff, ".");
    strncpy(&buff[16], &nsec[6], 3);

    return buff;    
}