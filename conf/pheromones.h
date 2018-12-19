#ifndef CONF_PHERO_H
#define CONF_PHERO_H


#define CELL_SIZE   	10  	// size of pheromone cell (must divide field height and width)
#define SMELL_UNIT  	100.0	// pheromone intensity when initially dropped
#define SMELL_THRESH	1.0 	// minimum intensity of a pheromone not approximable to 0
#define DROP_BACKOFF	5   	// backoff time before another pheromone can be dropped in same cell
#define DROP_FACTOR 	0.975 	// regulates pheromone dropped (trail length)
#define EVAPOR_FACTOR	0.98 	// regulates pheromone evaporation (trail duration)

#define WCET_EVAPOR 	50	// Task WCET
#define PRD_EVAPOR  	50	// Task period
#define DL_EVAPOR   	50	// Task deadline
#define PRIO_EVAPOR 	20	// Task priority


#endif
