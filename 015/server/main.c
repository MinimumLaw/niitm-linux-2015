#include <stdio.h>

int main(int argc, char** argv, char** env)
{
    if(argc != 2) {
	printf("USAGE:\n\t%s <listen_port>\n", argv[0]);
	return -1;
    }

    return 0;
}