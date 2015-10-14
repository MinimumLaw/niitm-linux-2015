#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

int	sock,port;
struct	sockaddr_in bind_addr, bcast_addr;

int main(int argc, char** argv, char** env)
{
    if(argc != 2) {
	printf("USAGE:\n\t%s <PORT>\n", argv[0]);
	return -1;
    }

    /* create socket */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port);
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bcast_addr.sin_family = AF_INET;
    bcast_addr.sin_port = htons(port);
    bcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    if(bind(sock, (struct sockaddr*)&bind_addr, sizeof(struct sockaddr_in)) < 0) {
	perror("bind");
	return -1;
    }
    
    /* chat here */
    while(1) {
	char buff[80];
	fd_set rs, ws, xs;
	int ret,rd,wr;
	
	
	FD_ZERO(&rs);
	FD_SET(sock, &rs);
	FD_SET(STDIN_FILENO, &rs);
	
	FD_ZERO(&xs);
	FD_ZERO(&ws);
	
	
	ret = select(sock + 1, &rs, &ws, &xs, NULL);
	if(ret < 0) {
	    perror("select");
	    return errno;
	};

	if(FD_ISSET(STDIN_FILENO,&rs)) {
	    int ret = read(STDIN_FILENO,&buff, sizeof(buff));
	    
	    if(ret < 0) {
		perror("stdin read");
	    } else if (ret > 0) {
		ret = write(wr, &buff, ret);
		if(ret < 0) {
		    perror("write sock");
		}
	    }
	} else if (FD_ISSET(sock, &rs)) {
	    int ret = read(sock, &buff, sizeof(buff));
	    
	    if(ret < 0) {
		perror("sock read");
	    } else if (ret > 0) {
		ret = write(STDOUT_FILENO, &buff, ret);
		if(ret < 0) {
		    perror("write stdout");
		}
	    }
	} else {
	    printf("Khm... Shit happened...\n");
	    continue;
	}
    }
    
}