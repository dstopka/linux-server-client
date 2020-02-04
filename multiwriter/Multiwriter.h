#ifndef _MULTIREADER_H_
#define _MULTIREADER_H_

struct Arguments
{
    int connectionsNumber;
    int port;
    float interspace;
    float runtime;
};

void getArguments(struct Arguments* args, int* argc, char** argv[]);
void onError(char* message);
int createClient(int port);
int createServer(struct sockaddr_un* addr);
int createSocket();
struct sockaddr_un randomAddr();

#endif