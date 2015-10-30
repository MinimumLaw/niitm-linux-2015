#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>

int fd_in, fd_out;

int open_in(char* name);
int open_out(char* name);

int open_in(char* name)
{
    int ret = open(name, O_RDWR);
    if(ret < 0) {
	printf("DEBUG: %s not present. Create\n", name);
	umask(0);
	ret = mkfifo(name, 0666);
	if(ret < 0) return ret;
	printf("DEBUG: %s Created\n", name);
	return open_in(name);
    }
    printf("DEBUG: %s open success\n", name);
    return ret;
}

int open_out(char* name)
{
    int ret = open(name, O_RDWR);
    if(ret < 0) {
	printf("DEBUG: %s not present. Create\n", name);
	umask(0);
	ret = mkfifo(name, 0666);
	printf("DEBUG: %s Created\n", name);
	if(ret < 0) return ret;
	return open_out(name);
    }
    printf("DEBUG: %s open success\n", name);
    return ret;
}

int main(int argc, char** argv, char** env)
{
    if(argc != 3) {
	printf("USAGE:\n\t%s <PIPE_IN> <PIPE_OUT>\n", argv[0]);
	return -1;
    }

    fd_in = open_in(argv[1]);
    if(fd_in < 0) {
	perror("in pine");
	return errno;
    };

    fd_out = open_out(argv[2]);
    if(fd_out < 0) {
	perror("out pipe");
	return errno;
    };

    /* chat here */
    while(1) {
	int ret;
	char buff[80];
	struct pollfd pfd[] = {
	    {
		.fd = fd_in,
		.events = POLLIN,
	    },
	    {
		.fd = STDIN_FILENO,
		.events = POLLIN,
	    },
	};

	ret = poll(pfd, sizeof(pfd)/sizeof(struct pollfd), 0);
	if(ret < 0) {
	    perror("poll");
	    return errno;
	};

	if(pfd[0].revents == POLLIN){
	    ret = read(fd_in,&buff, sizeof(buff));
	    if(ret < 0) {
	        perror("read");
	    } else if (ret > 0) {
		ret = write(STDOUT_FILENO, &buff, ret);
		if(ret < 0) {
		    perror("write");
		}
	    }
	}
	
	if(pfd[1].revents == POLLIN) {
	    ret = read(STDIN_FILENO,&buff, sizeof(buff));
	    if(ret < 0) {
	        perror("read");
	    } else if (ret > 0) {
		ret = write(fd_out, &buff, ret);
		if(ret < 0) {
		    perror("write");
		}
	    }
	}
    }
}
