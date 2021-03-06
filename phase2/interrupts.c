/*==========================================================================*\
  interrupts.c

  Contains functions to handle device and clock interrupts in the OS.

  Interval timer interrupts occur when a process' timer runs out, they are
  handled by reseting the timer and putting the process back on the ready queue.

  Device interrupts are handled by signalling the device's semaphore and
  unblocking a process from it.

  Authors: Peter Rozzi and Patrick Gemperline
\*==========================================================================*/
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/scheduler.e"

#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"

HIDDEN unsigned int getCause();
HIDDEN int getLineNumber();
HIDDEN int getDeviceNumber(int lineNum);
HIDDEN unsigned int getDeviceRegister(int lineNum, int semIndex);
HIDDEN void finish();


void interruptHandler()
{
    unsigned int cause;
    int lineNum;
    int status;
    int deviceNum; 
    int semIndex;

    cput_t endTimeOfDay;
    cput_t timeUsed;
    devreg_t* deviceReg;
    ((state_t*)INT_OLD)->pc=((state_t*)INT_OLD)->pc-4;//Go back to the executing instruction after interrupt
    cause = ((state_t*)INT_OLD)->CP15_Cause>>24; //shift right by 24 bits for comparison


   
    //if there was a process running, we have to manage its timer
    if(currentProc != NULL)
    {
        
		endTimeOfDay=getTODLO();
		timeUsed = endTimeOfDay - startTimeOfDay;
		currentProc->p_time = currentProc->p_time + timeUsed;
		
		
		//copy the old interrupt area into the current state
		copyState((state_t *) INT_OLD, &(currentProc->p_s));
	}
    
    //Determine which line caused the interrupt
    if((cause & LINE2) == LINE2) //timer ran out
    {
            if(endOfInterval <= getTODLO())
            {
                pcb_PTR temp = removeBlocked(&(deviceSema[DEVICES-1]));

                //While there are processes waiting on the clock
                while(temp != NULL)
                {
                    temp->p_semAdd = NULL;
                    softBlockCnt--;
                    
                    //Add it to the ready queue
                    insertProcQ(&(readyQueue), temp);
                    
                    //Remove the next process
                    temp = removeBlocked(&(deviceSema[DEVICES-1]));
                }
                //Set the seamphore to zero bceuse we unblocked all processes
                deviceSema[DEVICES-1] = 0;

                //reset the interval timer
                setTIMER(QUANTUM);
                endOfInterval = getTODLO() + INT_TIME;
                
                finish();
            }
            else //a process' quantum ended
            {
                //stop it from running and put it back on the ready queue
                if (currentProc != NULL)
                {
                    insertProcQ(&(readyQueue),currentProc);
                }

                //reset the quantum timer and get a new job
                setTIMER(QUANTUM);
                scheduler();
        }
    }
    else if((cause & LINE3) !=0)
    {
       lineNum = DISK;
    }
    else if((cause & LINE4) !=0)
    {
       lineNum = TAPE;
    }

    else if((cause & LINE5) !=0)
    {
       lineNum = NETWORK;
    }

    else if((cause & LINE6) !=0)
    {
       lineNum = PRINTER;
    }

    else if((cause & LINE7) !=0)
    {
       lineNum = TERMINAL;
    }
    else
    {
        PANIC();
    }

    
    //Calculate the device number, the device's semaphore index, and the device's register location
    deviceNum = getDeviceNumber(lineNum);
    if(deviceNum == -1) PANIC();
    
    lineNum = lineNum - NULLLINES; 
    semIndex = lineNum * DEVICEPERLINE + deviceNum;
    deviceReg = (devreg_t*)getDeviceRegister(lineNum, semIndex);

    //if it's a terminal, we need to do special things
    if(lineNum = TERMINAL)
    {
        
        //the terminal was writing
        if(((deviceReg->term.transm_status) & 0xF) != READY)
        {
            
            semIndex = semIndex + DEVICEPERLINE;
            status = deviceReg->term.transm_status;
            deviceReg->term.transm_command = ACK;
            
        }
        else //the terminal is reading
        {
            status = deviceReg->term.recv_status;
            deviceReg->term.recv_command = ACK;
        }
    }
    else
    {
        status = deviceReg->dtp.status;
        deviceReg->dtp.command = ACK;
    }

    //signal the device's semaphore
    deviceSema[semIndex] = deviceSema[semIndex] + 1;
    if(deviceSema[semIndex] <= 0)
    {
        pcb_PTR temp = removeBlocked(&(deviceSema[semIndex]));
        if(temp != NULL)
        {
            temp->p_semAdd = NULL;
            (temp->p_s).a1 = status;
            softBlockCnt--;
            insertProcQ(&(readyQueue), temp);
        }
        else
        {
            semaStat[semIndex] = status;
        }
    }
    finish();
}


/*
 * Looks through each bit of the given device line
 * to determine which device caused the interrupt.
 * Returns -1 if the device is not found.
 */
HIDDEN int getDeviceNumber(int lineNum)
{
    unsigned int deviceNum = 0;
    unsigned int tempDevice = DEVICEFRONT;
    bool found = FALSE;

    //set our bitmap to the proper line
    unsigned int* bitMap = (unsigned int*)(INTMAP + ((lineNum-3) * DEVICEREGLENG));
    

    while(!found)
    {
        if(deviceNum > 7)
        {
            deviceNum = -1;
            break;
        }
        if((tempDevice & *bitMap) != 0)
        {
            found = TRUE;
        }
        else
        {
            tempDevice = tempDevice << 1; //shift to the next device
            deviceNum++;
        }
    }
    return deviceNum;
}

/*
 * Uses a magic formula to find the location of the given device's registers
 */
HIDDEN unsigned int getDeviceRegister(int lineNum, int semIndex)
{
    unsigned int registerLocation;
    registerLocation = DEVICEREGSTART + (semIndex * DEVICEREGSIZE);
    return registerLocation;
}

HIDDEN void finish()
{
	if(currentProc != NULL)
    {
		startTimeOfDay = getTODLO();
		loadState(&(currentProc->p_s));
	}
    else
    {
        scheduler();
    }
}