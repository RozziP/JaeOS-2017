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

int remainingTime;
/*
*Takes a new job from the ready queue and runs it.
*if there are no jobs on the readyqueue and we have nothing to run, the CPU halts
*if nothing is blocked by I/O, we panic
*if there are processes blocked by I/O, we wait
*Otherwise, we initialize the new process' time fields and load its state
*/
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
            //setTIMER(QUANTUM);

            //remainingTime=endOfInterval-getTODLO();

            //setTIMER(endOfInterval);
            
            setSTATUS((getSTATUS() & INTS_ON) | SYSMODE);
            
            //wait(remainingTime);
            WAIT();
        }

    }
    //there are ready processes
    currentProc = removeProcQ(&readyQueue);

    int timeDif= endOfInterval-getTODLO();
    
    //If there is less than than one quantum left on the clock
    if(timeDif < QUANTUM){			
        
        //Set the new job's timer to be the remaining interval time
        setTIMER(timeDif);
    }
    else
    {
        //Set the new job's timer to be a full quantum
        setTIMER(QUANTUM);
    }
    
    startTimeOfDay = getTODLO();

    loadState(&(currentProc -> p_s));

        
    
    
        
}

