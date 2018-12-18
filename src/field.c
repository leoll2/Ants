#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "field.h"


#define MAX(x, y) (((x) > (y)) ? (x) : (y))		// TODO: TO BE MOVED IN CONF/UTILS
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


int tid;			// pheromone decay thread id
cell ph[PH_SIZE_H][PH_SIZE_V];
food foods[MAX_FOOD_SRC];
unsigned int n_food_src;


void deploy_pheromone(unsigned int id, int x, int y, phero_type type, float value) {

	assert((x < FIELD_WIDTH) && (y < FIELD_HEIGHT));
	assert((type == HOME) || (type == FOOD));
	assert((value >= 0) && (value <= SMELL_UNIT));

	if (value < SMELL_THRESH)
		return;

	int i = x / CELL_SIZE;
	int j = y / CELL_SIZE;

	pthread_mutex_lock(&ph[i][j].mtx);

	switch(type) {
		case FOOD:
			if (ph[i][j].backoff_food == 0) {
				ph[i][j].food = MAX(ph[i][j].food, value);
				ph[i][j].backoff_food = DEPLOY_BACKOFF;
			}
			break;
		case HOME:
			if (ph[i][j].backoff_home == 0) {
				ph[i][j].home = MAX(ph[i][j].home, value);
				ph[i][j].backoff_home = DEPLOY_BACKOFF;
			}
			break;
		default:
			printf("This should not happen! (unrecognized pheromone type)\n");
	}
	pthread_mutex_unlock(&ph[i][j].mtx);
}




visual_scan find_target_visually(int x, int y, int radius, phero_type desired_type) {

	visual_scan res;
	res.success = false;
	res.oth_obj_found = false;

	// Check if there is any source of food nearby
	for (int i = 0; i < MAX_FOOD_SRC; ++i) {
		pthread_mutex_lock(&foods[i].mtx);
		if (foods[i].units > 0 && hypot(foods[i].x - x, foods[i].y - y) <= radius) {
			switch(desired_type) {
				case FOOD:
					res.success = true;
					res.target_x = foods[i].x;
					res.target_y = foods[i].y;
					break;
				case HOME:
					res.oth_obj_found = true;
					break;
				default:
					printf("This should not happen! (unrecognized pheromone type)\n");
			}
			pthread_mutex_unlock(&foods[i].mtx);
			break;
		} else {
			pthread_mutex_unlock(&foods[i].mtx);
		}
	}

	// Check if anthill is nearby
	if (hypot(x - HOME_X, y - HOME_Y) <= radius) {
		switch(desired_type) {
			case HOME:
				res.success = true;
				res.target_x = HOME_X;
				res.target_y = HOME_Y;
				break;
			case FOOD:
				res.oth_obj_found = true;
				break;
			default:
				printf("This should not happen! (unrecognized pheromone type)\n");
		}
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


unsigned int start_pheromones(void) {

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


void stop_pheromones(void) {

	stop_thread(tid);
	printf("Pheromones decay thread stopped.\n");
}


int consume_food(int x, int y) {

	if ((x < 0) || (x >= FIELD_WIDTH) || (y < 0) || (y >= FIELD_HEIGHT))
		return -2;

	// For each food source
	for (int i = 0; i < MAX_FOOD_SRC; ++i) {
		pthread_mutex_lock(&foods[i].mtx);

		// Check if it actually exists
		if (foods[i].units == 0) {
			pthread_mutex_unlock(&foods[i].mtx);
			continue;
		}

		// Check if the coordinates match
		if ((foods[i].x != x) || foods[i].y != y) {
			pthread_mutex_unlock(&foods[i].mtx);
			continue;
		}

		// Consume one unit of food
		if (--foods[i].units == 0)
			--n_food_src;
		pthread_mutex_unlock(&foods[i].mtx);
		return 0;
	}
	return -1;
}


int deploy_food(int x, int y) {

	if ((x < 0) || (x >= FIELD_WIDTH) || (y < 0) || (y >= FIELD_HEIGHT))
		return -2;

	for (int i = 0; i < MAX_FOOD_SRC; ++i) {
		pthread_mutex_lock(&foods[i].mtx);
		if (foods[i].units == 0) {
			foods[i].units = FOOD_UNITS;
			foods[i].x = x;
			foods[i].y = y;
			++n_food_src;
			pthread_mutex_unlock(&foods[i].mtx);
			return 0;
		}
		pthread_mutex_unlock(&foods[i].mtx);
	}

	return -1;
}


void init_foods(void) {

	for (int i = 0; i < MAX_FOOD_SRC; ++i) {
		pthread_mutex_init(&foods[i].mtx, NULL);
		foods[i].units = 0;
	}
}