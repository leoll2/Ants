#ifndef ANT_H
#define ANT_H

#include <pthread.h>

#include "conf/population.h"
#include "rt_thread.h"

typedef struct position {
	uint16_t x;
	uint16_t y;
	float angle;
} position;


typedef struct ant {
	position pos;
	task_par thr;
} ant;

ant ants[POP_MAX];



void *ant_behaviour(void *p);


int spawn_ants(unsigned int n_ants);

void kill_ants(void);


#endif