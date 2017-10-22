/*
initial.c

*/

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

    procCount    = 0;
    softBlockCnt = 0;
    currentProc  = NULL;
    readyQueue   = mkEmptyProcQueue();

    //I don't know the locations of the new area states
    //Create the new areas
    state_t* newTLB      = ;
    state_t* newPgrmTrap = ;
    state_t* newSys      = ;
    state_t* newInt      = ;
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
    p->p_s->sp   = (RAMTOP - FRAMESIZE)  //dont know RAMTOP yet
    p->p_s->pc   = (unsigned int)test(); //start by running test() in p2test.c
    p->p_s->cpsr = //no interrupts, kernel mode
    p->p_s->CP15_Control = //VM off
    insertProcQ(&readyQueue, p);

    //timer = 5000, or some equivalent

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
    newArea->cpsr = //no interrupts, kernel mode
    newArea->CP15_Control = //VM off
}