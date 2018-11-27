#ifndef FIELD_H
#define FIELD_H

#include "conf/field.h"
#include "conf/pheromones.h"
#include "rt_thread.h"


#define PH_SIZE_H 		(FIELD_WIDTH  / CELL_SIZE)
#define PH_SIZE_V		(FIELD_HEIGHT / CELL_SIZE)


typedef struct pheromone {
	float food;		// smell indicating food
	float home;		// smell indicating home
	pthread_mutex_t mtx;
} pheromone;

extern pheromone ph[PH_SIZE_H][PH_SIZE_V];

unsigned int start_pheromones();

void stop_pheromones();


#endif