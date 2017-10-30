#include "../e/asl.e"
#include "../e/pcb.e"
#include "const.h"
#include "types.h"

int procCount;
int softBlockCnt;
int sema4[DEVICES];
pcb_PTR currentProc;
pcb_PTR readyQueue;