/*
interrupts.c

*/
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../h/globals.h"
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"


interruptHandler()
{
    //int cause = (state_t*)INT_OLD->CP15_Cause;
    unsigned int cause = getCause();
    int lineNum;
    int deviceNum;

    //Determine which line caused the interrupt
    if(cause & LINE3 !=0)
    {
       line = DISK;
    }

    else if(cause & LINE4 !=0)
    {
       line = TAPE;
    }

    else if(cause & LINE5 !=0)
    {
       line = NETWORK;
    }

    else if(cause & LINE6 !=0)
    {
       line = PRINTER;
    }

    else if(cause & LINE7 !=0)
    {
       line = TERMINAL;
    }
    else
    {
        PANIC();
    }

    //TODO find which device on the line caused the interrupt
    deviceNum = //?
    //TODO find the location of that device's registers

    //copy status register and put in r0???
    //set command field to ACK???

    //signal the device's semaphore
    int sem = sema4[/*device*/];
    sem++;
    pcb_PTR temp = removeBlocked(sem);
    insertProcQ(&readyQueue, temp);
    softBlockCnt--;

    LDST(INT_OLD);
}

/*
interruptHandler()
{
    figure out which device caused the interrupt
    calculate the address of the device's registers
    copy the status field
    set command field to ACK
    put the copied status in r0
    signal the sem of the device
    LDST(oldInt)
}
*/