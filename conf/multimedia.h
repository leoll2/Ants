#ifndef CONF_MULTIMEDIA_H
#define CONF_MULTIMEDIA_H


/* ======================== 
*  ======== COLORS ========
*  ======================== */
#define TOOLBAR_H 		100     // height of menu area
//#define TOOLBAR_W 		500     // width of menu area
//#define STATS_PANEL_H	540     // height of status window
#define STATS_PANEL_W 	250     // width of status window
#define ICON_SIZE		40		// size of toolbar icon


/* ======================== 
*  ======== COLORS ========
*  ======================== */
#define COLOR_RED       makecol(0xFF,0x00,0x00)
#define COLOR_GREEN     makecol(0x00,0xFF,0x00)
#define COLOR_BLUE      makecol(0x00,0x00,0xFF)
#define COLOR_BLACK		makecol(0x00,0x00,0x00)
#define COLOR_WHITE		makecol(0xFF,0xFF,0xFF)

#define COLOR_TOOLBAR		COLOR_WHITE
#define COLOR_ICON_BORDER	COLOR_BLUE
#define COLOR_TEXT			COLOR_BLUE


/* ======================== 
*  ======== TASKS =========
*  ======================== */
#define WCET_GRAPHICS		60
#define PRD_GRAPHICS		100
#define DL_GRAPHICS			80
#define PRIO_GRAPHICS		30

#define WCET_KEYBOARD		10
#define PRD_KEYBOARD		200
#define DL_KEYBOARD			200
#define PRIO_KEYBOARD		20

#define WCET_MOUSE			10
#define PRD_MOUSE			100
#define DL_MOUSE			100
#define PRIO_MOUSE			20

#endif
