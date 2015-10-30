#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>

long	iterations = 0;
int	rsize;
long	count;

int compare(const void* a, const void* b)
{
    iterations++;
    return memcmp(a,b,rsize);
}

int main(int argc, char** argv, char** env)
{
    void*	fmap;
    int		fd;
    struct stat	sb;

    if(argc != 3) {
	printf("USAGE:\n\t%s <file> <rsize>\n", argv[0]);
	return -1;
    }

    rsize = atoi(argv[2]);
    if(rsize<1 || rsize>256){
	printf("Record size must be in range [1..256]\n");
	return -1;
    }

    fd = open(argv[1], O_RDWR);
    if(fd<0){
	perror("open");
	return -1;
    }

    if(fstat(fd, &sb) == -1){
	perror("fstat");
	return -1;
    }
    count = sb.st_size / rsize;

    printf("DEBUG: file size=%ld, record_size=%d, records_count=%ld\n",
	sb.st_size, rsize, count);

    fmap = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(fmap == MAP_FAILED){
	perror("mmap");
	return -1;
    };

    qsort(fmap, count, rsize, compare);
    printf("DEBUG: sorting done with %ld iterations\n", iterations);
    msync(fmap, sb.st_size, MS_SYNC);

    munmap(fmap, sb.st_size);
    close(fd);

    return 0;
}
