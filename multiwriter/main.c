#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "Multiwriter.h"


int main(int argc, char** argv)
{
    struct sockaddr_un servaddr;
    struct Arguments args;
    getArguments(&args, &argc, &argv);
    int serverfd;
    int inetfd;
    servaddr = randomAddr();
    serverfd = createServer(&servaddr);
    inetfd = createClient(args.port);
    if(serverfd)
    {

    }

    while (1) {
    
    }
    // int sockfd, connfd; 
    // struct sockaddr_in servaddr, cli; 
  
    // // socket create and varification 
    // sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    // if (sockfd == -1) { 
    //     printf("socket creation failed...\n"); 
    //     exit(0); 
    // } 
    // else
    //     printf("Socket successfully created..\n"); 
    // bzero(&servaddr, sizeof(servaddr)); 
  
    // // assign IP, PORT 
    // servaddr.sin_family = AF_INET; 
    // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    // servaddr.sin_port = htons(PORT); 
  
    // // connect the client socket to server socket 
    // if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
    //     printf("connection with the server failed...\n"); 
    //     exit(0); 
    // } 
    // else
    //     printf("connected to the server..\n"); 
  
    // // close the socket 
    // close(sockfd); 

    _exit(EXIT_SUCCESS);
}

