#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include "field.h"


int tid;			// thread id

pheromone ph[PH_SIZE_H][PH_SIZE_V];


static inline void phero_exp_decay(int i, int j) {

	pthread_mutex_lock(&ph[i][j].mtx);

    ph[i][j].food = 2.1;		// STUB
    ph[i][j].home = 3.4;		// STUB

    pthread_mutex_unlock(&ph[i][j].mtx);
}


void *decay_behaviour(void *arg) {

	for (int i = 0; i < PH_SIZE_H; ++i)
        for (int j = 0; j < PH_SIZE_V; ++j)
            phero_exp_decay(i, j);
}


unsigned int start_pheromones() {

	// make sure CELL_SIZE divides the field dimensions
	assert(FIELD_WIDTH  % CELL_SIZE == 0);
	assert(FIELD_HEIGHT % CELL_SIZE == 0);

	for (int i = 0; i < PH_SIZE_H; ++i) {
		for (int j = 0; j < PH_SIZE_V; ++j) {
			pthread_mutex_init(&ph[i][j].mtx, NULL);
			ph[i][j].food = ph[i][j].home = 0.0;
		}
	}

	tid = start_thread(decay_behaviour, NULL, SCHED_FIFO, WCET_DECAY, PRD_DECAY, DL_DECAY, PRIO_DECAY);
	if (tid < 0) {
		printf("Failed to start pheromone decay thread\n");
		return 1;
	} else 
		printf("Started pheromone decay thread with id #%d\n", tid);

	return 0;
}

void stop_pheromones() {

	stop_thread(tid);
	printf("Pheromones decay thread stopped.\n");
}
