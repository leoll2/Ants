#ifndef RT_THREAD_H
#define RT_THREAD_H

#include <pthread.h>
#include <stdbool.h>


#define MAX_THREADS		256


typedef struct task_par {

	bool in_use;		// structure is free or in use
	pthread_mutex_t mtx;// lock on the structure
	pthread_t tid;		// thread id
	void *(*behaviour)(void *);	// behaviour code
	void *data;			// behaviour-specific data
	long wcet; 			// worst case execution time (us)
	int period; 		// period (ms)
	int deadline; 		// relative deadline (ms)
	int priority; 		// priority [0,99]
	int dl_missed;		// no. of deadline misses
	bool stopped;		// thread was ordered to stop
	struct timespec at; // next activ. time
	struct timespec dl; // abs. deadline
} task_par;


void init_rt_thread_manager(void);

int start_thread(
		void *(*func)(void *), // routine code
		void *args,		// routine args
		int policy,		// scheduling policy
		long wcet,		// worst case execution time (us)
		int prd,		// period (ms)
		int dl,			// relative deadline (ms)
		int prio 		// priority [0,99]
);

int stop_thread(unsigned int id);

#endif