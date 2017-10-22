/*
scheduler.c

*/

/*
scheduler()
{   
    check if things are in the readyQueue

    if there isn't anything and procCount == 0
        halt
    if there isn't anything and procCount == 0 and softBlockedCnt ==0
        panic
    if there isn't anything and procCount > 0 and softBlockedCnt > 0
        enable interrupts
        wait
    
    if there are things in the readyQueue
        currentProc = removeProcQ(&readyQueue)
        record the current time of day
        set currentProc's quantum timer
        LDST(&currentProc->p_s)
}

*/