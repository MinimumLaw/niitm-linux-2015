#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

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

void* summator(void* arg)
{
    i_param* param = (i_param *)arg;
    param->result = param->delta * (f(param->end) + f(param->begin))/2;
    return arg; /* FixMe: pthread_exit(arg); not needed ??? */
}

pthread_t *pth_pid;
i_param   *params;

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
    if((pth_pid = malloc(iterations * sizeof(pthread_t))) == NULL) {
	perror("malloc pids");
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

	if(pthread_create(&pth_pid[tmp], NULL, &summator, &params[tmp])) {
	    perror("pthread_create");
	    return -1;
	}
    }

    /* wait pthreads and calculate result */
    for(tmp=0; tmp<iterations; tmp++) {
	if(pthread_join(pth_pid[tmp], NULL)) {
	    perror("pthread_join");
	    return -1;
	} else {
	    answer += params[tmp].result;
	}
    }

    /* free allocated memory */
    free(params);
    free(pth_pid);

    printf("Calculated result is : %lf\n", answer);

    return 0;
}
