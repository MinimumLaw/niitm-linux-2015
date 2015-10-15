#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BUFFER_SIZE	16384

char inbuff[BUFFER_SIZE];
char *path;

void interrupt_callback(int signum)
{
    printf("\nshell finished.\n");
    _exit(0);
}

int main(int argc, char** argv, char** env)
{
    signal(SIGINT, interrupt_callback);

    /* Add pwd to PATH list */
    path = getenv("PATH");
    if(path) {
	strcpy(inbuff, path);
	strcat(inbuff,":.");
    } else {
	strcpy(inbuff,".");
    }

    if(setenv("PATH",inbuff,1)) {
	perror("setenv");
	return -1;
    }

    printf("simple shell started...\n");
    while(1) {
	int readed;
	int cpid;

	write(STDOUT_FILENO, "shell> ",7);

	readed = read(STDIN_FILENO, inbuff, BUFFER_SIZE);
	if(readed < 0) {
	    perror("read");
	    return -1;
	};

	inbuff[readed] = 0; /* convert to ASCII string */
	system(inbuff); /* and exec ;-) */
    }
    return 0;
}
