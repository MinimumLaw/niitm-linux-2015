#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mount.h>

#define NEED_DEEP	5
#define MAX_PATH	128


int do_child(int level)
{
    char	proc_name[MAX_PATH];
    int		ret;

    printf("Starting level: %d\n", level);

    snprintf(proc_name, MAX_PATH, "./proc%d", level);
    ret = mkdir(proc_name, 0555);
    if(ret < 0) {
	perror("mkdir");
	return ret;
    }
    ret = mount("proc", proc_name, "proc", 0, NULL);
    if(ret < 0) {
	perror("mount");
	return ret;
    }

#ifndef DEBUG
	ret = unshare(CLONE_NEWPID);
	if(ret < 0) {
	    perror("unshare");
	    return ret;
	}
#endif

    if(level == 0) { /* on last level do nothing and sleep */
	ret = 0;
#ifdef DEBUG
	sleep(5);
#else
	sleep(100);
#endif
	ret = umount(proc_name);
	if(ret < 0) {
	    perror("umount");
	    return ret;
	}
	ret = rmdir(proc_name);
	if(ret < 0) {
	    perror("rmdir");
	    return ret;
	}
    } else { /* in process */
	int pid;

	pid = fork();
	if(pid < 0) {
	    perror("fork");
	    return pid;
	}

	if(pid == 0) { /* child process */
	    do_child(level-1); /* recursive */
	} else { /* parent process */
	    int status;
	    printf("LEVEL=%d: Child pid=%d\n",
		level, pid);
	    ret = waitpid(pid, &status, 0);
	    ret = umount(proc_name);
	    if(ret < 0) {
		perror("umount");
		return ret;
	    }
	    ret = rmdir(proc_name);
	    if(ret < 0) {
		perror("rmdir");
		return ret;
	    }
	    printf("LEVEL=%d: Child pid=%d exit (code=%d)\n",
		level, pid, status);
	}
    }

    return ret;
}

int main(int argc, char** argv, char** env)
{
    return do_child(NEED_DEEP);
}
