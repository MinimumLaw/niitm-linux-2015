#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

int parent_pid(int pid_code)
{
    FILE *file_st;
    char string_st[20];
    char buf_ppid[10];
    int  num_ppid =0;
    char name_dir[1024]; /* ??? */

    sprintf(name_dir,"/proc/%d/status",pid_code);
  
    file_st = fopen(name_dir,"r");
    if (!file_st) {
	printf("Error open file %s\n", name_dir);
	return -1;
    }

    while (fgets(string_st,sizeof(string_st),file_st)) {
	if (strstr(string_st,"PPid")) {
    	    strcpy(buf_ppid,&string_st[6]);
            num_ppid = atoi(buf_ppid);
            break;
	}	
    }
  
    fclose(file_st);

    return num_ppid;
}

int print_tree(int pid_code)
{
    char tree_pid[10]; /* 0..65535 ??? */
    char buf[1024]; /* ??? */
    int  ppid_code = 0;

    sprintf(tree_pid,"%d",pid_code);
    while(pid_code)
    {
	ppid_code = parent_pid(pid_code);
	sprintf(buf," <= %d",ppid_code);
	strcat(tree_pid,buf);
	pid_code = ppid_code;

	if(!ppid_code)
    	    printf(" * %s \n",tree_pid);
    }
    return 0;
}

int scan_all_proc(void)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir("/proc/");

    if (!dir) {
        perror("diropen");
        return -1;
    };

    while ( (entry = readdir(dir)) != NULL )
    {
        if (atoi(entry->d_name))
            print_tree(atoi(entry->d_name));
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
	return print_tree(atoi(argv[1]));
    else
	return scan_all_proc();

    /* unreach */
    return 0;
}

