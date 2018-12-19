#ifndef ANT_H
#define ANT_H

#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#include "conf/ants.h"
#include "conf/field.h"
#include "field.h"
#include "rt_thread.h"

extern const float TWO_PI;

typedef enum behaviour {EXPLORING, EXPLOITING, EATING, RESTING} behaviour;

typedef struct position {
	int 			x;			// [pixel] x coordinate
	int 			y;			// [pixel] y coordinate
	float 			angle;		// [radians] angle in [0, 2*pi)
} position;


typedef struct ant {
	bool 			alive;		// is ant allocated? is the data meaningful?
	unsigned int 	tid;		// thread identifier
	position 		pos;		// ant position
	fragrance		interest;	// looking for food or home?
	behaviour		behaviour;	// following a trail or exploring?
	float			excitement;	// intensity of the pheromone to be released
	float 			audacity;	// propensity for explorative decisions
	bool			diverted;	// has the ant recently diverted from its path to explore?
	unsigned int 	expl_desire;// ticks left before the ant stops exploring
	pthread_mutex_t mtx;		// mutex to protect the struct
} ant;


extern ant ants[POP_SIZE_MAX];
extern uint8_t n_ants;
extern pthread_mutex_t ants_mtx;


void init_ants_manager();
int spawn_ant(void);
unsigned int spawn_ants(unsigned int n_ants);
int kill_ant(unsigned int i);
void kill_ants(void);
int get_ant_id_by_pos(int x, int y);


#endif