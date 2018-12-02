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


void deploy_pheromone(unsigned int id, int x, int y, phero_type type, float value) {

	assert((x < FIELD_WIDTH) && (y < FIELD_HEIGHT));
	assert((type == HOME) || (type == FOOD));
	assert(value >= 0);

	if (value < SMELL_THRESH)
		return;

	int i = x / CELL_SIZE;
	int j = y / CELL_SIZE;

	pthread_mutex_lock(&ph[i][j].mtx);

	switch(type) {
		case FOOD:
			if (ph[i][j].backoff_food == 0) {
				ph[i][j].food += value;		// ADD HOME TOO!
				if (ph[i][j].food > SMELL_UB)
					ph[i][j].food = SMELL_UB;
				ph[i][j].backoff_food = DEPLOY_BACKOFF;
			}
			break;
		case HOME:
			if (ph[i][j].backoff_home == 0) {
				ph[i][j].home += value;		// ADD HOME TOO!
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


visual_scan find_target_visually(int x, int y, int radius, phero_type type) {

	visual_scan res;
	res.success = false;

	switch (type) {
		case FOOD:
			if (hypot(x - FOOD_X, y - FOOD_Y) <= radius) {
				res.success = true;
				res.target_x = FOOD_X;
				res.target_y = FOOD_Y;
			}
			break;
		case HOME:
			if (hypot(x - HOME_X, y - HOME_Y) <= radius) {
				res.success = true;
				res.target_x = HOME_X;
				res.target_y = HOME_Y;
			}
			break;
		default:
			printf("This should not happen! (unrecognized pheromone type)\n");
	}

	return res;
}


/* x and y are pixel coordinates, radius is cell-sized coordinate
*/
smell_scan find_smell_direction(int x, int y, int orientation, int radius, 
								scan_mode mode, phero_type type) 
{
	assert((x < FIELD_WIDTH) && (y < FIELD_HEIGHT));
	assert((type == HOME) || (type == FOOD));
	assert((mode == FULL) || (mode == FORWARD));

	smell_scan res;					// result variable

	int cell_i = x / CELL_SIZE;		// cell corresponding to the current position
	int cell_j = y / CELL_SIZE;		

	// range of cells to be scanned
	const int i_min = MAX(cell_i - radius, 0);
	const int j_min = MAX(cell_j - radius, 0);
	const int i_max = MIN(cell_i + radius, PH_SIZE_H - 1);
	const int j_max = MIN(cell_j + radius, PH_SIZE_V - 1);

	// Initialize the result variable with the current cell values
	res.local_optimum = true;
	res.opt_x = cell_i * CELL_SIZE + (CELL_SIZE / 2);
	res.opt_y = cell_j * CELL_SIZE + (CELL_SIZE / 2);
	pthread_mutex_lock(&ph[cell_i][cell_j].mtx);
	res.opt_val = (type == FOOD) ? ph[cell_i][cell_j].food : 
								   ph[cell_i][cell_j].home;
	res.success = (type == FOOD) ? ph[cell_i][cell_j].food > 0 : 
								   ph[cell_i][cell_j].home > 0;
	pthread_mutex_unlock(&ph[cell_i][cell_j].mtx);

	// Scan nearby cells to find the one with the highest pheromone intensity
	for (int i = i_min; i <= i_max; i++) {
		for (int j = j_min; j <= j_max; j++) {
			// Skip check if the cell is out of detection radius
			if (hypot(cell_i - i, cell_j - j) > (double)radius)
				continue;
			// Skip check if scan mode is FORWARD and cell is out of the 180° detection arc
			if (mode == FORWARD) {
				double cell_angle = atan2(cell_j - j, i - cell_i);
				if (cos(cell_angle) * cos(orientation) + sin(cell_angle) * sin(orientation) < 0)
					continue;
			}
			// Get the pheromone intensity of the cell
			pthread_mutex_lock(&ph[i][j].mtx);
			float ph_value;
			switch (type) {
				case FOOD:
					ph_value = ph[i][j].food;
					break;
				case HOME:
					ph_value = ph[i][j].home;
					break;
				default:
					printf("This should not happen! (unrecognized pheromone type)\n");
			}
			pthread_mutex_unlock(&ph[i][j].mtx);
			// Update the result variable if that intensity is the highest so far
			if ((ph_value > 0) && (ph_value > res.opt_val)) {
				res.success = true;
				res.local_optimum = false;
				res.opt_x = i * CELL_SIZE + (CELL_SIZE / 2);
				res.opt_y = j * CELL_SIZE + (CELL_SIZE / 2);
				res.opt_val = ph_value;
			}
		}
	}
	return res;
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
