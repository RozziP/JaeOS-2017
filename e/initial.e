/*
initial.e
*/

#include "../h/const.h"
#include "../h/types.h"

extern int procCount;
extern int softBlockCnt;
extern int sema4[DEVICES];
extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;

extern void main();
extern void copyState(state_t* src, state_t* dest);