/*
interrupts.c

*/
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