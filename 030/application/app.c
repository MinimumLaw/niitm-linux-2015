#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "./../module/kbuf_ioctl.h"

const char dev_name[] = "./kbuf";

int wr;
char message[128];
char buff[1024];

int main(int argc, char** argv, char** env)
{
    int fd;

    if(argc == 2)
	strcpy(message,argv[1]);
    else
	strcpy(message,"Hello, world!");

    fd = open(dev_name, O_RDWR);
    if(fd < 0) {
	perror("open");
	return fd;
    }
    printf("Device file %s opened\n", dev_name);

    wr = write(fd,message,strlen(message));
    printf("Write to device %s %d bytes\n",
	dev_name, wr);

    close(fd);
    printf("Device file %s closed\n", dev_name);

    /* no seek() in module, just close and reopen */

    fd = open(dev_name, O_RDWR);
    if(fd < 0) {
	perror("open");
	return fd;
    }

    wr = read(fd, buff, wr);
    printf("Read back from device %s %d bytes\n",
	dev_name, wr);

    printf("Contents: %s\n", buff);

    wr = ioctl(fd, IOKBUF_STAT);
    if(wr < 0)
	perror("ioctl");
    else
        printf("Statistics writen to syslog ioctl() return %d\n", wr);

    close(fd);
    printf("Device file %s closed\n", dev_name);

    return 0;
}