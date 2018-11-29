#ifndef FIELD_H
#define FIELD_H

#include "conf/field.h"
#include "conf/pheromones.h"
#include "rt_thread.h"

#define PHERO_T			unsigned int
#define PHERO_HOME		0
#define PHERO_FOOD		1

#define PH_SIZE_H 		(FIELD_WIDTH  / CELL_SIZE)
#define PH_SIZE_V		(FIELD_HEIGHT / CELL_SIZE)


typedef struct cell {
	float food;		// food scent intensity
	float home;		// anthill scent intensity
	unsigned int backoff_food;	// cooldown before food pheromone can be deployed
	unsigned int backoff_home;	// cooldown before food pheromone can be deployed 
	pthread_mutex_t mtx;
} cell;

extern cell ph[PH_SIZE_H][PH_SIZE_V];

void deploy_pheromone(unsigned int id, int x, int y, PHERO_T type);

unsigned int start_pheromones();

void stop_pheromones();


#endif