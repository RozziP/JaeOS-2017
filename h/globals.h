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
extern cput_t startTimeOfDay;
extern cput_t endTimeOfDay;
extern cput_t timeUsed;
extern cput_t timeLeft;

#endif
