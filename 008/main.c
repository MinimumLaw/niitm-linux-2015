#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int get_info(int pid)
{
    DIR* task;
    char  dname[80];
    struct dirent* pthreads;

    sprintf(dname, "/proc/%d/task", pid);

    task = opendir(dname);
    if(!task) {
	printf("%s :\n", dname);
	perror("opendir");
	return -1;
    }

    while(pthreads = readdir(task)) {
	int tpid;

	if(!strcmp(pthreads->d_name,".") ||
		!strcmp(pthreads->d_name,"..")) continue;

	if(sscanf(pthreads->d_name,"%d",&tpid) == 0) {
	    printf("Shit! (%s : %d)\n",pthreads->d_name,  pid);
	    return -1;
	}

	if(tpid == pid) {
	    printf("%d - main task\n", tpid);
	} else {
	    printf("\t\\%d\n", tpid);
	}
    }

    closedir(task);
    return 0;
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

	if(!strcmp(entry->d_name,".") ||
		!strcmp(entry->d_name,"..")) continue;

	if(sscanf(entry->d_name,"%d",&pid) == 0) {
	    fprintf(stderr, "Error convert string (%s) to number (%d)\n",
		    entry->d_name, pid);
	    continue;
	};

	if(get_info(pid)) {
	    printf("Error retry pthread list for pid %d", pid);
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
