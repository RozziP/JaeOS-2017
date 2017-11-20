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
HIDDEN unsigned int getDeviceRegister(int lineNum, int semIndex);

HIDDEN void intDebug(unsigned int x){
    return;
}

unsigned int cause;

void interruptHandler()
{
    cause = ((state_t*)INT_OLD) -> CP15_Cause >> 24; //shift right by 24 bits for comparison
    int lineNum;
    int status;
    int deviceNum; 
    int semIndex;
    devreg_t* deviceReg;
    ((state_t*)INT_OLD) -> pc = ((state_t*)INT_OLD) -> pc -4; //Go back to the executing instruction after interrupt

   
    //if there was a process running, we have to manage its timer
    if(currentProc != NULL)
    {
        intDebug(0x2);
		endTimeOfDay=getTODLO();
		timeUsed = endTimeOfDay - startTimeOfDay;
		currentProc->p_time = currentProc->p_time + timeUsed;
		
		
		//copy the old interrupt area into the current state
		copyState((state_t *) INT_OLD, &(currentProc->p_s));
	}
    
    //Determine which line caused the interrupt
    if((cause & LINE2) != 0) //timer ran out
    {
        intDebug(0x4);
        //stop the current process and put it back on the readyqueue
        insertProcQ(&(readyQueue), currentProc);
        currentProc = NULL;
        setTIMER(QUANTUM);
        scheduler();
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
        intDebug(0x5);
        PANIC();
    }

    
    //Calculate the device number, the device's semaphore index, and the device's register location
    deviceNum = getDeviceNumber(lineNum);
    intDebug(0x1111);
    intDebug(deviceNum);
    if(deviceNum == -1) PANIC();
    lineNum = lineNum - NULLLINES; 
    semIndex = lineNum * DEVICEPERLINE + deviceNum;
    deviceReg = (devreg_t*)getDeviceRegister(lineNum, semIndex);
    intDebug((unsigned int) deviceReg);

    //if it's a terminal, we need to do special things
    if(lineNum = TERMINAL)
    {
        intDebug(0x6);
        intDebug((deviceReg -> term.transm_status) & 0x0F);
        //the terminal was writing
        if(((deviceReg -> term.transm_status) & 0x0F) != READY)
        {
            intDebug(0x7);
            intDebug(sema4[semIndex]);
            semIndex = semIndex + DEVICEPERLINE;
            intDebug(sema4[semIndex]);
            deviceReg->term.transm_command = ACK;
            status = deviceReg->term.transm_status;
        }
        else //the terminal is reading
        {
            deviceReg->term.recv_command = ACK;
            status = deviceReg->term.recv_status;
        }
    }
    else{
        status = deviceReg -> dtp.status;
        deviceReg->dtp.command = ACK;
    }
    /*
    intDebug(0x333);
    devreg_t* tempDevReg;
    for (int line =0; line<8; line= line +1){
        for (int device = 0; device <8; device=device+1){
            tempDevReg = (devreg_t*) getDeviceRegister(line,device);
            tempDevReg -> dtp .command = 1;
            tempDevReg -> term.recv_command =1;
        }
    }
    intDebug(0x666);
    */

    //signal the device's semaphore
    sema4[semIndex] = sema4[semIndex] + 1;
    intDebug(sema4[semIndex]);
    if(sema4[semIndex] <= 0)
    {
        pcb_PTR temp = removeBlocked(&(sema4[semIndex]));
        if(temp != NULL)
        {
            temp->p_semAdd = NULL;
            (temp->p_s).a1 = status;
            softBlockCnt--;
            intDebug(0x1234);
            insertProcQ(&(readyQueue), temp);
            intDebug(0x5678);
            startTimeOfDay=getTODLO();
            scheduler();
        }
    }
    else{
        //save dev stat;
    }
        
    
    intDebug(0x9);
    startTimeOfDay = getTODLO();
    loadState(&(currentProc->p_s));

}



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

//devAddrBase = 0x0000.0040 + ((IntlineNo - 3) * 0x80) + (DevNo * 0x10)
HIDDEN unsigned int getDeviceRegister(int lineNum, int semIndex)
{
    unsigned int registerLocation;
    registerLocation = DEVICEREGSTART + (semIndex * DEVICEREGSIZE);
    //registerLocation = DEVICEREGSTART + (lineNum * LINEOFFSET) + (semIndex * DEVICEOFFSET);
    return registerLocation;
}