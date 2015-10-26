#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sched.h>

#define CHILD_STACK_SIZE	(16384)

typedef struct {
    double begin;
    double end;
    double delta;
    double result;
} i_param;

/* implementation of y=f(x) */
double f(double x)
{
#ifdef _TEST_MATH_
    return x;
#else
    return sin(x);
#endif
}

int summator(void* arg)
{
    i_param* param = (i_param *)arg;
    param->result = param->delta * (f(param->end) + f(param->begin))/2;
    return 0;
}

char		*stacks;
pid_t		*pids;
i_param   	*params;

int main(int argc, char** argv, char** env)
{
    double begin, end, temp, answer;
    int	tmp, iterations;

    if(argc != 3){
	printf("USAGE\n\t%s <interval> <number_of_pthreads>\n", argv[0]);
	return -1;
    }

    begin = 0.0;
    answer = 0.0;
    if(sscanf(argv[1], "%lf", &end) != 1 || end < 0.0) {
	printf("Invalid interval: %s\n", argv[1]);
	return -1;
    }
    if(sscanf(argv[2], "%d", &iterations) != 1 || iterations < 1) {
	printf("Invalid number of pthreads: %s\n", argv[2]);
	return -1;
    }
    printf("Calculate sum(0..%lf)f(x) in %d steps\n", end, iterations);

    /* allocate all needed memory */
    if((pids = (pid_t*)malloc(iterations * sizeof(pid_t))) == NULL) {
	perror("malloc pids");
	return -1;
    }

    if((stacks = malloc(iterations * CHILD_STACK_SIZE)) == NULL) {
	perror("malloc stack");
	return -1;
    }

    if((params = malloc(iterations * sizeof(i_param))) == NULL) {
	perror("malloc params");
	return -1;
    }

    /* allocate params and start pthreads */
    for(tmp=0; tmp<iterations; tmp++) {
	params[tmp].begin = begin;
	params[tmp].delta = end/iterations;
	begin += params[tmp].delta; /* new begin/ actual end calculations */
	params[tmp].end = begin;

	pids[tmp] = clone(summator,
		stacks + tmp*CHILD_STACK_SIZE + CHILD_STACK_SIZE,
		CLONE_VM,
		&params[tmp]);
	if(pids[tmp] < 0) {
	    perror("clone");
	    return -1;
	} else {
	    printf("Debug: child %d created\n", pids[tmp]);
	}
    }

    /* wait pthreads and calculate result  */
    for(tmp=0; tmp<iterations; tmp++) {
	pid_t ret;
    	printf("Waiting for %d: ", pids[tmp]);
	do {
    	    ret = waitpid(pids[tmp],NULL, __WCLONE);
    	    switch(ret){
    		case 0: 
    		    printf(".");
    		    break;
    		case -1: 
    		    printf("?");
    		    break;
    		default:
    		    printf("[DONE]\n");
    		    break;
    	    };
	} while(ret == 0 || ret == -1);
	answer += params[tmp].result;
    } 

    /* free allocated memory */
    free(stacks);
    free(params);
    free(pids);

    printf("Calculated result is : %lf\n", answer);

    return 0;
}
