#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "rwqueue.h"

game_rules	*rules;
game_board	*board;

int show_statistics(game_rules *rules, game_board *board)
{
    int x,e,u,tmp;

    x=e=u=tmp=0;

    while(tmp<rules->size){
	switch(board[tmp++]) {
	case 'X': x++; break;
	case 'U': u++; break;
	case 'E': e++; break;
	default:
	    printf("Unknown board symbol found!\n");
	}
    }

    printf("Rules: Bears: %d, Bulls: %d, Board size: %d\n",
	rules->bears, rules->bulls, rules->size);
    printf("Status: Empty: %d, Bears: %d, Bulls: %d, Total: %d\n",
	x, e, u, x+e+u);

    return 0;
}

int main(int argc, char** argv, char** env)
{
    int rule_fd;
    int board_fd;

    rule_fd = shm_open(SHM_RULES,O_RDWR,0666);
    if(rule_fd == -1){
	perror("shm_open::rules");
	printf("M.b. not running?\n");
	return -1;
    };

    rules = mmap(NULL, sizeof(game_rules), PROT_READ | PROT_WRITE, MAP_PRIVATE, rule_fd, 0);
    if(rules == MAP_FAILED){
	perror("mmap::rules");
	return -1;
    }

    board_fd = shm_open(SHM_BOARD,O_RDWR,0666);
    if(board_fd == -1){
	perror("shm_open::board");
	printf("M.b. not creating? Try again!\n");
	return -1;
    };

    board = mmap(NULL, rules->size, PROT_READ, MAP_PRIVATE, board_fd, 0);
    if(board == MAP_FAILED){
	perror("mmap::board");
	return -1;
    }

    show_statistics(rules, board);

    munmap(board, rules->size);
    munmap(rules, sizeof(game_rules));

    close(board_fd);
    close(rule_fd);

    return 0;
}
