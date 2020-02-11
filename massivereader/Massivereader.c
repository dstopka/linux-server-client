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
    printf("%s error\t%d\n", message, errno);
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

void epollPush(int epollfd, int flags, struct SocketData* sockData)
{
    struct epoll_event event;
    event.data.ptr = sockData;
	event.events = flags;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockData->fd, &event) < 0)
        onError("epoll_ctl");
}

//---------------------------------------

void acceptAddConnection(int sockfd, int epfd)
{
	struct sockaddr newAddr;
    struct SocketData sockData;
	socklen_t newAddrlen = sizeof(newAddr);
	int infd;
	if((infd = accept(sockfd, &newAddr, &newAddrlen)) < 0)
        onError("accept");
    sockData.local = 0;
    sockData.fd = infd;
    makeNonBlock(infd);
	epollPush(epfd, EPOLLIN | EPOLLET, &sockData);  
}

//---------------------------------------

void onIncomingData(int fd, int epollfd)
{
    while(1)
    {
        struct sockaddr_un addr;
        int readB;
	    if((readB = read(fd, &addr, sizeof(struct sockaddr_un))) != sizeof(struct sockaddr_un))
            break;
        int newSockfd;
        if((newSockfd = connectSocket(&addr)))
        {
            struct SocketData* sockData = (struct SocketData*)malloc(sizeof(struct SocketData));
            if(sockData == NULL)
                onError("memory allocation");
            sockData->fd = newSockfd;
            sockData->addr = addr;
            sockData->local = 1;
            epollPush(epollfd, EPOLLIN | EPOLLET, sockData);
            if(write(fd, &addr, sizeof(struct sockaddr_un)) < 0)
                onError("write +");
        }
        else 
        {
            addr.sun_family = -1;
            if(write(fd, &addr, sizeof(struct sockaddr_un)) < 0)
                onError("write +");
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

int connectSocket(struct sockaddr_un* addr)
{
    int socketfd;
    if((socketfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
        onError("socket");

    if(connect(socketfd, (struct sockaddr *)addr, sizeof(struct sockaddr_un)) < 0)
        onError("connect");

    //makeNonBlock(socketfd);
    return socketfd;
}

//---------------------------------------

void makeLog(struct Arguments* args, int* logfd)
{
    int fd = -1;
    char* file = (char *)malloc(strlen(args->filePrefix) + 4);
    if(file == NULL)
        onError("memory allocation");

    
    while(fd == -1)
    {
        sprintf(file,"%s%03d", args->filePrefix, args->filesNo++);
        fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    }

    if(close(*logfd) < 0)
        onError("close");
    *logfd = fd;
    flag = 0;
    free(file);
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

char* timeToStr(struct timespec time)
{
    //struct tm* tm;
    char* buff=(char*)malloc(20);
    if(buff == NULL)
        onError("memory allocation");
    buff[19] = 0;
    int minutes;
    int seconds;
    long nanoseconds;

    // Thread safe but not signal safe
    // if((tm = localtime(&time.tv_sec)) == NULL)
    //     onError("localtime");

    // strftime(buff, sizeof buff, "%M*:%S,", tm);

    seconds = time.tv_sec;
    minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;

    char mins[3];
    mins[2] = 0;
    if(minutes < 10)
    {
        mins[0] = '0';
        mins[1] = '0' + (char)(minutes%10);
    }
    else 
    {
        for(int i = 0; i < 2; ++i)
        {
            mins[1-i] = '0' + (char)(minutes%10);
            minutes /= 10;
        }    
    }

    strncpy(buff, mins, 2);

    strcat(buff, "*:");

    char secs[3];
    secs[2] = 0;
    if(seconds < 10)
    {
        secs[0] = '0';
        secs[1] = '0' + (char)(seconds%10);
    }
    else 
    {
        for(int i = 0; i < 2; ++i)
        {
            secs[1-i] = '0' + (char)(seconds%10);
            seconds /= 10;
        }    
    }

    strncpy(&buff[4], secs, 2);

    nanoseconds = time.tv_nsec;
    char nsec[10];
    nsec[9] = 0;
    for (int i = 0; i < 9; ++i) 
    {
        nsec[8-i] = '0' + (char)(nanoseconds%10);
        nanoseconds /= 10;
    }

    strcat(buff, ",");
    strncpy(&buff[7], nsec, 2);
    strcat(buff, ".");
    strncpy(&buff[10], &nsec[2], 2);
    strcat(buff, ".");
    strncpy(&buff[13], &nsec[4], 2);
    strcat(buff, ".");
    strncpy(&buff[16], &nsec[6], 3);

    return buff;    
}

//---------------------------------------

void readLocalData(struct SocketData* socketData, int fileFd)
{
    char timestamp[20];
    char connectionAddr[108];
    struct timespec readTime;
    struct timespec currentTime;
    char* subTime;
    char* currentTimeStr;

    if(read(socketData->fd, &timestamp, 20) != 20)
        onError("read1");

    if(read(socketData->fd, &connectionAddr, 108) != 108)
        onError("read2");

    if(read(socketData->fd, &readTime, sizeof(struct timespec)) != sizeof(struct timespec))
        onError("read3");

    if(strcmp(connectionAddr, socketData->addr.sun_path))
        return;

    if(clock_gettime(CLOCK_REALTIME, &currentTime) < 0)
        onError("clock_gettime");

    currentTimeStr = timeToStr(currentTime);
    if(write(fileFd, currentTimeStr, 20) < 20)
        onError("write");
    free(currentTimeStr);
    if(write(fileFd, ":", 1) < 1)
        onError("write");
    if(write(fileFd, timestamp, 20) < 20)
        onError("write");
    if(write(fileFd, ":", 1) < 1)
        onError("write");
    subTime = timeDifference(readTime, currentTime);
    if(write(fileFd, subTime, 20) < 20)
        onError("write");
    free(subTime);   
    if(write(fileFd, "\n", 1) < 1)
        onError("write");

}

//---------------------------------------

char* timeDifference(struct timespec timeStart, struct timespec timeEnd)
{
    struct timespec time;
    long start;
    long end;
    int subTime;
    char* timeStr;
    start = timeStart.tv_sec * 1000000000 + timeStart.tv_nsec;
    end = timeEnd.tv_sec * 1000000000 + timeEnd.tv_nsec;
    subTime = end - start;
    time.tv_sec = subTime / 1000000000;
    time.tv_nsec = subTime % 1000000000;
    timeStr = timeToStr(time);
    return timeStr;
}
