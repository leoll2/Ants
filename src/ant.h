#ifndef ANT_H
#define ANT_H

#include <pthread.h>

struct state {
	uint16_t x;
	uint16_t y;
	float angle;
};


pthread_t ant1, ant2;

void *ant_behaviour(void *p);


int spawn_ants(unsigned int n_ants);

void kill_ants(void);


#endif