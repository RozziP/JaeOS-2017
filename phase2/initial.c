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

#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"
#include "../e/p2test.e"

HIDDEN void initArea(state_t* newArea);

HIDDEN void initAreasBreak(){
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
    initASL();
    initPcbs();

    //initialize globals
    procCount    = 0;
    softBlockCnt = 0;
    currentProc  = NULL;
    readyQueue   = mkEmptyProcQ();

    initAreasBreak();

    //Create the new areas
    state_t* newTLB      = (state_t *) TLB_NEW;
    state_t* newPgrmTrap = (state_t *) PRGRM_NEW;
    state_t* newSys      = (state_t *) SYS_NEW;
    state_t* newInt      = (state_t *) INT_NEW;
    //Set their PCs to their respective handlers
    newTLB -> pc        = (unsigned int)tlbHandler;
    newPgrmTrap -> pc   = (unsigned int)prgrmTrapHandler;
    newSys -> pc        = (unsigned int)sysCallHandler;
    newInt -> pc        = (unsigned int)interruptHandler;
    //Set their other fields to defaults
    initArea(newTLB);
    initArea(newPgrmTrap);
    initArea(newSys);
    initArea(newInt);

    //Initialize device semaphores to 0
    for(int i = 0; i < DEVICES; i++)
    {
        sema4[i] = 0;
    }

    //Initialize the starting process
    pcb_PTR starter = allocPcb();
    starter->p_s.sp   = (RAMTOP - FRAMESIZE); 
    starter->p_s.pc   = (unsigned int)test;  //start by running test() in p2test.c
    starter->p_s.cpsr = ALLOFF | SYSMODE;       //Interrupts off, kernel mode
    starter->p_s.CP15_Control = ALLOFF;         //VM off
    insertProcQ(&readyQueue, starter);

    //off to see the wizard
    scheduler();

}

//loads given state
void loadState(state_t* stateToLoad)
{
    LDST(&stateToLoad);
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

/*
Populate the fields of the given new area
This function just saves eight lines of code in main()
*/
HIDDEN void initArea(state_t* newArea)
{
    newArea->sp   = RAMTOP;
    newArea->cpsr = INTS_OFF | SYSMODE;   //Interrupts off, kernel mode
    newArea->CP15_Control = ALLOFF;     //VM off
}
