#include <math.h>
#include <stdio.h>
#include <allegro/keyboard.h>
#include "ant.h"

const float TWO_PI = 2 * M_PI;

ant ants[POP_SIZE_MAX];
uint8_t n_ants = 0;
pthread_mutex_t ants_mtx = PTHREAD_MUTEX_INITIALIZER;
int numero;


void init_ant(ant *const a, int tid) {

	a->alive = 		true;
	a->tid = 		(unsigned int)tid;
	a->pos.x = 		HOME_X;
	a->pos.y = 		HOME_Y;
	a->pos.angle = 	rand() * TWO_PI;
	a->interest = 	FOOD;
	a->behaviour = 	RESTING;
	a->excitement = 1.0;
	a->audacity = 	AUDACITY;
	a->diverted = 	false;
	a->expl_desire = 0;
}


void move_ant_random(ant *const a) {	// DEBUG PURPOSE

	a->pos.x = rand() % FIELD_WIDTH;
	a->pos.y = rand() % FIELD_HEIGHT;
}


static inline void reverse_direction(ant *const a) {

	a->pos.angle = fmod((a->pos.angle + M_PI), TWO_PI);
}



static inline void divert_direction(ant *const a) {

	a->diverted = true;
	if (rand() % 2)
		a->pos.angle = fmod((a->pos.angle + M_PI / 2), TWO_PI);
	else
		a->pos.angle = fmod((a->pos.angle + 3 * M_PI / 2), TWO_PI);
}


static inline bool want_to_explore(float audacity) {

	return (((double)rand() / RAND_MAX) < audacity);
}



void advancement_step(ant *const a) {

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
	if (a->pos.angle < 0)
		a->pos.angle += TWO_PI;
}


/* Return true if the target has been reached, false if yet to be reached. */
bool tracking_step(ant *const a, int target_x, int target_y) {

	// Return if ant is already in the target position
	if ((a->pos.x == target_x) && (a->pos.y == target_y))
		return true;

	// Align with the target
	a->pos.angle = fmod(atan2(a->pos.y - target_y, target_x - a->pos.x), TWO_PI);
	if (a->pos.angle < 0)
		a->pos.angle += TWO_PI;

	// If target is close enough to be directly reached, go there
	if (hypot(target_x - a->pos.x, target_y - a->pos.y) <= STEP_LENGTH) {
		a->pos.x = target_x;
		a->pos.y = target_y;
		return true;
	} else {
		a->pos.x += STEP_LENGTH * cos(a->pos.angle);
		a->pos.y -= STEP_LENGTH * sin(a->pos.angle);
		return false;
	}
}


void *ant_routine(void *arg) {

	ant *const a = (ant *)arg;

	visual_scan v_scan;
	smell_scan  s_scan;

	pthread_mutex_lock(&a->mtx);

	switch (a->behaviour) {
		case RESTING:
			reverse_direction(a);
			a->interest = FOOD;
			a->behaviour = EXPLOITING;
			a->audacity *= AUDACITY_DISCOUNT;
			a->excitement = 1.0;
			break;
		case EATING:
			reverse_direction(a);
			a->interest = HOME;
			a->behaviour = EXPLOITING;
			a->audacity *= AUDACITY_DISCOUNT;
			a->excitement = 1.0;
			break;
		case EXPLORING:
			if (!a->diverted)
				divert_direction(a);
			advancement_step(a);
			a->expl_desire--;
			if (a->expl_desire == 0) {
				a->diverted = false;
				a->behaviour = EXPLOITING;
			}
			break;
		case EXPLOITING:
			// Is the target near enough to be seen?
			v_scan = find_target_visually(a->pos.x, a->pos.y, VISION_RADIUS, a->interest);
			if (v_scan.success) {
				if (tracking_step(a, v_scan.target_x, v_scan.target_y))
					a->behaviour = (a->interest == FOOD) ? EATING : RESTING;
				break;
			}
			// Are there pheromones nearby?
			s_scan = find_smell_direction(a->pos.x, a->pos.y, a->pos.angle,
					OLFACTION_RADIUS, FULL, a->interest
			);
			if (s_scan.success && !s_scan.local_optimum) {
				if (want_to_explore(a->audacity)) {
					a->expl_desire = EXPL_DURATION;
					a->behaviour = EXPLORING;
				} else {
					tracking_step(a, s_scan.opt_x, s_scan.opt_y);
				}
			} else if (s_scan.success && s_scan.local_optimum) {
				// Explore to escape from local minimum
				a->expl_desire = 3;
				a->behaviour = EXPLORING;
			} else {
				advancement_step(a);
			}
			break;
		default:
			printf("This should not happen! (unrecognized ant behaviour)\n");
	}

	deploy_pheromone(a->tid, a->pos.x, a->pos.y,
					 a->interest == FOOD ? HOME : FOOD,
					 a->excitement * SMELL_UNIT
	);
	a->excitement *= DEPLOY_FACTOR;

	pthread_mutex_unlock(&a->mtx);
}



