#ifndef CONF_MULTIMEDIA_H
#define CONF_MULTIMEDIA_H


/* ======================== 
*  ====== DIMENSIONS ======
*  ======================== */
#define TOOLBAR_H 			116     // height of menu area
#define STATS_PANEL_W 		250     // width of status window
#define STATS_ANT_OFF_H		250		// vertical offset of ant details within stats panel
#define ICON_SIZE			40		// size of toolbar icon


/* ======================== 
*  ======== COLORS ========
*  ======================== */
#define COLOR_RED       makecol(0xFF,0x00,0x00)
#define COLOR_GREEN     makecol(0x00,0xFF,0x00)
#define COLOR_BLUE      makecol(0x00,0x00,0xFF)
#define COLOR_BLACK		makecol(0x00,0x00,0x00)
#define COLOR_WHITE		makecol(0xFF,0xFF,0xFF)
#define COLOR_BROWN		makecol(0x68,0x47,0x14)
#define COLOR_SAND		makecol(0xFF,0xF3,0xC3)

#define COLOR_TEXT				COLOR_BROWN

#define COLOR_TOOLBAR			COLOR_SAND
#define COLOR_TOOLBAR_BORDER	COLOR_BROWN
#define COLOR_ICON_BORDER		COLOR_BROWN

#define COLOR_STATS_PANEL		COLOR_SAND
#define COLOR_STATS_BORDER		COLOR_BROWN


/* ======================== 
*  ======== TASKS =========
*  ======================== */
#define WCET_GRAPHICS		50
#define PRD_GRAPHICS		50
#define DL_GRAPHICS			50
#define PRIO_GRAPHICS		30

#define WCET_KEYBOARD		10
#define PRD_KEYBOARD		200
#define DL_KEYBOARD			200
#define PRIO_KEYBOARD		10

#define WCET_MOUSE			10
#define PRD_MOUSE			50
#define DL_MOUSE			50
#define PRIO_MOUSE			20

#endif
