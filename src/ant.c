#include <math.h>
#include <stdio.h>

#include "ant.h"

const float TWO_PI = 2 * M_PI;

ant ants[POP_SIZE_MAX];
uint8_t n_ants = 0;
pthread_mutex_t ants_mtx = PTHREAD_MUTEX_INITIALIZER;


void init_ant(ant *const a, int id) {

	a->alive = true;
	a->id = (unsigned int)id;
	a->pos.x = a->pos.y = 0;
	a->pos.angle = rand() * TWO_PI;
	a->interest = FOOD;
	a->behaviour = RESTING;
	a->excitement = 1.0;
}


void move_ant_random(ant *const a) {	// DEBUG PURPOSE
	
	a->pos.x = rand() % FIELD_WIDTH;
	a->pos.y = rand() % FIELD_HEIGHT;
}


void reverse_direction(ant *const a) {

	a->pos.angle = fmod((a->pos.angle + M_PI), TWO_PI);
}


void exploration_step(ant *const a) {

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
	//printf("ant: %d  x:%d  y: %d  angle: %f\n", a->id, a->pos.x, a->pos.y, a->pos.angle);
}


/* Return true if the target has been reached, false if yet to be reached. */
bool tracking_step(ant *const a, int target_x, int target_y) {

	// Return if ant is already in the target position
	if ((a->pos.x == target_x) && (a->pos.y = target_y))
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
			// reverse_direction(a);	// TO BE FIXED (ants are initialized resting)
			a->interest = FOOD;
			a->behaviour = EXPLORING;
			a->excitement = 1.0;
			break;
		case EATING:
			reverse_direction(a);
			a->interest = HOME;
			a->behaviour = EXPLORING;
			a->excitement = 1.0;
			break;
		case EXPLORING:
			v_scan = find_target_visually(a->pos.x, a->pos.y, VISION_RADIUS, a->interest);
			if (v_scan.success) {
				if (tracking_step(a, v_scan.target_x, v_scan.target_y))
					a->behaviour = (a->interest == FOOD) ? EATING : RESTING;
				else
					a->behaviour = TRACKING;
				break;
			}
			s_scan = find_smell_direction(a->pos.x, a->pos.y, a->pos.angle, 
					OLFACTION_RADIUS, FULL, a->interest
			);
			if (s_scan.success) {
				tracking_step(a, s_scan.opt_x, s_scan.opt_y);
				a->behaviour = TRACKING;
			} else {
				exploration_step(a);
			}
			break;
		case TRACKING:
			v_scan = find_target_visually(a->pos.x, a->pos.y, VISION_RADIUS, a->interest);
			if (v_scan.success) {
				if (tracking_step(a, v_scan.target_x, v_scan.target_y))
					a->behaviour = (a->interest == FOOD) ? EATING : RESTING;
				break;
			}
			s_scan = find_smell_direction(a->pos.x, a->pos.y, a->pos.angle, 
					OLFACTION_RADIUS, FULL, a->interest		// CHANGED DBG: should be FORWARD
			);
			if (s_scan.success && !s_scan.local_optimum) {
				tracking_step(a, s_scan.opt_x, s_scan.opt_y);
			} else if (s_scan.success && s_scan.local_optimum) {
				// TODO: esci dal minimo locale
				printf("ant: %d interest: %s minimo locale!\n", a->id, 
								(a->interest == FOOD) ? "food" : "home");
				a->behaviour = EXPLORING;
			} else {
				exploration_step(a);
				a->behaviour = EXPLORING;
			}
			break;
		default:
			printf("This should not happen! (unrecognized ant behaviour)\n");
	}

	deploy_pheromone(a->id, a->pos.x, a->pos.y, 
					 a->interest == FOOD ? HOME : FOOD,
					 a->excitement * SMELL_UNIT
	);
	a->excitement *= DEPLOY_FACTOR;
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
		id = start_thread(ant_routine, &ants[i], SCHED_FIFO, WCET_ANTS, PRD_ANTS, DL_ANTS, PRIO_ANTS);
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