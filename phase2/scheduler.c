/*==========================================================================*\
  scheduler.c
  Contains only the scheduler, a function that facilitates the flow of
  process execution.

  The scheduler ensures that there is always a process that is either running,
  or waiting on I/O
  
  Authors: Peter Rozzi and Patrick Gemperline
  Date: 10-24-17
\*==========================================================================*/
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"

#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"

HIDDEN void wait(unsigned int x){
    return;
}

HIDDEN void soft(){
    return;
}

HIDDEN void procZero(){
    return;
}
HIDDEN void newProcBreak(){
    return;
}

HIDDEN void currentProcCount(unsigned int x){
    return;
}

void scheduler()
{   
    currentProcCount(procCount);

    if(emptyProcQ(readyQueue))
    {
        currentProc = NULL;

        if(procCount == 0)
        {
            procZero();
            HALT();
        }

        else if(softBlockCnt == 0)
        {
            soft();
            PANIC();
        }

        else
        {
            setTIMER(endOfInterval-getTODLO());
            wait(softBlockCnt);
            wait(procCount);
            
            setSTATUS((getSTATUS() & INTS_ON) | SYSMODE);
            wait(0);
            WAIT();
        }

    }
     //there are ready processes
    
    currentProc = removeProcQ(&readyQueue);
    
    /*If there is less than than one quantum left on the clock...*/
    if((endOfInterval - getTODLO()) < QUANTUM){			
        
        /*Set the new job's timer to be the remaining interval time*/
        setTIMER(endOfInterval-getTODLO());
    }
    else{
        /*Set the new job's timer to be a full quantum*/
        setTIMER(QUANTUM);
    }
    
    newProcBreak();

    startTimeOfDay = getTODLO();

    loadState(&(currentProc -> p_s));

        
    
    
        
}

