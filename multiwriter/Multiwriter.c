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
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <limits.h>
#include "Multiwriter.h"

int running = 1;

void onError(char* message)
{
    printf("%s error %d\n", message, errno);
    _exit(EXIT_FAILURE);
}

//---------------------------------------

void getArguments(struct Arguments* args, int argc, char* argv[])
{
    //int readArgs = 0;
    int opt;
	while((opt = getopt(argc, argv, "S:p:d:T:")) != -1)
	{
        char* p;
		switch(opt)
		{			
            case 'S':
                args->connectionsNumber = strtol(optarg, &p, 10);    
                if(*p != '\0')
                {
                    printf("Connections number argument must be an integer!\n");
                    _exit(EXIT_FAILURE);
                }
                break;

            case 'p':
                args->port = strtol(optarg, &p, 10);    
                if(*p != '\0')
                {
                    printf("Port argument must be an integer!\n");
                    _exit(EXIT_FAILURE);
                }
                break;

            case 'd':
                args->interspace = strtof(optarg, &p);    
                if(*p != '\0')
                {
                    printf("Interspace argument must be a float!\n");
                    _exit(EXIT_FAILURE);
                }
                break;

            case 'T':
                args->runtime = strtof(optarg, &p);    
                if(*p != '\0')
                {
                    printf("Runtime argument must be a float!\n");
                    _exit(EXIT_FAILURE);
                }
                break;

			default: 
				printf("WRONG ARGUMENTS!\n");
				_exit(EXIT_FAILURE);
		}
	}
    // if(readArgs < 4)
    // {
    //     printf("To few arguments!\n");
    //     _exit(EXIT_FAILURE);
    // }
    if(optind+1 < argc)
    {
        printf("Unrecognised arguments were specified!\n");
        _exit(EXIT_FAILURE);
    }
}

//---------------------------------------

int createServer(struct sockaddr_un* addr)
{
    int sockfd;

    if((sockfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
        onError("socket");    
 
    if(bind(sockfd, (struct sockaddr*)addr, sizeof(struct sockaddr_un))<0)
        onError("bind");

    makeNonBlock(sockfd);

    if ((listen(sockfd, 5)) < 0) 
        onError("listen");

    return sockfd;
}

//---------------------------------------

struct sockaddr_un randomAddr()
{
    struct sockaddr_un servaddr;
    int fd;
    int readBytes;
    int size = sizeof(((struct sockaddr_un*)0)->sun_path)-2;
    char* buff = (char*)malloc(size);

    if((fd = open("/dev/urandom", O_RDONLY, 0)) < 0)
	    onError("open");
    if((readBytes=read(fd, buff, size) < 0))
        onError("read");

    memset(&servaddr, 0, sizeof(struct sockaddr_un));
    servaddr.sun_family = AF_LOCAL;
    strncpy(&servaddr.sun_path[1], buff, size);

    free(buff);
    return servaddr;
}

//---------------------------------------

int createClient(int port)
{
    struct sockaddr_in servaddr;
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        onError("socket");

    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port=htons(port);

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr_in)) < 0)
        onError("connect");

    makeNonBlock(sockfd);

    return sockfd;
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

//---------------------------------------

void epollPush(int epollfd, int socketfd, int flags)
{
    struct epoll_event event;
    event.data.fd = socketfd;
	event.events = flags;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &event) < 0)
        onError("epoll_ctl add");
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

void acceptConnection(int serverfd, int** socketsList, int epfd)
{
    int sockfd;
    if((sockfd = accept(serverfd, NULL, NULL)) < 0)
        onError("accept");
    **socketsList = sockfd;
    (*socketsList)++;
    epollPush(epfd, sockfd, EPOLLIN | EPOLLET);
}

//---------------------------------------

void onIncomingData(int inetfd, struct Connections* connections)
{
     while(1) {
        struct sockaddr_un addr;
        if (read(inetfd, &addr, sizeof(struct sockaddr_un)) != sizeof(struct sockaddr_un))
            break;
        if(addr.sun_family != USHRT_MAX)
            connections->connectedNo++;
        else
            connections->rejectedNo++;
    }
}

//---------------------------------------

timer_t createTimer()
{
    timer_t id;
    struct sigevent sev;
    sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGUSR1;

	if(timer_create(CLOCK_REALTIME, &sev, &id) < 0)
	{
		perror("timer_create");
		_exit(EXIT_FAILURE);
	}
    return id;
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

void sigUsr1Handler()
{
    running = 0;
}

//---------------------------------------

struct itimerspec setTime(float time)
{
    double t = time;
    struct itimerspec ts;
    t = time * 10000000;
    ts.it_value.tv_sec = (int)(t / 1000000000);
    ts.it_value.tv_nsec = (int)time % 1000000000;
    ts.it_interval.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;

    return ts;
}