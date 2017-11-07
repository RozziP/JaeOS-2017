#ifndef GLOBALS
#define GLOBALS

#include "../e/asl.e"
#include "../e/pcb.e"
#include "const.h"
#include "types.h"

extern int procCount;
extern int softBlockCnt;
extern int sema4[DEVICES];
extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;

#endif
