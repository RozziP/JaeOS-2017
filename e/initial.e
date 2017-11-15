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

//time things
extern cput_t startTimeOfDay;
extern cput_t endTimeOfDay;
extern cput_t timeUsed;
extern cput_t timeLeft;

extern void main();
extern void copyState(state_t* src, state_t* dest);
extern void loadState(state_t* stateToLoad);