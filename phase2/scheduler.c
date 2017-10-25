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
#include "../e/globals.e"
#include "../h/const.h"
#include "../h/types.h"

scheduler()
{   
    //If there was a process running, do something with its time slice
    //we havent decided what time stuff to do yet

    if(emptyProcQ(readyQueue))
    {
        if(procCount == 0)
        {
            HALT();
        }

        if(procCount > 0 && softBlockCnt == 0)
        {
            PANIC();
        }

        if(procCount > 0 && softBlockCnt > 0)
        {
            currentProc->p_s->cpsr = currentProc->p_s->cpsr | INTS_ON; //Turn interrupts on - this might not work
            WAIT();
        }

    }
    else //there are ready processes
    {
        currentProc = removeProcQ(&readyQueue);
        //get the current time of day and do something with it
        setTimer(QUANTUM);
        LDST(&currentProc->p_s);
    }
    
        
}

