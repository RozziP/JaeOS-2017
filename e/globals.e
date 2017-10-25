#include "asl.e"
#include "pcb.e"
#include "../h/const.h"
#include "../h/types.h"

int procCount;
int softBlockCnt;
int sema4[DEVICES];
pcb_PTR currentProc;
pcb_PTR readyQueue;