#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Multiwriter.h"

void getArguments(struct Arguments* args, int* argc, char** argv[])
{
    int readArgs = 0;
    int opt;
	while((opt = getopt(*argc, *argv, "S:p:d:T:")) != -1)
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
                args->port = strtof(optarg, &p);    
                if(*p != '\0')
                {
                    printf("Interspace argument must be an integer!\n");
                    _exit(EXIT_FAILURE);
                }
                break;

            case 'T':
                args->port = strtof(optarg, &p);    
                if(*p != '\0')
                {
                    printf("Runtime argument must be an integer!\n");
                    _exit(EXIT_FAILURE);
                }
                break;

			default: 
				printf("WRONG ARGUMENTS!\n");
				_exit(EXIT_FAILURE);
		}
	}
    if(readArgs < 4)
    {
        printf("To few arguments!\n");
        _exit(EXIT_FAILURE);
    }
    if(optind+1 < *argc)
    {
        printf("Unrecognised arguments were specified!\n");
        _exit(EXIT_FAILURE);
    }
}