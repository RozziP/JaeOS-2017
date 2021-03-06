/*==========================================================================*\
  scheduler.c
  Contains only the scheduler, a function that facilitates the flow of
  process execution.

  The scheduler ensures that there is always a process that is either running,
  or waiting on I/O by using a round-robin scheduing algorithm.
  
  Authors: Peter Rozzi and Patrick Gemperline
\*==========================================================================*/
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"

#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"


/*
*Takes a new job from the ready queue and runs it.
*if there are no jobs on the readyqueue and we have nothing to run, the CPU halts
*if nothing is blocked by I/O, we panic
*if there are processes blocked by I/O, we wait
*Otherwise, we initialize the new process' time fields and load its state
*/
void scheduler()
{   
    int remainingTime;
    int timeDif;

    pcb_PTR newProc = removeProcQ(&readyQueue);
    
    if(newProc == NULL) //see if processes are ready to run
    {
        currentProc = NULL;
        if(procCount == 0)
        {
            // if no processes left system is done

            HALT();
        }

        else if(softBlockCnt == 0)
        {
            // if processes exist but none are ready and none are softblocked, dead lock has occured

            PANIC();
        }
        else
        {
            // processes exist but all are waiting and 1 or more is waiting for IO

            //Calculate time left before next psudo clock tic
            if (endOfInterval>getTODLO()){
                remainingTime = endOfInterval - getTODLO();
            }
            else{
                remainingTime = 0;
            }
            
            setTIMER(remainingTime);
            
            setSTATUS((getSTATUS() & INTS_ON) | SYSMODE);
    
            WAIT();
        }

    }
    //there are ready processes

    currentProc = newProc;


    //Calculat time left until psudo clock tic
    timeDif = endOfInterval - getTODLO();
    if  (timeDif < 0)
    {
        timeDif = 0;
    }

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

    loadState(&(currentProc->p_s));
        
}

