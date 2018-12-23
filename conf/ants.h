#ifndef CONF_ANTS_H
#define CONF_ANTS_H

/* ======================== 
*  ========= ANT ==========
*  ======================== */

#define	POP_SIZE_MAX		50	// maximum number of ants
#define DEFAULT_POP  		10	// initial population by default
#define STEP_LENGTH  		5	// [pixel] length of a single step
#define EXPL_CONE   		0.3	// [radians] max deviation between consecutive moves
#define OLFACTION_RADIUS 	3	// [cell] olfaction radius
#define VISION_RADIUS 		30	// [pixel] vision radius
#define MAX_AUDACITY		0.005	// max probability of exploration rather than eploitation
#define EXPL_DURATION		8	// [ticks] exploration duration (indifference to pheromones)

/* ======================== 
*  ===== STATE UPDATE =====
*  ======================== */

#define PRD_ANTS		50	// Task period
#define DL_ANTS			50	// Task deadline
#define PRIO_ANTS		20	// Task priority


#endif
