#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    brd->p_bulls = malloc(brd->bulls * sizeof(pthread_t));
    if(!brd->p_bulls) {
	perror("malloc::bulls pids");
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
	if(!pthread_mutex_lock(&brd->mutex)) {
	    perror("bull:lock()");
	    pthread_exit(NULL);
	}

	if(brd->board[offset] == 'X' || brd->board[offset] == 'E')
	    brd->board[offset] = 'U';

	if(!pthread_mutex_unlock(&brd->mutex)) {
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
	if(!pthread_mutex_lock(&brd->mutex)) {
	    perror("bear:lock()");
	    pthread_exit(NULL);
	}
	if(brd->board[offset] == 'X' || brd->board[offset] == 'U')
	    brd->board[offset] = 'E';

	if(!pthread_mutex_unlock(&brd->mutex)) {
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
    
    for(tmp=0; tmp<brd->bulls; tmp++) {
	if(pthread_create(&brd->p_bulls[tmp], NULL,
	    &bull_pthread, (void *)brd)!=0) {
	    perror("bulls:pthread:create");
	    return false;
	}
    }

    for(tmp=0; tmp<brd->bears; tmp++) {
	if(pthread_create(&brd->p_bears[tmp], NULL,
	    &bear_pthread, (void *)brd)!=0) {
	    perror("bears:pthread:create");
	    return false;
	}
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

    if(!pthread_mutex_lock(&brd->mutex)) {
	perror("stat:lock()");
	pthread_exit(NULL);
    }

    while(tmp++<brd->size){
	switch(brd->board[tmp]) {
	case 'X': x++; break;
	case 'U': u++; break;
	case 'E': e++; break;
	default:
	    printf("Unknown board symbol found!\n");
	}
    }

    printf("Empty: %d, Bears: %d, Bulls: %d, Total: %d\n",
	x, e, u, x+e+u);

    if(!pthread_mutex_lock(&brd->mutex)) {
	perror("stat:unlock()");
	pthread_exit(NULL);
    }
}

int main(int argc, char** argv, char** env)
{
    if(argc != 4) {
	printf("USAGE:\n\t%s <bulls> <bears> <size>\n", argv[0]);
	return -1;
    };
    
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

    if(!start_game(&brd)) {
	printf("DEBUG: Start failed!\n");
	return -1;
    }

    while(1) {
	show_statistics(&brd);
    }

    return 0;
}