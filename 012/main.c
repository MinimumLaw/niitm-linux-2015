#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sched.h> /* sched_yield() here */
#include <errno.h>

typedef struct {
	pthread_mutex_t	mutex;
	char*	board;
	bool	need_exit;
	pthread_t *p_bulls;
	pthread_t *p_bears;
	int	bulls;
	int	bears;
	int	size;
} game_board;

game_board	brd;

bool init_game_board(game_board *brd)
{
    brd->board = malloc(brd->size);
    if(!brd->board) {
	perror("malloc::game board");
	return false;
    }

    /* default all board free */
    memset((void *)brd->board,'X',brd->size);

    brd->p_bulls = malloc(brd->bulls * sizeof(pthread_t));
    if(!brd->p_bulls) {
	perror("malloc::bulls pids");
	return false;
    }

    brd->p_bears = malloc(brd->bears * sizeof(pthread_t));
    if(!brd->p_bulls) {
	perror("malloc::bears pids");
	return false;
    }

    if(pthread_mutex_init(&brd->mutex, NULL) != 0) {
	perror("mutex:init");
	return false;
    }

    return true;
}

void* bull_pthread(void* arg)
{ /* find X or E on board and replace by U */
    game_board *brd = (game_board *)arg;
    int offset = 0;

    while(1){
	if(brd->need_exit) pthread_exit(NULL);
	sched_yield();
	if(pthread_mutex_lock(&brd->mutex)) {
	    perror("bull:lock()");
	    pthread_exit(NULL);
	}

	if(brd->board[offset] == 'X' || brd->board[offset] == 'E')
	    brd->board[offset] = 'U';

	if(pthread_mutex_unlock(&brd->mutex)) {
	    perror("bull:unlock()");
	    pthread_exit(NULL);
	}
	if(++offset >= brd->size) offset =0;
    };
    return NULL; /* unreach */
}

void* bear_pthread(void* arg)
{ /* find X or U on board and replace by E */
    game_board *brd = (game_board *)arg;
    int offset = 0;

    while(1){
	if(brd->need_exit) pthread_exit(NULL);
	sched_yield();
	if(pthread_mutex_lock(&brd->mutex)) {
	    perror("bear:lock()");
	    pthread_exit(NULL);
	}
	if(brd->board[offset] == 'X' || brd->board[offset] == 'U')
	    brd->board[offset] = 'E';

	if(pthread_mutex_unlock(&brd->mutex)) {
	    perror("bear:unlock()");
	    pthread_exit(NULL);
	}

	if(++offset >= brd->size) offset =0;
    }
    return NULL; /* unreach */
}

bool start_game(game_board *brd)
{
    int tmp;
    
    brd->need_exit = false;
    
    printf("DEBUG: Create bears...\n");
    for(tmp=0; tmp<brd->bears; tmp++) {
	printf("DEBUG: bear %d create ", tmp+1);
	if(pthread_create(&(brd->p_bears[tmp]), NULL,
	    bear_pthread, (void *)brd)) {
	    printf("FAIL\n");
	    perror("bears:pthread:create");
	    return false;
	}
	printf("DONE\n");
    }

    printf("DEBUG: Create bulls...\n");
    for(tmp=0; tmp<brd->bulls; tmp++) {
	printf("DEBUG: bull %d create ", tmp+1);
	if(pthread_create(&(brd->p_bulls[tmp]), NULL,
	    bull_pthread, (void *)brd)) {
	    printf("FAIL\n");
	    perror("bulls:pthread:create");
	    return false;
	}
	printf("DONE\n");
    }
    
    return true;
}

void stop_game(game_board *brd)
{
    brd->need_exit = true;
    /* wait for bulls and bears pthread ends */
}

void show_statistics(game_board *brd)
{
    int x,e,u,tmp;

    x=e=u=tmp=0;

    if(pthread_mutex_lock(&brd->mutex)) {
	perror("stat:lock()");
	pthread_exit(NULL);
    }

    while(tmp<brd->size){
	switch(brd->board[tmp++]) {
	case 'X': x++; break;
	case 'U': u++; break;
	case 'E': e++; break;
	default:
	    printf("Unknown board symbol found!\n");
	}
    }

    printf("Empty: %d, Bears: %d, Bulls: %d, Total: %d\n",
	x, e, u, x+e+u);

    if(pthread_mutex_unlock(&brd->mutex)) {
	perror("stat:unlock()");
	pthread_exit(NULL);
    }
}

void interrupt_callback(int signum)
{
    int tmp;
    
    printf("Termination required.\n");
    brd.need_exit = true;
    
    printf("DEBUG: Killing bears...\n");
    for(tmp=0; tmp<brd.bears; tmp++) {
	printf("DEBUG: bear %d kill ", tmp+1);
	if(pthread_join(brd.p_bears[tmp], NULL)) {
	    printf("FAIL\n");
	    perror("bears:pthread:kill");
	}
	printf("DONE\n");
    }

    printf("DEBUG: Killing bulls...\n");
    for(tmp=0; tmp<brd.bulls; tmp++) {
	printf("DEBUG: bull %d kill ", tmp+1);
	if(pthread_join(brd.p_bulls[tmp], NULL)) {
	    printf("FAIL\n");
	    perror("bulls:pthread:kill");
	}
	printf("DONE\n");
    }
    
    free(brd.board);
    free(brd.p_bulls);
    free(brd.p_bears);

    exit(0);
}

int main(int argc, char** argv, char** env)
{
    if(argc != 4) {
	printf("USAGE:\n\t%s <bulls> <bears> <size>\n", argv[0]);
	return -1;
    };
    
    signal(SIGINT, interrupt_callback);
    
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

    while(1) {
	show_statistics(&brd);
	sched_yield(); 
    }

    return 0;
}