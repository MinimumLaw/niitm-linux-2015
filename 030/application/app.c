#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

const char dev_name[] = "./kbuf";

int main(int argc, char** argv, char** env)
{
    int fd;

    fd = open(dev_name, O_RDWR);
    if(fd < 0) {
	perror("open");
	return fd;
    }

    printf("Device file %s opened\n", dev_name);

    close(fd);
    printf("Device file %s closed\n", dev_name);

    return 0;
}