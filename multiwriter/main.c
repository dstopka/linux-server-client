#include <stdlib.h>
#include <unistd.h>
#include "Multiwriter.h"


int main(int argc, char** argv)
{
    struct Arguments args;
    getArguments(&args, &argc, &argv);

    _exit(EXIT_SUCCESS);
}

