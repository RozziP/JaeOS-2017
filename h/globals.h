#ifndef GLOBALS
#define GLOBALS

#include "../e/pcb.e"
#include "const.h"
#include "types.h"

extern int procCount;
extern int softBlockCnt;
extern int sema4[DEVICES];
extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;

//time things
cput_t startTimeOfDay;
cput_t endTimeOfDay;
cput_t timeUsed;
cput_t timeLeft;

#endif
