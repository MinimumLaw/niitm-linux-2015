#include <stdio.h>

int main(int argc, char** argv, char** env)
{
    if(argc != 4){
	printf("USAGE:\n\t%s <server_addr> <server_port> <name>\n", argv[0]);
	return -1;
    }

    return 0;
}