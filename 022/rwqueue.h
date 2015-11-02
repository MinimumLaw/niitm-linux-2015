#ifndef _INCLUDE_RWQUEUE_H_
#define _INCLUDE_RWQUEUE_H_

/* unlink require full path */
#define SHM_MOUNT_PATH "/dev/shm/"

#define SHM_BOARD "me.noip.minimumlaw.rwqueue.board.bin"
#define SHM_RULES "me.noip.minimumlaw.rwqueue.rules.bin"

typedef struct {
	pthread_mutex_t	mutex;
	bool	need_exit;
	pthread_t *p_bulls;
	pthread_t *p_bears;
	int	bulls;
	int	bears;
	int	size;
} game_rules;

typedef char	game_board;

#endif
