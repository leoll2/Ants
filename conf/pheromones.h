#ifndef CONF_PHERO_H
#define CONF_PHERO_H


#define CELL_SIZE			10		// size of pheromone cell (must divide field height and width)
#define SMELL_UNIT			100.0	// pheromone intensity when initially deployed
#define SMELL_THRESH		1.0		// minimum intensity of a pheromone not approximable to 0
#define DROP_BACKOFF		5		// backoff time before another pheromone can be deployed in same cell
#define DROP_FACTOR			0.975	// regulates pheromone deployment (trail length)
#define DECAY_FACTOR		0.98	// regulates pheromone evaporation (trail duration)

#define WCET_DECAY			50
#define PRD_DECAY			50
#define DL_DECAY 			50
#define PRIO_DECAY			20


#endif
