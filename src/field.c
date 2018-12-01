#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "field.h"


#define MAX(x, y) (((x) > (y)) ? (x) : (y))		// TODO: TO BE MOVED IN CONF/UTILS
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


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


/* x and y are pixel coordinates, radius is cell-sized coordinate
*  Return: best_x in the most significant 16 bits, best_y in the least significant ones.
*/
uint32_t find_smell_direction(int x, int y, int orientation, int radius, SCAN_MODE_T mode, PHERO_T type) {

	assert((x < FIELD_WIDTH) && (y < FIELD_HEIGHT));
	assert((type == PHERO_HOME) || (type == PHERO_FOOD));
	assert((mode == SCAN_ALL)   || (mode == SCAN_FORWARD));

	int cell_i = x / CELL_SIZE;
	int cell_j = y / CELL_SIZE;

	int i = MAX(cell_i - radius, 0);
	int j = MAX(cell_j - radius, 0);
	const int i_max = MIN(cell_i + radius, PH_SIZE_H - 1);
	const int j_max = MIN(cell_j + radius, PH_SIZE_V - 1);

	long ph_best_i = cell_i;
	long ph_best_j = cell_j;
	pthread_mutex_lock(&ph[cell_i][cell_j].mtx);
	float ph_best = (type == PHERO_FOOD) ? ph[cell_i][cell_j].food : ph[cell_i][cell_j].home;
	pthread_mutex_unlock(&ph[cell_i][cell_j].mtx);

	for (i; i < i_max; ++i) {
		for (j; j < j_max; ++j) {
			// Skip check if the cell is out of detection radius
			if (hypot(cell_i - i, cell_j - i) > (double)radius)
				continue;
			// Skip check if SCAN_FORWARD and cell is out of the 180° detection arc
			if (mode == SCAN_FORWARD) {
				double cell_angle = atan2(cell_j - j, i - cell_i);
				if (cos(cell_angle) * cos(orientation) + sin(cell_angle) * sin(orientation) < 0)
					continue;
			}
			pthread_mutex_lock(&ph[i][j].mtx);
			switch (type) {
				case PHERO_FOOD:
					if (ph[i][j].food > ph_best) {
						ph_best_i = i;
						ph_best_j = j;
						ph_best = ph[i][j].food;
					}
					break;
				case PHERO_HOME:
					if (ph[i][j].home > ph_best) {
						ph_best_i = i;
						ph_best_j = j;
						ph_best = ph[i][j].home;
					}
					break;
				default:
					printf("This should not happen! (unrecognized pheromone type)\n");
			}
			pthread_mutex_unlock(&ph[i][j].mtx);
		}
	}
	return ((((CELL_SIZE / 2) + CELL_SIZE * ph_best_i) & 0xFFFF) << 16) +
		    (((CELL_SIZE / 2) + CELL_SIZE * ph_best_j) & 0xFFFF);
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
