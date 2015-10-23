#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int	serv,sock,port;
int 	broadcastEnable=1;
struct	sockaddr_in serv_addr, sock_addr;

int main(int argc, char** argv, char** env)
{
    if(argc != 2) {
	printf("USAGE:\n\t%s <PORT>\n", argv[0]);
	return -1;
    }

    port = atoi(argv[1]);
    if(port < 1 || port>65535) {
	printf("Port error!\n");
	return -1;
    }

    /* create server (receiver) socket */
    serv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock<0) {
	perror("server socket");
	return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(serv, (const struct sockaddr*)&serv_addr, sizeof(struct sockaddr_in)) < 0) {
	perror("bind");
	return -1;
    }

    /* create client (sending) socket */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(sock<0) {
	perror("client socket");
	return -1;
    }

    if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
	    &broadcastEnable, sizeof(broadcastEnable)) < 0) {
	perror("setsockopt");
	return -1;
    }

    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

    if(connect(sock, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr_in)) < 0) {
	perror("connect");
	return -1;
    }

    /* chat here */
    while(1) {
	char buff[80];
	fd_set rs, ws, xs;
	int ret,rd,wr;

	FD_ZERO(&rs);
	FD_SET(serv, &rs);
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
		ret = write(sock, &buff, ret);
		if(ret < 0) {
		    perror("write sock");
		}
	    }
	} else if (FD_ISSET(serv, &rs)) {
	    int ret = read(serv, &buff, sizeof(buff));

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
