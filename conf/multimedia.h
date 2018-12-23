#ifndef CONF_MULTIMEDIA_H
#define CONF_MULTIMEDIA_H


/* ======================== 
*  ====== DIMENSIONS ======
*  ======================== */
#define TOOLBAR_H 		116	// [pixel] height of menu area
#define STATS_PANEL_W 		250	// [pixel] width of status window
#define STATS_ANT_OFF_H		250	// [pixel] vertical offset of ant details within stats panel
#define ICON_SIZE		40	// [pixel] size of a toolbar icon


/* ======================== 
*  ======== PALETTE =======
*  ======================== */
#define COLOR_RED 		makecol(0xFF,0x00,0x00)
#define COLOR_GREEN		makecol(0x00,0xFF,0x00)
#define COLOR_BLUE	 	makecol(0x00,0x00,0xFF)
#define COLOR_BLACK		makecol(0x00,0x00,0x00)
#define COLOR_WHITE		makecol(0xFF,0xFF,0xFF)
#define COLOR_BROWN		makecol(0x68,0x47,0x14)
#define COLOR_SAND 		makecol(0xFF,0xF3,0xC3)

#define COLOR_TEXT  		COLOR_BROWN

#define COLOR_TOOLBAR 		COLOR_SAND
#define COLOR_TOOLBAR_BORDER 	COLOR_BROWN
#define COLOR_ICON_BORDER    	COLOR_BROWN

#define COLOR_STATS_PANEL  	COLOR_SAND
#define COLOR_STATS_BORDER 	COLOR_BROWN


/* ======================== 
*  ====== GUI TASKS =======
*  ======================== */

#define PRD_GRAPHICS		50	// Task period
#define DL_GRAPHICS 		50	// Task deadline
#define PRIO_GRAPHICS 		30	// Task priority

#define PRD_KEYBOARD 		200	// Task period
#define DL_KEYBOARD   		200	// Task deadline
#define PRIO_KEYBOARD 		10	// Task priority

#define PRD_MOUSE		50	// Task period
#define DL_MOUSE		50	// Task deadline
#define PRIO_MOUSE		20	// Task priority

#endif
