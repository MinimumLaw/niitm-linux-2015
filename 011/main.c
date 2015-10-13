#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

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
	char buff[80];
	fd_set rs, ws, xs;
	int ret,rd,wr;
	
	
	FD_ZERO(&rs);
	FD_SET(fd_in, &rs);
	FD_SET(STDIN_FILENO, &rs);
	
	FD_ZERO(&xs);
	FD_ZERO(&ws);
	
	
	ret = select(fd_out + 1, &rs, &ws, &xs, NULL);
	if(ret < 0) {
	    perror("select");
	    return errno;
	};

	if(FD_ISSET(STDIN_FILENO,&rs)) {
	    rd = STDIN_FILENO;
	    wr = fd_out;
	} else if (FD_ISSET(fd_in, &rs)) {
	    rd = fd_in;
	    wr = STDOUT_FILENO;
	} else {
	    printf("Khm... Shit happened...\n");
	    continue;
	}
	
	ret = read(rd,&buff, sizeof(buff));
	if(ret < 0) {
	    perror("read");
	} else if (ret > 0) {
	    ret = write(wr, &buff, ret);
	    if(ret < 0) {
		perror("write");
	    }
	}
    }
    
}