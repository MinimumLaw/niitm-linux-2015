#include <stdio.h>
#include <pthread.h>

int main(int argc, char** argv, char** env)
{
    if(argc != 3){
	printf("USAGE\n\t%s <interval> <number_of_pthreads>\n", argv[0]);
	return -1;
    }

    return 0;
}