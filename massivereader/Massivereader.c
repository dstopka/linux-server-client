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
#include "Massivereader.h"

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
                {
                    printf("Memory allocation failed!\n");
                    _exit(EXIT_FAILURE);
                }
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

void createSocket(int* sockfd, int port)
{
    struct sockaddr_in servaddr;
    int flags;

    if((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket error\n");
        _exit(EXIT_FAILURE);
    }

    memset(&servaddr,'\0', sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port=htons(port);
 
    if(bind(*sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
    {
        printf("listen error\t%d\n", errno);
        _exit(EXIT_FAILURE);
    }

    if ((flags = fcntl(*sockfd, F_GETFL, 0)) < 0)
    {
        printf("fcntl getfl error\n");
        _exit(EXIT_FAILURE);
    }

    if (fcntl(*sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        printf("fcntl setfl error\n");
        _exit(EXIT_FAILURE);
    }
}

void epollPush(int epollfd, int socketfd, int flags)
{
    struct epoll_event event;
    event.data.fd = socketfd;
	event.events = EPOLLIN | EPOLLET;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &event) < 0)
    {
		printf("epoll_ctl error\n");
        _exit(EXIT_FAILURE);
	}
}