#ifndef FIELD_H
#define FIELD_H

#include "conf/field.h"
#include "conf/pheromones.h"
#include "rt_thread.h"

typedef enum phero_type {HOME, FOOD} 	phero_type;
typedef enum scan_mode  {FULL, FORWARD} scan_mode;

#define PH_SIZE_H 		(FIELD_WIDTH  / CELL_SIZE)
#define PH_SIZE_V		(FIELD_HEIGHT / CELL_SIZE)



typedef struct food {
	int x;
	int y;
	unsigned int units;
	pthread_mutex_t mtx;
} food;

typedef struct cell {
	float 			food;			// food scent intensity
	float 			home;			// anthill scent intensity
	unsigned int 	backoff_food;	// cooldown before food pheromone can be deployed
	unsigned int 	backoff_home;	// cooldown before food pheromone can be deployed 
	pthread_mutex_t mtx;
} cell;

typedef struct visual_scan {
	bool 			success;		// desired object detected
	int 			target_x;		// [pixel] target x
	int 			target_y;		// [pixel] target y
	bool			oth_obj_found; 	// any other object detected
} visual_scan;

typedef struct smell_scan {
	bool 			success;		// any pheromone detected nearby?
	bool 			local_optimum;	// is the ant already in the cell with the strongest smell?
	int 			opt_x;			// [pixel] optimal x
	int 			opt_y;			// [pixel] optimal y
	float 			opt_val;		// pheromone value in optimal (x,y)
} smell_scan;

extern cell ph[PH_SIZE_H][PH_SIZE_V];
extern food foods[MAX_FOOD_SRC];
extern unsigned int n_food_src;

visual_scan find_target_visually(int x, int y, int radius, phero_type desired_type);

smell_scan find_smell_direction(int x, int y, int orientation, int radius, scan_mode mode, phero_type type);

void deploy_pheromone(unsigned int id, int x, int y, phero_type type, float value);

unsigned int start_pheromones(void);

void stop_pheromones(void);

void init_foods(void);

int consume_food(int x, int y);

int deploy_food(int x, int y);

#endif