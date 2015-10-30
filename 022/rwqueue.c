#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sched.h> /* sched_yield() here */
#include <errno.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rwqueue.h"

int fd_rules, fd_board;

game_rules	*rules;
game_board	board;

bool init_game_board(game_rules *rules, game_board board)
{
    /* default all board free */
    memset((void *)board,'X',rules->size);

    rules->p_bulls = malloc(rules->bulls * sizeof(pthread_t));
    if(!rules->p_bulls) {
	perror("malloc::bulls pids");
	return false;
    }

    rules->p_bears = malloc(rules->bears * sizeof(pthread_t));
    if(!rules->p_bulls) {
	perror("malloc::bears pids");
	return false;
    }

    if(pthread_mutex_init(&rules->mutex, NULL) != 0) {
	perror("mutex:init");
	return false;
    }

    return true;
}

void* bull_pthread(void* arg)
{ /* find X or E on board and replace by U */
    game_rules *rules = (game_rules *)arg;
    int offset = 0;

    while(1){
	if(rules->need_exit) pthread_exit(NULL);
	sched_yield();
	if(pthread_mutex_lock(&rules->mutex)) {
	    perror("bull:lock()");
	    pthread_exit(NULL);
	}

	if(board[offset] == 'X' || board[offset] == 'E')
	    board[offset] = 'U';

	if(pthread_mutex_unlock(&rules->mutex)) {
	    perror("bull:unlock()");
	    pthread_exit(NULL);
	}
	if(++offset >= rules->size) offset =0;
    };
    return NULL; /* unreach */
}

void* bear_pthread(void* arg)
{ /* find X or U on board and replace by E */
    game_rules *rules = (game_rules *)arg;
    int offset = 0;

    while(1){
	if(rules->need_exit) pthread_exit(NULL);
	sched_yield();
	if(pthread_mutex_lock(&rules->mutex)) {
	    perror("bear:lock()");
	    pthread_exit(NULL);
	}
	if(board[offset] == 'X' || board[offset] == 'U')
	    board[offset] = 'E';

	if(pthread_mutex_unlock(&rules->mutex)) {
	    perror("bear:unlock()");
	    pthread_exit(NULL);
	}

	if(++offset >= rules->size) offset =0;
    }
    return NULL; /* unreach */
}

bool start_game(game_rules *rules)
{
    int tmp;

    rules->need_exit = false;

    printf("DEBUG: Create bears...\n");
    for(tmp=0; tmp<rules->bears; tmp++) {
	printf("DEBUG: bear %d create ", tmp+1);
	if(pthread_create(&(rules->p_bears[tmp]), NULL,
		bear_pthread, (void *)rules)) {
	    printf("FAIL\n");
	    perror("bears:pthread:create");
	    return false;
	}
	printf("DONE\n");
    }

    printf("DEBUG: Create bulls...\n");
    for(tmp=0; tmp<rules->bulls; tmp++) {
	printf("DEBUG: bull %d create ", tmp+1);
	if(pthread_create(&(rules->p_bulls[tmp]), NULL,
		bull_pthread, (void *)rules)) {
	    printf("FAIL\n");
	    perror("bulls:pthread:create");
	    return false;
	}
	printf("DONE\n");
    }

    return true;
}

void stop_game(game_rules *rules)
{
    rules->need_exit = true;
    /* wait for bulls and bears pthread ends */
}

void interrupt_callback(int signum)
{
    int tmp;

    printf("Termination required.\n");
    rules->need_exit = true;

    printf("DEBUG: Killing bears...\n");
    for(tmp=0; tmp<rules->bears; tmp++) {
	printf("DEBUG: bear %d kill ", tmp+1);
	if(pthread_join(rules->p_bears[tmp], NULL)) {
	    printf("FAIL\n");
	    perror("bears:pthread:kill");
	}
	printf("DONE\n");
    }

    printf("DEBUG: Killing bulls...\n");
    for(tmp=0; tmp<rules->bulls; tmp++) {
	printf("DEBUG: bull %d kill ", tmp+1);
	if(pthread_join(rules->p_bulls[tmp], NULL)) {
	    printf("FAIL\n");
	    perror("bulls:pthread:kill");
	}
	printf("DONE\n");
    }

    exit(0);
}

int main(int argc, char** argv, char** env)
{
    if(argc != 4) {
	printf("USAGE:\n\t%s <bulls> <bears> <size>\n", argv[0]);
	return -1;
    };

    signal(SIGINT, interrupt_callback);

    fd_rules = shm_open(SHM_RULES, O_RDWR | O_CREAT | O_EXCL, 0666);
    if(fd_rules == -1) {
	perror("shm_open::rules");
	printf("Open failed. M.B. not shm mount, allready running or other troubles!\n");
	return -1;
    }

    fd_board = shm_open(SHM_BOARD, O_RDWR | O_CREAT | O_EXCL, 0666);
    if(fd_board == -1) {
	perror("shm_open::board");
	printf("Open failed. M.B. not shm mount, allready running or other troubles!\n");
	return -1;
    }

#if 0
    brd.bulls = atoi(argv[1]);
    brd.bears = atoi(argv[2]);
    brd.size  = atoi(argv[3]);

    if(brd.bulls < 1) {
	printf("ERROR: 1 or more bulls required!\n"); return -1; }
    if(brd.bulls < 1) {
	printf("ERROR: 1 or more bears required!\n"); return -1; }
    if(brd.bulls < 1) {
	printf("ERROR: Size must be 1 or higher!n"); return -1; }

    if(!init_game_board(&brd)) {
	printf("DEBUG: Init failed!\n");
	return -1;
    }

    show_statistics(&brd);

    if(!start_game(&brd)) {
	printf("DEBUG: Start failed!\n");
	return -1;
    }

    while(1) { /* Forever young, forever drunked... */
	sched_yield(); 
    }
#endif
    return 0;
}
