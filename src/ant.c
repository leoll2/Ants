#include <stdio.h>

#include "ant.h"

ant ants[POP_SIZE_MAX];
uint8_t n_ants = 0;
pthread_mutex_t ants_mtx = PTHREAD_MUTEX_INITIALIZER;


void *ant_behaviour(void *arg) {

	ant *a = (ant *)arg;

	pthread_mutex_lock(&a->mtx);

	a->pos.x = rand()%640;
	a->pos.y = rand()%480;
	//printf("Running ant %d\n", a->id);

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
		pthread_mutex_lock(&ants[i].mtx);
		id = start_thread(ant_behaviour, &ants[i], SCHED_FIFO, WCET_ANTS, PRD_ANTS, DL_ANTS, PRIO_ANTS);
		if (id < 0) {
			pthread_mutex_unlock(&ants[i].mtx);
			pthread_mutex_unlock(&ants_mtx);
			printf("Failed to instantiate ant #%d\n", i);
			return 1;
		} else {
			ants[i].alive = true;
			ants[i].id = (unsigned int)id;
			ants[i].pos.x = ants[i].pos.y = ants[i].pos.angle = 0;
			pthread_mutex_unlock(&ants[i].mtx);
			++n_ants;	// TODO: ADD LOCK
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