/* Allocates a previously unused ant structure and returns its index.
*  If none is free, return POP_SIZE_MAX.
*/
unsigned int allocate_ant_id(void) {

	unsigned int i = 0;

	pthread_mutex_lock(&ants_mtx);

	if (n_ants == POP_SIZE_MAX) {
		pthread_mutex_unlock(&ants_mtx);
		return POP_SIZE_MAX;
	}

	while (ants[i].alive)
		++i;

	ants[i].alive = true;
	++n_ants;

	pthread_mutex_unlock(&ants_mtx);
	return i;
}



/* Deallocates the ant struct of a terminated ant. */
void deallocate_ant_id(unsigned int id) {

	pthread_mutex_lock(&ants_mtx);
	--n_ants;
	ants[id].alive = false;
	pthread_mutex_unlock(&ants_mtx);
}



/* Add a new ant to the current population.
*  Returns 0 if success, -1 if population is full, -2 if thread pool is full.
*/
int spawn_ant(void) {

	unsigned int a_id;		// id of the new ant (index in ants array)
	int t_id;				// if the new thread

	a_id = allocate_ant_id();
	if (a_id == POP_SIZE_MAX) {
		printf("Failed to spawn a new ant (ants limit reached)\n");
		return -1;
	}

	pthread_mutex_lock(&ants[a_id].mtx);	// prevents the ant from running before initialization

	t_id = start_thread(ant_routine, &ants[a_id], SCHED_FIFO, WCET_ANTS, PRD_ANTS, DL_ANTS, PRIO_ANTS);
	if (t_id < 0) {
		pthread_mutex_unlock(&ants[a_id].mtx);
		deallocate_ant_id(a_id);
		printf("Failed to spawn a new ant (threads limit reached)\n");
		return -2;
	} else {
		init_ant(&ants[a_id], t_id);
		pthread_mutex_unlock(&ants[a_id].mtx);
		printf("Created ant with id %d and tid #%d\n", a_id, t_id);
	}
	pthread_mutex_unlock(&ants[a_id].mtx);
	return 0;
}



/* Kills the ant with the specified id.
*  Returns 0 if success, -1 if it was already dead. */
int kill_ant(unsigned int i) {

	pthread_mutex_lock(&ants[i].mtx);
	if (!ants[i].alive) {
		pthread_mutex_unlock(&ants[i].mtx);
		return -1;
	}
	stop_thread(ants[i].tid);
	pthread_mutex_unlock(&ants[i].mtx);

	deallocate_ant_id(i);

	return 0;
}


/* Spawns the specified number of ants. If there is not enough room for all,
*  create as many as possible.
*  Returns the number of successfully spawned ants.
*/
unsigned int spawn_ants(unsigned int n) {

	for (int i = 0; i < n; ++i) {
		if (spawn_ant() < 0)
			return i;
	}

	return n;
}



void kill_ants(void) {

	printf("Start killing ants \n");

	for (int i = 0; i < POP_SIZE_MAX; ++i) {
		kill_ant(i);
	}

	printf("Finished killing ants \n");
}



int get_ant_id_by_pos(int x, int y) {

	for (int i = 0; i < POP_SIZE_MAX; ++i) {
		if (ants[i].alive && hypot(x - ants[i].pos.x, y - ants[i].pos.y) < 10)
			return i;
	}
	return -1;
}


void init_ants_manager(void) {

	pthread_mutex_lock(&ants_mtx);

	for (int i = 0; i < POP_SIZE_MAX; ++i) {
		pthread_mutex_init(&ants[i].mtx, NULL);
		ants[i].alive = false;
	}

	pthread_mutex_unlock(&ants_mtx);
}