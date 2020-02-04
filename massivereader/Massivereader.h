#ifndef _MASSIVEREADER_H_
#define _MASSIVEREADER_H_

#define MAX_EVENTS 256

struct Arguments
{
    int port;
    char* filePrefix; 
};

void getArguments(struct Arguments* args, int argc, char** argv);
void createSocket(int* sockfd, int port);
void epollPush(int epollfd, int socketfd, int flags);

#endif