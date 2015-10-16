#include <stdio.h>
#include <string.h>

#define STRLEN	80

char buff[STRLEN];

int main(int argc, char** argv, char** env)
{
    FILE* fd;
    int count=0;
    char *restart;

    if(argc != 3){
	printf("USAGE:\n\t%s <string> <filename>\n", argv[0]);
	return -1;
    }
    
    while(fgets(buff, STRLEN - 1, stdin) != NULL) {
	restart = buff;
	while( (restart = strstr(restart, argv[1])) != NULL ) {
	    restart++;
	    count++;
	}
	fputs(buff, stdout);
    }
    fclose(stdout); /* for next proc in pipe */
    

    if((fd = fopen(argv[2],"w")) == NULL) {
	perror(argv[2]);
	return -1;
    };
    
    fprintf(fd,"%d\n",count);
    
    fclose(fd);
    
    return 0;
}