#include <stdio.h>
#include <string.h>

#include "rt_thread.h"


void time_copy(struct timespec *dst, struct timespec src) {
	
	dst->tv_sec  = src.tv_sec;
	dst->tv_nsec = src.tv_nsec;
}


void time_add_ms(struct timespec *t, int ms) {

	t->tv_sec += ms/1000;
	t->tv_nsec += (ms%1000)*1000000;
	if (t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}


int time_cmp(struct timespec t1, struct timespec t2) {
	
	if (t1.tv_sec > t2.tv_sec) return 1;
	if (t1.tv_sec < t2.tv_sec) return -1;
	if (t1.tv_nsec > t2.tv_nsec) return 1;
	if (t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}

/*
* Initialize pthread_attr_t structure properly
*/
int init_sched_attr(pthread_attr_t *attr, int policy, int prio) {

	struct sched_param sp;
	int err;

	err = pthread_attr_init(attr);
	if (err)
		return 1;

	err = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
	if (err)
		return 2;

	err = pthread_attr_setschedpolicy(attr, policy);
	if (err)
		return 3;

	sp.sched_priority = prio;
	err = pthread_attr_setschedparam(attr, &sp);
	if (err)
		return 4;

	return 0;
}


int start_thread(
		void *(*func)(void *), 
		void *container,
		task_par *tp,
		int policy, 
		long wcet,
		int prd,
		int dl,
		int prio)
{
	pthread_attr_t attr;
	int err;

	tp->container = container;
	tp->wcet = wcet;
	tp->period = prd;
	tp->deadline = dl;
	tp->priority = prio;
	tp->stopped = 0;
	tp->dmiss = 0;

	err = init_sched_attr(&attr, policy, prio);
	if (err) {
		printf("Init of sched attributes failed (error: %d)\n", err);
		return 1;
	}

	err = pthread_create(&tp->tid, &attr, func, (void*)container);
	if (err) {
		printf("Thread creation failed (error: %s)\n", strerror(err));
		return 2;
	}

	err = pthread_attr_destroy(&attr);
	if (err) {
		printf("Destruction of sched attributes failed (error: %d)\n", err);
		return 3;
	}

	return 0;
}