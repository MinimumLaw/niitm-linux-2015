#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

/*
 * Attention: 
 * Recursion used!
 * FixMe: buffers _MUST_ be malloc/free'ed!!!
 */
int get_info(int pid)
{
    FILE* fd;
    char  fname[80];
    char  stat_str[80];
    int	  parent = -1;
    
    /* Not show and not seek parent for PID = 0 */
    if(pid == 0) {
	printf("[KERN]\n");
	return 0;
    }
    
    
    sprintf(fname, "/proc/%d/status", pid);
    fd = fopen(fname,"r");
    if (!fd) {
	printf("%s: \n", fname);
	perror("fopen");
	return -1;
    };

    while (fgets(stat_str,sizeof(stat_str),fd)) {
	if (strstr(stat_str,"PPid")) {
            parent = atoi(stat_str + 6);
            break;
	}	
    }
    
    printf("[%d]", pid);
    if(parent == -1) {
	printf("\n");
	return 0;
    } else {
	printf(" <= ");
	return get_info(parent);
    };
}


int scan_all_proc(void)
{
    DIR *dir;
    int pid;
    struct dirent *entry;

    dir = opendir("/proc/");

    if (!dir) {
        perror("diropen");
        return -1;
    };

    while ( (entry = readdir(dir)) != NULL ) {
        if(sscanf(entry->d_name,"%d",&pid) == 0) {
    	    fprintf(stderr, "Error convert string (%s) to number (%d)\n",
        	    entry->d_name, pid);
    	    continue;
        };
        
        if(get_info(pid)) {
    	    printf("Error retry process tree for pid %d", pid);
    	    continue;
        };
    };
    return closedir(dir);
}


int main(int argc, char** argv, char** env)
{
    if(argc > 2) {
	printf("USAGE:\n\t%s [pid]\n\t if no pid - show all process tree\n", argv[0]);
	return -1;
    }

    if(argc==2)
	return get_info(atoi(argv[1]));
    else
	return scan_all_proc();

    /* unreach */
    return 0;
}

