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

HIDDEN void wait(){
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

void scheduler()
{   
    //If there was a process running, do something with its time slice
    //we havent decided what time stuff to do yet

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
            setTIMER(QUANTUM);
            wait();
            setSTATUS((getSTATUS() & INTS_ON) | SYSMODE);
            wait();
            WAIT();
        }

    }
    else //there are ready processes
    {
        currentProc = removeProcQ(&readyQueue);
        startTimeOfDay = getTODLO();
        endTimeOfDay = 0;
        timeUsed = 0;
        timeLeft=QUANTUM;
        newProcBreak();

        loadState(&(currentProc->p_s));

        
    }
    
        
}

