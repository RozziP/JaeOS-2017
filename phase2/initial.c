/*==========================================================================*\
  initial.c
  Contains the initialization function for the OS
  Initializes the ASL and all PCBs, global variables, new handler areas, and
  device semaphores.
  
  Then, creates the initial process, sets the timer, and calls the scheduler.
  
  Authors: Peter Rozzi and Patrick Gemperline
  Date: 10-24-17
\*==========================================================================*/

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/scheduler.e"
#include "../e/exceptions.e"
#include "../e/initial.e"
#include "../e/interrupts.e"
#include "../e/p2test.e"

#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"

HIDDEN void initArea(state_t* newArea);

void initAreasBreak(){
    return;
}

int procCount;
int softBlockCnt;
int sema4[DEVICES];
pcb_PTR currentProc;
pcb_PTR readyQueue;
cput_t startTimeOfDay;
cput_t endTimeOfDay;
cput_t timeUsed;
cput_t timeLeft;

void main()
{
    state_t* newArea;

    initASL();
    initPcbs();

    //initialize globals
    procCount    = 0;
    softBlockCnt = 0;
    currentProc  = NULL;
    readyQueue   = mkEmptyProcQ();

    initAreasBreak();

    /*
    Create the new areas, initialize them with VM off, 
    interrupts enabled, and in system mode
    */
    newArea = (state_t *) TLB_NEW;
    newArea->pc = (unsigned int)tlbHandler;
    newArea->sp   = RAM_TOP;
    newArea->cpsr = ALLOFF | INTS_OFF | SYSMODE;  
    newArea->CP15_Control = ALLOFF;    

    newArea = (state_t *) PRGRM_NEW;
    newArea->pc = (unsigned int)prgrmTrapHandler;
    newArea->sp   = RAM_TOP;
    newArea->cpsr = ALLOFF | INTS_OFF | SYSMODE;   
    newArea->CP15_Control = ALLOFF;     

    newArea = (state_t *) SYS_NEW;
    newArea->pc = (unsigned int)sysCallHandler;
    newArea->sp   = RAM_TOP;
    newArea->cpsr = ALLOFF | INTS_OFF | SYSMODE; 
    newArea->CP15_Control = ALLOFF;     

    newArea = (state_t *) INT_NEW;
    newArea->pc = (unsigned int)interruptHandler;
    newArea->sp   = RAM_TOP;
    newArea->cpsr = ALLOFF | INTS_OFF | SYSMODE;   
    newArea->CP15_Control = ALLOFF;

    //Initialize device semaphores to 0
    int i = 0;
    while(i < DEVICES)
    {
        sema4[i] = 0;
        i++;
    }

    //Initialize the starting process
    pcb_PTR starter = allocPcb();
    starter->p_s.sp   = (RAM_TOP - FRAME_SIZE); //changed from framesize
    starter->p_s.pc   = (unsigned int)test;  //start by running test() in p2test.c
    starter->p_s.cpsr = ALLOFF | SYSMODE;       //Interrupts on, kernel mode
    starter->p_s.CP15_Control = ALLOFF;         //VM off
    insertProcQ(&readyQueue, starter);
    procCount=1;

    //off to see the wizard
    scheduler();

    //screwed if were here
    PANIC();

}

//loads given state
void loadState(state_t* stateToLoad)
{
    LDST(stateToLoad);
}

void copyState(state_t* src, state_t* dest)
{   
    dest->a1 = src->a1;
    dest->a2 = src->a2;
    dest->a3 = src->a3;
    dest->a4 = src->a4;
    dest->v1 = src->v1;
    dest->v2 = src->v2;
    dest->v3 = src->v3;
    dest->v4 = src->v4;
    dest->v5 = src->v5;
    dest->v6 = src->v6;
    dest->sl = src->sl;
    dest->fp = src->fp;
    dest->ip = src->ip;
    dest->sp = src->sp;
    dest->lr = src->lr;
    dest->pc = src->pc;
    dest->cpsr = src->cpsr;
    dest->CP15_Control = src->CP15_Control;
    dest->CP15_EntryHi = src->CP15_EntryHi;
    dest->CP15_Cause = src->CP15_Cause;
    dest->TOD_Hi = src->TOD_Hi;
    dest->TOD_Low = src->TOD_Low;
}
