#include <math.h>
#include <stdio.h>

#include "ant.h"
#include "field.h"

const float TWO_PI = 2 * M_PI;

ant ants[POP_SIZE_MAX];
uint8_t n_ants = 0;
pthread_mutex_t ants_mtx = PTHREAD_MUTEX_INITIALIZER;


void init_ant(ant *const a, int id) {

	a->alive = true;
	a->id = (unsigned int)id;
	a->pos.x = a->pos.y = 0;
	a->pos.angle = rand() * TWO_PI;
}


void move_ant_random(ant *const a) {	// DEBUG PURPOSE
	
	a->pos.x = rand() % FIELD_WIDTH;
	a->pos.y = rand() % FIELD_HEIGHT;
}


void move_ant(ant *const a) {

	a->pos.x += (int)round(STEP_LENGTH * cos(a->pos.angle));
	if (a->pos.x <= 0) {
		a->pos.x *= -1;
		a->pos.angle = fmod(M_PI - a->pos.angle, TWO_PI);
	}
	else if (a->pos.x >= FIELD_WIDTH - 1) {
		a->pos.x -= 2 * (a->pos.x - FIELD_WIDTH + 1);
		a->pos.angle = fmod(M_PI - a->pos.angle, TWO_PI);
	}

	a->pos.y -= (int)round(STEP_LENGTH * sin(a->pos.angle));
	if (a->pos.y <= 0) {
		a->pos.y *= -1;
		a->pos.angle = fmod(-a->pos.angle, TWO_PI);
	}
	else if (a->pos.y >= FIELD_HEIGHT - 1) {
		a->pos.y -= 2 * (a->pos.y - FIELD_HEIGHT + 1);
		a->pos.angle = fmod(-a->pos.angle, TWO_PI);
	}

	a->pos.angle = fmod(a->pos.angle + (EXPL_CONE * (double)rand() / RAND_MAX - EXPL_CONE / 2), TWO_PI);
	//printf("ant: %d  x:%d  y: %d  angle: %f\n", a->id, a->pos.x, a->pos.y, a->pos.angle);
}


void *ant_behaviour(void *arg) {

	ant *const a = (ant *)arg;

	pthread_mutex_lock(&a->mtx);

	//move_ant_random(a);
	move_ant(a);
	deploy_pheromone(a->id, a->pos.x, a->pos.y, PHERO_FOOD);
	deploy_pheromone(a->id, a->pos.x, a->pos.y, PHERO_HOME);

	pthread_mutex_unlock(&a->mtx);
}


int spawn_ants(unsigned int n) {

	int id;

	pthread_mutex_lock(&ants_mtx);

	// clear the alive flag of all ants and init mutex
	for (int i = 0; i < n; ++i) {
		pthread_mutex_init(&ants[i].mtx, NULL);
		ants[i].alive = false;
	}

	for (int i = 0; i < n; ++i) {
		pthread_mutex_lock(&ants[i].mtx);	// prevents the ant from running before initialization
		id = start_thread(ant_behaviour, &ants[i], SCHED_FIFO, WCET_ANTS, PRD_ANTS, DL_ANTS, PRIO_ANTS);
		if (id < 0) {
			pthread_mutex_unlock(&ants[i].mtx);
			pthread_mutex_unlock(&ants_mtx);
			printf("Failed to instantiate ant #%d\n", i);
			return 1;
		} else {
			init_ant(&ants[i], id);
			pthread_mutex_unlock(&ants[i].mtx);
			++n_ants;	// TODO: ADD LOCK ON GLOBAL STRUCTURE
			printf("Created ant %d with id #%d\n", i, id);
		}
	}

	pthread_mutex_unlock(&ants_mtx);

	return 0;
}


void kill_ants(void) {

	int iter = n_ants;
	printf("Start killing ants \n");

	pthread_mutex_lock(&ants_mtx);

	for (int i = 0; i < iter; ++i) {
		stop_thread(ants[i].id);
		ants[i].alive = false;
		--n_ants;	// TODO: ADD LOCK
	}

	if (n_ants > 0)
		printf("For some reason %d ants remained alive\n", n_ants);

	pthread_mutex_unlock(&ants_mtx);

	printf("Finished killing ants \n");
}