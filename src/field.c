#include <assert.h>
#include <pthread.h>
#include <stdio.h>

#include "field.h"


int tid;			// thread id

cell ph[PH_SIZE_H][PH_SIZE_V];


void deploy_pheromone(unsigned int id, int x, int y, PHERO_T type) {

	assert((x < FIELD_WIDTH) && (y < FIELD_HEIGHT));
	assert((type == PHERO_HOME) || (type == PHERO_FOOD));

	int i = x / CELL_SIZE;
	int j = y / CELL_SIZE;

	pthread_mutex_lock(&ph[i][j].mtx);

	switch(type) {
		case PHERO_FOOD:
			if (ph[i][j].backoff_food == 0) {
				ph[i][j].food += SMELL_UNIT;		// ADD HOME TOO!
				if (ph[i][j].food > SMELL_UB)
					ph[i][j].food = SMELL_UB;
				ph[i][j].backoff_food = DEPLOY_BACKOFF;
			}
			break;
		case PHERO_HOME:
			if (ph[i][j].backoff_home == 0) {
				ph[i][j].home += SMELL_UNIT;		// ADD HOME TOO!
				if (ph[i][j].home > SMELL_UB)
					ph[i][j].home = SMELL_UB;
				ph[i][j].backoff_home = DEPLOY_BACKOFF;
			}
			break;
		default:
			printf("This should not happen! (unrecognized pheromone type)\n");
	}
	pthread_mutex_unlock(&ph[i][j].mtx);
}


static inline void phero_exp_decay(int i, int j) {

	pthread_mutex_lock(&ph[i][j].mtx);

	if (ph[i][j].backoff_home)
		ph[i][j].backoff_home--;
	if (ph[i][j].backoff_food)
		ph[i][j].backoff_food--;

    ph[i][j].food *= DECAY_FACTOR;
    if (ph[i][j].food < SMELL_THRESH)
    	ph[i][j].food = 0.0;
    ph[i][j].home *= DECAY_FACTOR;
    if (ph[i][j].home < SMELL_THRESH)
    	ph[i][j].home = 0.0;

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
			ph[i][j].backoff_home = ph[i][j].backoff_food = 0;
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
