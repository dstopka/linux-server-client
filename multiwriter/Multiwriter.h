#ifndef _MULTIREADER_H_
#define _MULTIREADER_H_

#include <bits/types/struct_itimerspec.h>
#include <time.h>

#define MAX_EVENTS 256

extern int running;

struct Arguments
{
    int connectionsNumber;
    int port;
    float interspace;
    float runtime;
};

struct Connections
{
    int* connectedSockets;
    int connectedNo;
    int rejectedNo;
};

void getArguments(struct Arguments* args, int argc, char* argv[]);
void onError(char* message);
int createClient(int port);
int createServer(struct sockaddr_un* addr);
int createSocket();
struct sockaddr_un randomAddr();
char* timeToStr();
void epollPush(int epollfd, int socketfd, int flags);
void makeNonBlock(int sockfd);
void acceptConnection(int serverfd, int** socketsList, int epfd);
void onIncomingData(int inetfd, struct Connections* connections);
timer_t createTimer();
void setHandler();
void sigUsr1Handler();
struct itimerspec setTime(float time);
void sumServiceTime(struct timespec start, struct timespec end, struct timespec* sum);
void sendData(int max, struct Connections* connected, struct sockaddr_un addr, struct timespec* serviceTime);

#endif