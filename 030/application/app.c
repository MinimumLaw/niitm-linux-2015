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
    int ret;

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

    printf("Now, seek to start of file\n");
    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
	perror("lseek");
	return ret;
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

    wr = ioctl(fd, IOKBUF_PROCSTAT, getpid());
    if(wr < 0)
	perror("ioctl");
    else
        printf("Statistics about myself in syslog ioctl() return %d\n", wr);

    close(fd);
    printf("Device file %s closed\n", dev_name);

    return 0;
}