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
#include "../e/initial.e"
#include "../h/const.h"
#include "../h/types.h"

void initArea(state_t* newArea);

main()
{
    initASL();
    initPcb();

    //initialize globals
    procCount    = 0;
    softBlockCnt = 0;
    currentProc  = NULL;
    readyQueue   = mkEmptyProcQueue();

    //Create the new areas
    state_t* newTLB      = TLB_NEW;
    state_t* newPgrmTrap = PRGRM_NEW;
    state_t* newSys      = SYS_NEW;
    state_t* newInt      = INT_NEW;
    //Set their PCs to their respective handlers
    newTLB->pc        = (unsigned int)TLBHandler();
    newPgrmTrap->pc   = (unsigned int)PrgrmTrapHandler();
    newSys->pc        = (unsigned int)SysCallHandler();
    newInt->pc        = (unsigned int)InterruptHandler();
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
    starter->p_s->sp   = (RAMTOP - FRAMESIZE); 
    starter->p_s->pc   = (unsigned int)test();   //start by running test() in p2test.c
    starter->p_s->cpsr = ALLOFF | SYSMODE;       //Interrupts off, kernel mode
    starter->p_s->CP15_Control = ALLOFF;         //VM off
    insertProcQ(&readyQueue, starter);

    //off to see the wizard
    scheduler();
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