/*
interrupts.c

*/
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
HIDDEN unsigned int getDeviceRegister(int lineNum, int DeviceNum);



void interruptHandler()
{
    unsigned int cause = ((state_t*)INT_OLD) -> CP15_Cause >> 24; //shift right by 24 bits for comparison
    int lineNum;
    int status;
    int deviceNum; 
    int semIndex;
    devreg_t* deviceReg;
    ((state_t*)INT_OLD) -> pc = ((state_t*)INT_OLD) -> pc - 4; //Go back to the executing instruction after interrupt

    //if there was a process running, we have to manage its timer
    if(currentProc != NULL)
    {
		endTimeOfDay=getTODLO();
		timeUsed = endTimeOfDay - startTimeOfDay;
		currentProc->p_time = currentProc->p_time + timeUsed;
		timeLeft = timeLeft - timeUsed;
		
		//copy the old interrupt area into the current state
		copyState((state_t *) INT_OLD, &(currentProc->p_s));
	}

    //Determine which line caused the interrupt
    if((cause & LINE2) != 0) //timer ran out
    {
        //stop the current process and put it back on the readyqueue
        insertProcQ(&(readyQueue), currentProc);
        currentProc = NULL;
        setTIMER(QUANTUM);
        scheduler();
    }
    else if(cause & LINE3 !=0)
    {
       lineNum = DISK;
    }
    else if(cause & LINE4 !=0)
    {
       lineNum = TAPE;
    }

    else if(cause & LINE5 !=0)
    {
       lineNum = NETWORK;
    }

    else if(cause & LINE6 !=0)
    {
       lineNum = PRINTER;
    }
    
    else if(cause & LINE7 !=0)
    {
       lineNum = TERMINAL;
    }
    else
    {
        PANIC();
    }

    
    //Calculate the device number, the device's semaphore index, and the device's register location
    deviceNum = getDeviceNumber(lineNum);
    lineNum = lineNum - NULLLINES; //idk why we do this but the book says so
    semIndex = lineNum * DEVICEPERLINE + deviceNum;
    deviceReg = (devreg_t*)getDeviceRegister(lineNum, semIndex);

    //if it's a terminal, we need to do special things
    if(lineNum = TERMINAL)
    {
        bool isRead = TRUE;
        //the terminal was writing
        if(/*!read*/)
        {
            semIndex += DEVICEPERLINE;
            isRead = FALSE;
            deviceReg->term.transm_command = ACK;
            status = deviceReg->term.transm_status;
        }
        else //the terminal is reading
        {
            deviceReg->term.recv_command = ACK;
            status = deviceReg->recv_status;
        }
    }
    else //it's not a terminal
    {
        deviceReg->dtp.command = ACK;
        status = deviceReg->dtp.status;

        //signal the device's semaphore
        int* sem = &(sema4[semIndex]);
        *sem++
        if(sem <= 0)
        {
            pcb_PTR temp = removeBlocked(&sem);
            if(temp != NULL)
            {
                temp->p_semAdd = NULL;
                (temp->p_s).a1 = status;
                softBlockCnt--;

                insertProcQ(&(readyQueue), temp);
            }
        }
        
    }
    startTimeOfDay = getTODLO();
    loadState((state_t *)INT_OLD);

}



HIDDEN int getDeviceNumber(int lineNum)
{
    int deviceNum = 0;
    unsigned int tempDevice = DEVICEFRONT;
    bool found = FALSE;

    //set our bitmap to the proper line
    unsigned int* bitMap = (unsigned int*)(INTMAP + (lineNum * DEVICEREGSIZE));

    while(!found)
    {
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

//devAddrBase = 0x0000.0040 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10)
HIDDEN unsigned int getDeviceRegister(int lineNum, int semIndex){
    unsigned int registerLocation = DEVICEREGSTART + (lineNum * LINEOFFSET) + (semIndex * DEVICEOFFSET);
    return registerLocation;
}

