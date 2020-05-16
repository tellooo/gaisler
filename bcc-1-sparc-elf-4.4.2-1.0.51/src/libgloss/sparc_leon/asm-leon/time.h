#ifndef _ASMSPARC_TIME_H
#define _ASMSPARC_TIME_H

extern volatile struct timespec xtime;
extern unsigned long tick_nsec; /* nsec per tick (resolution) */
extern unsigned long nodotimer;

#endif
