#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ant.h"

uint8_t stop = 0;

uint8_t n_ants = 0;


void *ant_behaviour(void *container) {

	struct timespec dt;

	ant *a = (ant *)container;

	while(!a->thr.stopped) {
		printf("Thread: %d  x: %d  y: %d  stopped: %d\n", pthread_self(), a->pos.x, a->pos.y, a->thr.stopped);
		/*dt.tv_sec = 2;
		dt.tv_nsec = 0;
		clock_nanosleep(CLOCK_MONOTONIC, 0, &dt, NULL);*/
	}

	printf("Stopping thread %d \n", pthread_self());
}



int spawn_ants(unsigned int n_ants) {

	int ret;

	for (int i = 0; i < POP_MAX; ++i) {
		ret = start_thread(ant_behaviour, &ants[i], &ants[i].thr, SCHED_FIFO, 50, 100, 80, 25);
		if (ret) {
			printf("Failed to instantiate ant #%d (error: %d)\n", i, ret);
			return 1;
		}
		++n_ants;	// TODO: ADD LOCK
	}

	return 0;
}


void kill_ants(void) {


	printf("Start killing \n");
	exit(5);

	for (int i = 0; i < n_ants; ++i) {
		ants[i].thr.stopped = 1;
		pthread_join(ants[i].thr.tid, NULL);
		--n_ants;	// TODO: ADD LOCK
	}

	if (n_ants > 0)
		printf("For some reason %d ants remained alive\n", n_ants);

	printf("Finished killing \n");
}