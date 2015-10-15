#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFLEN	128

char buff[BUFFLEN];

/* FixMe: bad algorithm (to more stack) !!! */
void strrev(char* str)
{
    char buff[BUFFLEN];
    int len = strlen(str) - 2; /* \n adn \x0 */
    int x,y;

    strcpy(buff, str);
    for(x=len, y=0; x >= 0; x--, y++)
	str[y] = buff[x];
}

int main(int argc, char** argv, char** env)
{
    FILE *fp;

    if(argc != 2) {
	printf("USAGE:\n\t%s <cmd>",argv[0]);
	return -1;
    }

    if(fp = popen(argv[1], "r")) {
	while (fgets(buff, sizeof(buff)-1, fp) != NULL) {
	    strrev(buff);
	    printf("%s", buff);
	}

	return pclose(fp);
    }

    fprintf(stderr,"Exec failed: %s\n", argv[1]);
    return -1;
}