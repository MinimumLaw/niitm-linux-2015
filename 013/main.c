#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

int fd_in, fd_out;

int open_in(char* name);
int open_out(char* name);

int open_in(char* port_number)
{
    int ret;
    int port = atoi(port_number);
    struct sockaddr_in bind_addr;

    if(port<1 || port>65535) {
	printf("open_in : input port must be in range 1 to 65535\n");
	return -1;
    }
    ret = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    bind_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    if(bind(ret, (struct sockaddr*)&bind_addr, sizeof(struct sockaddr_in)) < 0) {
	perror("bind");
	return -1;
    }

    return ret;
}

int open_out(char* port_number)
{
    int ret;
    int port = atoi(port_number);
    struct sockaddr_in conn_addr;

    if(port<1 || port>65535) {
	printf("open_in : output port must be in range 1 to 65535\n");
	return -1;
    }
    ret = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    conn_addr.sin_family = AF_INET;
    conn_addr.sin_port = htons(port);
    conn_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    if(connect(ret, (struct sockaddr*)&conn_addr, sizeof(struct sockaddr_in)) < 0) {
	perror("connect");
	return -1;
    }

    return ret;
}

int main(int argc, char** argv, char** env)
{
    if(argc != 3) {
	printf("USAGE:\n\t%s <PORT_IN> <PORT_OUT>\n", argv[0]);
	return -1;
    }

    fd_in = open_in(argv[1]);
    if(fd_in < 0) {
	perror("Create IN connection (bind)");
	return errno;
    };

    fd_out = open_out(argv[2]);
    if(fd_out < 0) {
	perror("Create OUT connection (connect)");
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