#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <sys/utsname.h>

int main(int argc, char** argv, char** env)
{
    struct utsname uts;
    int pid;

    if(argc < 2) {
	printf("Usage:\n\t%s <child_host_name>\n", argv[0]);
	return -1;
    }

    pid = fork();
    switch(pid){
	case 0:
	    printf("In child process...\n");
	    pid = unshare(CLONE_NEWUTS);
	    if(pid == -1) {
		perror("unshare");
		return -1;
	    }
	    pid = sethostname(argv[1], strlen(argv[1]));
	    if(pid == -1) {
		perror("sethostname");
		return -1;
	    }
	    break;
	case -1:
	    perror("fork");
	    return pid;
	    break;
	default:
	    printf("Child created (pid %d)\n", pid);
	    break;
    }

    pid = uname(&uts);
    if(pid == -1) {
	perror("uname");
    }
    printf("HOSTNAME: %s\n",uts.nodename);
    printf("RELEASE: %s\n",uts.release);

    return 0;
}