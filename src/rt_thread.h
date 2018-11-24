#ifndef RT_THREAD_H
#define RT_THREAD_H

#include <pthread.h>


typedef struct task_par {
	pthread_t tid;	/* thread id */
	void *container;/* task argument */
	long wcet; 		/* in microseconds */
	int period; 	/* in milliseconds */
	int deadline; 	/* relative (ms) */
	int priority; 	/* in [0,99] */
	int dmiss; 		/* no. of misses */
	short stopped;	/* thread was ordered to stop */
	struct timespec at; /* next activ. time */
	struct timespec dl; /* abs. deadline */
} task_par;



void time_copy(struct timespec *dst, struct timespec src);

void time_add_ms(struct timespec *t, int ms);

int time_cmp(struct timespec t1, struct timespec t2);

int start_thread(
		void *(*func)(void *), // routine code
		void *container,// routine args
		task_par *tp, 	// task scheduling parameters (will be initialized)
		int policy,		// scheduling policy
		long wcet,		// worst case execution time (us)
		int prd,		// period (ms)
		int dl,			// relative deadline (ms)
		int prio 		// priority [0,99]
);


#endif