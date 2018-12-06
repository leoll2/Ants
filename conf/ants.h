#ifndef CONF_ANTS_H
#define CONF_ANTS_H

#define	POP_SIZE_MAX 		15		// maximum number of ants
#define STEP_LENGTH			5		// [pixel] length of a single step
#define EXPL_CONE			0.3		// [radians] max deviation between consecutive moves
#define OLFACTION_RADIUS	3		// [cell] olfaction radius
#define VISION_RADIUS		30		// [pixel] vision radius
#define AUDACITY			0.01	// probability of exploration rather than eploitation
#define EXPL_DURATION		5		// [ticks] exploration duration (indifference to pheromones)

#define DEFAULT_POP			8		// initial population by default

#define WCET_ANTS		50
#define PRD_ANTS		100
#define DL_ANTS			80
#define PRIO_ANTS		20


#endif