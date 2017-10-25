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
#include "../h/const.h"
#include "../h/types.h"

void initArea(state_t* newArea);

//Globals
int procCount;
int softBlockCnt;
int sema4[DEVICES];
pcb_PTR currentProc;
pcb_PTR readyQueue;


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
    state_t* newTLB      = TLBNEW;
    state_t* newPgrmTrap = PRGRMNEW;
    state_t* newSys      = SYSNEW;
    state_t* newInt      = INTNEW;
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
    pcb_PTR p = allocPcb();
    p->p_s->sp   = (RAMTOP - FRAMESIZE) 
    p->p_s->pc   = (unsigned int)test(); //start by running test() in p2test.c
    p->p_s->cpsr = INTSOFF | SYSMODE     //Interrupts off, kernel mode
    p->p_s->CP15_Control = ALLOFF;       //VM off
    insertProcQ(&readyQueue, p);

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
    newArea->cpsr = INTSOFF | SYSMODE   //Interrupts off, kernel mode
    newArea->CP15_Control = ALLOFF;     //VM off
}