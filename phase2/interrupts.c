/*
interrupts.c

*/
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../h/globals.h"
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"

HIDDEN unsigned int getCause();
HIDDEN int getLineNumber();
HIDDEN int getDeviceNumber(int lineNum);
HIDDEN int getDeviceRegister(int lineNum, int DeviceNum);



void interruptHandler()
{
    unsigned int cause = ((state_t*)INT_OLD) -> CP15_Cause >> 24; //shift right by 24 bits for comparison
    int lineNum;
    int deviceNum; 
    int semIndex;
    dtpreg_t* deviceReg;
    ((state_t*)INT_OLD) -> pc = ((state_t*)INT_OLD) -> pc - 4; //Go back to the executing instruction after interrupt

    //if there was a process running, we have to manage its timer
    if(currentProc != NULL){
		cput_t endTimeOfDay;
		
		endTimeOfDay=getTODLO();
		
		timeUsed = endTimeOfDay - startTimeOfDay;
		currentProc->p_time = currentProc->p_time + timeUsed;
		timeLeft = timeLeft - timeUsed;
		
		//copy the old interrupt area into the current state
		copyState(INT_OLD, &(currentProc->p_s));
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

    deviceNum = getDeviceNumber(lineNum);
    semIndex = (lineNum * DEVICEPERLINE) + deviceNum;
    deviceReg = getDeviceRegister(lineNum, deviceNum);

    //copy status register and put in r0???
    //set command field to ACK???
    //THEYRE STRUCTS
    
    //signal the device's semaphore
    int sem = sema4[semIndex]++;
    pcb_PTR temp = removeBlocked(&sem);
    if(temp != NULL)
    {
        temp->p_semAdd = NULL;

        (temp -> p_s).a1 = deviceReg -> status;
        softBlockCnt--;

        insertProcQ(&(readyQueue), temp);
    }

    deviceReg -> command = ACK;
    startTimeOfDay=getTODLO();
    loadState((state_t *) INT_OLD);
}



HIDDEN int getDeviceNumber(int lineNum)
{
    int deviceNum = 0;
    unsigned int tempDevice = DEVICEFRONT;
    BOOL found = FALSE;

    //set our bitmap to the proper line
    unsigned int * bitMap = (unsigned int*)(INTMAP + (lineNum * DEVICEREGSIZE));

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
}
   


HIDDEN int getDeviceRegister(int lineNum, int DeviceNum){
    unsigned int registerLocation = DEVICEREGSTART+(lineNum-NULLLINES)*DEVICEREGSIZE+DEVICEREGSIZE*DeviceNum;
}

