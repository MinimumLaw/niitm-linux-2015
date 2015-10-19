#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 8096

struct {
	char *ext;
	char *filetype;
} extensions [] = {
	{"gif", "image/gif" },  
	{"jpg", "image/jpg" }, 
	{"jpeg","image/jpeg"},
	{"png", "image/png" },  
	{"ico", "image/ico" },  
	{"txt", "text/raw" },
	{"htm", "text/html" },  
	{"html","text/html" },  
	{0,0} };

char	binary_data[] = "octet/stream";

char err_404[] = "HTTP/1.1 404 Not Found\r\n";
char err_403[] = "HTTP/1.1 403 Forbidden\r\n";
char err_500[] = "HTTP/1.1 500 Internal Server Error\r\n";

void send_error(int sock, int code)
{
    printf("Send error to client: ");
    switch(code){
	case 403:
	    printf("%s", err_403);
	    write(sock, err_403, sizeof(err_403));
	    break;
	case 404:
	    printf("%s", err_404);
	    write(sock, err_404, sizeof(err_404));
	    break;
	default: /* 500 */
	    printf("%s", err_500);
	    write(sock, err_500, sizeof(err_500));
	    break;
    }
}

void http_proc(int fd)
{
	int j, file_fd, buflen;
	long i, ret;
	size_t	len;
	char * fstr;
	static char buffer[BUFSIZE+1]; /* static so zero filled */
	char*	fname;

	ret =read(fd,buffer,BUFSIZE); 	/* read Web request in one go */
	switch(ret){
	    case -1:
		perror("Client read");
		return;
		break;
	    case 0:
		printf("Read empty data from client.");
		return;
		break;
	};

	buffer[ret]=0; /* convert to ASCII string */
	printf("Client request:\n%s\n", buffer);

	if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) ) {
		send_error(fd, 403);
		printf("Unsupported request detected\n");
		return;
	}

	/* trim request to ASCII string "GET URL" form */
	for(i=4;i<BUFSIZE;i++) {
		if(buffer[i] == ' ') {
			buffer[i] = 0;
			break;
		}
	}

	/* convert "GET /" to "GET /index.html" */
	if(!strncmp(buffer,"GET /\0",6) || !strncmp(buffer,"get /\0",6))
		strcpy(buffer,"GET /index.html");


	/* work out the file type and check we support it */
	buflen=strlen(buffer);
	fstr = (char *)0;
	for(i=0;extensions[i].ext != 0;i++) {
		len = strlen(extensions[i].ext);
		if( !strncmp(&buffer[buflen-len], extensions[i].ext, len)) {
			fstr =extensions[i].filetype;
			break;
		}
	}
	if(fstr == 0){
		printf("Unsipported extension. Use octet/stream\n");
		fstr = binary_data;
	}

	fname = buffer + 5; /* trim GET_/ from request - get filename */
	if(( file_fd = open(fname, O_RDONLY)) == -1) {
		perror(fname);
		send_error(fd, 404);
		return;
	}
	
	/* get requested file len, and seek to begin of file  */
	len = lseek(file_fd, (off_t)0, SEEK_END);
	lseek(file_fd, (off_t)0, SEEK_SET);

        sprintf(buffer,"HTTP/1.1 200 OK\r\n"
		"Server: lab016/1.0\r\n"
		"Content-Length: %ld\r\n"
		"Connection: close\r\n"
		"Content-Type: %s\r\n\r\n",
    	    len, fstr);
	printf("Server answer:\n%s\n", buffer);
	write(fd,buffer,strlen(buffer));

	/* send file */
	while (	(ret = read(file_fd, buffer, BUFSIZE)) > 0 ) {
		write(fd,buffer,ret);
	}
	close(fd);
}

int main(int argc, char** argv, char** env)
{
	int port, bind_sock, list_sock;
	static struct sockaddr_in bind_addr, list_addr;

	if(argc != 3) {
	    printf("USAGE:\n\t%s <PORT> <DOCROOT>\n",argv[0]);
	    return -1;
	}

	if(chdir(argv[2]) == -1){ 
		printf("ERROR: Is DOCROOT %s correct?\n",argv[2]);
		return -1;
	}
	
	if((bind_sock=socket(AF_INET, SOCK_STREAM,0)) <0) {
	    perror("socket");
	    return -1;
	};

	port = atoi(argv[1]);
	if(port < 1 || port >65535) {
		printf("Port must be in range 1 to 65535!\n");
		return -1;
	};
	
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind_addr.sin_port = htons(port);

	if(bind(bind_sock, (struct sockaddr *)&bind_addr,sizeof(bind_addr)) <0) {
		perror("bind");
		return -1;
	}

	/* FixMe: this time is one-stream server */
	if(listen(bind_sock,1) <0) {
		perror("listen");
		return -1;
	}

	printf("Server ready! Listen connection on port %d\n", port);

	while(1) { /* forever */
	    int tmp = sizeof(list_addr);
	    
	    list_sock = accept(bind_sock, (struct sockaddr*)&list_addr, &tmp);
	    if(list_sock < 0){
		perror("accept");
		return -1;
	    }
	    
	    /* FixMe: this time is one-stream server */
	    http_proc(list_sock);

	    close(list_sock);
	}
}
