/*
interrupts.c

*/
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../h/globals.h"
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"

HIDDEN unsigned int getCause();
HIDDEN int getLineNumber();
HIDDEN int getDeviceNumber(int lineNum);
HIDDEN int getDeviceRegister(int lineNum, int DeviceNum);



interruptHandler()
{
    //int cause = (state_t*)INT_OLD->CP15_Cause;
    unsigned int cause = getCause();
    int lineNum;
    int deviceNum;
    unsigned int deviceReg;

    //Determine which line caused the interrupt
    if(cause & BIT3 !=0)
    {
       line = DISK;
    }

    else if(cause & BIT4 !=0)
    {
       line = TAPE;
    }

    else if(cause & BIT5 !=0)
    {
       line = NETWORK;
    }

    else if(cause & BIT6 !=0)
    {
       line = PRINTER;
    }

    else if(cause & BIT7 !=0)
    {
       line = TERMINAL;
    }
    else
    {
        PANIC();
    }

    //TODO find which device on the line caused the interrupt
    deviceNum = getDeviceNumber(lineNum);
    //TODO find the location of that device's registers
    deviceReg=getDeviceRegister(lineNum,deviceNum);

    //copy status register and put in r0???
    //set command field to ACK???
    //THEYRE STRUCTS
    
    //signal the device's semaphore
    int sem = sema4[/*device*/];
    sem++;
    pcb_PTR temp = removeBlocked(sem);
    if(temp == NULL) PANIC();
    insertProcQ(&readyQueue, temp);
    softBlockCnt--;

    LDST(INT_OLD);
}



HIDDEN int getDeviceNumber(int lineNum){
    int cause
    switch(lineNum){
        case DISK:
            cause=DISKMAP;
        break;

        case TAPE:
            cause=TAPEMAP;
        break;

        case NETWORK:
            cause=NETWORKMAP;
        break;

        case PRINTER:
            cause=PRINTMAP;
        break;

        case TERMINAL:
            cause=TERMINALMAP;
        break;

        default: //everything else
            PANIC();
        break;
    }

    if((cause & BIT1) != 0) {
        return 0;
    }

    else if((cause & BIT2) != 0){
        return 1;
    }

    else if((cause & BIT3) != 0){
        return 2;
    }

    else if((cause & BIT4) != 0){
        return 3;
    }

    else if((cause & BIT5) != 0){
        return 4;
    }

    else if((cause & BIT6) != 0){
        return 5;
    }

    else if((cause & BIT7) != 0){
        return 6;
    }

    else if((cause & BIT8) != 0){
        return 7;
    }
    
    PANIC();
}


HIDDEN int getDeviceRegister(int lineNum, int DeviceNum){
    unsigned int registerLocation= DEVICEREGSTART+ (lineNum-NULLLINES) * DEVICEREGSIZE + DEVICEREGSIZE * DeviceNum;
}

