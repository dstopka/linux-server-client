#ifndef _MULTIREADER_H_
#define _MULTIREADER_H_

#define MAX_EVENTS 256

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
void timeToStr();
void epollPush(int epollfd, int socketfd, int flags);
void makeNonBlock(int sockfd);
void acceptConnection(int serverfd, int** socketsList, int epfd);
void onIncomingData(int inetfd, struct Connections* connections);
//char* my_itoa(int x);

#endif