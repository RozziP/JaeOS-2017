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

HIDDEN void waitBreak(){
    return;
}

HIDDEN void lsB(){
    return;
}

void scheduler()
{   
    //If there was a process running, do something with its time slice
    //we havent decided what time stuff to do yet

    if(emptyProcQ(readyQueue))
    {
        if(procCount == 0)
        {
            HALT();
        }

        if(softBlockCnt == 0)
        {
            PANIC();
        }
        else
        {
            waitBreak();
            setSTATUS(getSTATUS() & INTS_ON | SYSMODE);
            WAIT();
            waitBreak();
        }

    }
    else //there are ready processes
    {
        currentProc = removeProcQ(&readyQueue);
        lsB();
        loadState(&(currentProc->p_s));

        startTimeOfDay = getTODLO();
        endTimeOfDay = 0;
        timeUsed = 0;
        timeLeft=QUANTUM;
    }
    
        
}

