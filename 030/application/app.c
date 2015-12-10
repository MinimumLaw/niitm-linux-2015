#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>

#include "./../module/kbuf_ioctl.h"

const char dev_name[] = "./kbuf";

int wr;
char message[128];
char buff[1024];

void poll_dev(int fd)
{
    struct pollfd fds;
    int ret;

    printf("Try poll() device for read and write : ");
    memset(&fds, 0, sizeof(struct pollfd));
    fds.fd = fd;
    fds.events = POLLIN | POLLOUT;
    ret = poll(&fds, 1, 1);
    if(ret < 0) {
	perror("poll");
    } else {
	if(fds.revents & POLLIN)
	    printf("can read, ");
	if(fds.revents & POLLOUT)
	    printf("can write, ");
    };
    printf("done\n");
}

int main(int argc, char** argv, char** env)
{
    int fd;
    int ret;
    uint32_t count;

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

    printf("Now, seek to end of file\n");
    ret = lseek(fd, 0, SEEK_END);
    if(ret < 0) {
	perror("lseek");
	return ret;
    }

    poll_dev(fd);

    printf("Now, seek to start of file\n");
    ret = lseek(fd, 0, SEEK_SET);
    if(ret < 0) {
	perror("lseek");
	return ret;
    }

    poll_dev(fd);

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

    wr = ioctl(fd, IOKBUF_DEVSTAT, &count);
    if(wr < 0)
	perror("ioctl (devstat)");
    else
        printf("Statistics dev is %d  (ioctl return %d)\n", count, wr);

    close(fd);
    printf("Device file %s closed\n", dev_name);

    return 0;
}