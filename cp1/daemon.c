#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BIND_PORT 1666

volatile int USR1_COUNT, USR2_COUNT;
int fd;

void daemon_init(void);

void usr1_callback(int num) { ++USR1_COUNT; }
void usr2_callback(int num) { ++USR2_COUNT; }

void hup_callback(int num)
{
    syslog(LOG_DAEMON,"SIGHUP received! Restoring default!\n");
    daemon_init();
}

void term_callback(int num)
{
    close(fd);
    syslog(LOG_DAEMON,"SIGTERM received! Daemon finished!\n");
    exit(0);
}

void daemon_init(void)
{
    USR1_COUNT = 0;
    USR2_COUNT = 0;
    signal(SIGUSR1, usr1_callback);
    signal(SIGUSR2, usr2_callback);
    signal(SIGTERM, term_callback);
    signal(SIGHUP,   hup_callback);
    syslog(LOG_DAEMON,"Daemon started!\n");
}

char read_buff[128];
char write_buff[128];
char msgUSR1_EN[] = "Enable USR1 counter!\n";
char msgUSR1_DIS[] = "Disable USR1 counter!\n";
char msgUSR2_EN[] = "Enable USR2 counter!\n";
char msgUSR2_DIS[] = "Disable USR2 counter!\n";

char msgHello[] = "Hello from daemon!\nUse E1,D1,E2,D2,EXIT or STAT commands.\n";
char msgExit[] = "See you later, baby!\n";
char msgError[] = "Unsupport input:\n";

void do_client_chat(int fd)
{
    int count;
    write(fd, msgHello, strlen(msgHello));

    while((count = read(fd, read_buff, sizeof(read_buff))) > 0) {

	if(!strncmp(read_buff,"E1",2)) {
	    syslog(LOG_DAEMON, "%s", msgUSR1_EN);
	    write(fd, msgUSR1_EN, strlen(msgUSR1_EN));
	    signal(SIGUSR1, usr1_callback);
	} else if(!strncmp(read_buff,"D1",2)) {
	    syslog(LOG_DAEMON, "%s", msgUSR1_DIS);
	    write(fd, msgUSR1_DIS, strlen(msgUSR1_DIS));
	    signal(SIGUSR1, SIG_IGN);
	} else if(!strncmp(read_buff,"E2",2)) {
	    syslog(LOG_DAEMON, "%s", msgUSR2_EN);
	    write(fd, msgUSR2_EN, strlen(msgUSR2_EN));
	    signal(SIGUSR2, usr2_callback);
	} else if(!strncmp(read_buff,"D2",2)) {
	    syslog(LOG_DAEMON, "%s", msgUSR2_DIS);
	    write(fd, msgUSR2_DIS, strlen(msgUSR2_DIS));
	    signal(SIGUSR2, SIG_IGN);
	} else if(!strncmp(read_buff,"EXIT",4)) {
	    syslog(LOG_DAEMON, "%s", msgExit);
	    write(fd, msgExit, strlen(msgExit));
	    return;
	} else if(!strncmp(read_buff,"STAT",4)) {
	    int count;
	    count = snprintf(write_buff, sizeof(write_buff),
		"STAT: USR1=%d, USR2=%d\n",
		USR1_COUNT, USR2_COUNT);
	    syslog(LOG_DAEMON, "%s", write_buff);
	    write(fd, write_buff, count);
	} else {
	    write(fd, msgError, strlen(msgError));
	    write(fd, read_buff, count);
	}
    };
}

void daemon_start(void)
{
    int ret;
    struct sockaddr_in bind_addr;

    fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd < 0){
	syslog(LOG_DAEMON,"Error on create socket (%d)\n", fd);
	exit(fd);
    }

    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind_addr.sin_port = htons(BIND_PORT);

    ret = bind(fd, (struct sockaddr*)&bind_addr,sizeof(bind_addr));
    if(ret < 0) {
	syslog(LOG_DAEMON,"Error on binding to port %d (%d)\n", BIND_PORT, ret);
	exit(ret);
    }

    ret = listen(fd, 1);
    if(ret < 0) {
	syslog(LOG_DAEMON,"Error on listen (%d)\n", ret);
	exit(ret);
    }

    while(1) {
	struct sockaddr_in list_addr;
	int tmp = sizeof(list_addr);
	int rem_fd;

	rem_fd = accept(fd, (struct sockaddr*)&list_addr, &tmp);

	if(rem_fd < 0) {
	    syslog(LOG_DAEMON, "accept connection failed\n");
	    exit(-1);
	}

	do_client_chat(rem_fd);

	close(rem_fd);
    }

}

int main(int argc, char **argv, char **env)
{
    switch(fork()) {
	case 0: { /* child */
		setsid();
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		daemon_init();
		daemon_start();
	    }; break;
	case -1: { /* error */
		perror("fork");
		return -1;
	    };
	default: { /* parent */
		return 0;
	    };
    }

    return 0;
}