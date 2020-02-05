#ifndef _MASSIVEREADER_H_
#define _MASSIVEREADER_H_

#include <sys/un.h>
#define MAX_EVENTS 256

extern int flag;
struct Arguments
{
    int port;
    char* filePrefix; 
    int filesNo;
};

struct SocketData 
{
    int fd;
    int local;
    struct sockaddr_un addr;
};

void getArguments(struct Arguments* args, int argc, char** argv);
int createServer(int port);
void epollPush(int epollfd, int socketfd, int flags, int local, struct sockaddr_un addr);
void acceptAddConnection(int socket_fd, int epoll_fd);
void onIncomingData(int fd, int epollfd);
void makeNonBlock(int sockfd);
void onError(char* message);
int connectSocket(struct sockaddr_un addr);
void makeLog(struct Arguments* args, int* logfd);
void sigUsr1Handler();
char* timeToStr();
void readLocalData(struct SocketData* socketData, int fileFd);

#endif