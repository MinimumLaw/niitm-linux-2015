#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

int pid,sig;

int main(int argc, char **argv, char** env)
{

	if(argc != 3) {
		printf("USAGE:\n\t%s [PID] [SIG]\n",argv[0]);
		return -1;
	}

	pid = atoi(argv[1]);
	if(pid<1) {
		printf("ERROR: PID must be 1 one more.\n");
		return -1;
	};

	sig = atoi(argv[2]);
	if(sig<0 || sig>15) {
		printf("ERROR: SIG must be betwen 1 and 15\n");
		return -1;
	};

	if(kill(pid,sig) == -1) {
		perror("kill");
		return -1;
	}

	return 0; 
}
