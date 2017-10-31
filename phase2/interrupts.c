/*
interrupts.c

*/
#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/globals.h"
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"


interruptHandler()
{
    //int cause = (state_t*)INT_OLD->CP15_Cause;
    unsigned int cause = getCause();
    
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