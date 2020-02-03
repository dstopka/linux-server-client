#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct Arguments
{
    uint port;
    char* filePrefix; 
};

void getArguments(struct Arguments* args, int* argc, char** argv[]);

int main(int argc, char** argv)
{
    struct Arguments args;
    getArguments(&args, &argc, &argv);

    _exit(EXIT_SUCCESS);
}

//************

void getArguments(struct Arguments* args, int* argc, char** argv[])
{
    int opt;
	while((opt = getopt(*argc, *argv, "O:")) != -1)
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
                break;
            }
			default: 
				printf("WRONG ARGUMENTS!\n");
				_exit(EXIT_FAILURE);
		}
	}
    if(optind >= *argc)
    {
        printf("To few arguments!\n");
        _exit(EXIT_FAILURE);
    }
    char* p;
    args->port = strtoul((*argv)[optind], &p, 10);    
    if(*p != '\0')
    {
        printf("Port argument must be an integer!\n");
        _exit(EXIT_FAILURE);
    }
    if(optind+1 < *argc)
    {
        printf("Unrecognised arguments were specified!\n");
        _exit(EXIT_FAILURE);
    }
}