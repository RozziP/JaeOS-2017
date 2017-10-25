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
#include "../e/globals.e"
#include "../h/const.h"
#include "../h/types.h"

void initArea(state_t* newArea);

//Globals
// int procCount;
// int softBlockCnt;
// int sema4[DEVICES];
// pcb_PTR currentProc;
// pcb_PTR readyQueue;


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
    starter->p_s->cpsr = INTS_OFF | SYSMODE;     //Interrupts off, kernel mode
    starter->p_s->CP15_Control = ALLOFF;         //VM off
    insertProcQ(&readyQueue, starter);

    //timer = 5000, or some equivalent
    setTimer(QUANTUM);

    //off to see the wizard
    scheduler();
}

/*
Populate the fields of the given new area
This function just saves eight lines of code in main()
*/
void initArea(state_t* newArea)
{
    newArea->sp   = RAMTOP;
    newArea->cpsr = INTS_OFF | SYSMODE;   //Interrupts off, kernel mode
    newArea->CP15_Control = ALLOFF;     //VM off
}