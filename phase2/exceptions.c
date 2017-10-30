/*
exceptions.c

*/

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/globals.h"
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/uARMtypes.h"

HIDDEN void sys1(state_PTR callingProc);
HIDDEN void sys2();
HIDDEN void sys3(state_PTR callingProc);
HIDDEN void sys4(state_PTR callingProc);
HIDDEN void sys5(state_PTR callingProc);
HIDDEN void sys6(state_PTR callingProc);
HIDDEN void sys7(state_PTR callingProc);
HIDDEN void sys8(state_PTR callingProc);
HIDDEN void killAllChildren(pcb_PTR top);
HIDDEN void passUpOrDie(state_PTR callingProc,int cause);

void tlbHandler()
{
    state_PTR caller = TLB_OLD;
    passUpOrDie(caller, TLB);
}

void prgrmTrapHandler()
{
    state_PTR caller = PRGRM_OLD;
    passUpOrDie(caller, PRGRMTRAP);
}

void sysHandler()
{
    state_PTR callingProc;
    int requestedSysCall;
    
    callingProc = (state_PTR) SYS_OLD;
    requestedSysCall = callingProc-> //where to find this info?

    //Unauthorized access, shut it down
    if(requestedSysCall > 0 && requestedSysCall < 9 && callingProc->p_s->cpsr = USRMODE)
    {
        programTrapHandler();
    }

    //Direct to syscall
    switch(requestedSysCall){
        case BIRTH:
            sys1(caller);
        break;

        case DEATH:
            sys2();
        break;

        case SIGNAL:
            sys3(caller);
        break;

        case WAIT:
            sys4(caller);
        break;

        case UNSURE:
            sys5(caller);
        break;

        case CPUTIME:
            sys6(caller);
        break;

        case CLOCKWAIT:
            sys7(caller);
        break;

        case IOWAIT:
            sys8(caller);
        break;

        default: //everything else
            PassUpOrDie(caller, SYSTRAP);
        break;
    }

    //Critical Failure
    PANIC();

}

HIDDEN void sys1(state_PTR callingProc){
    pcb_PTR temp = allocPcb();
    
    if(temp == NULL){
        //no free pcbs
        callingProc-> /*which register*/ =FAILURE;
        LDST(callingProc);
    }
    ++procCount

    //Make new process a progeny of the callingProcess
    insertChild(currentProc, temp)

    //put it on the ready queue
    insertProcQ(&readyQueue, temp);

    callingProc -> /*register*/ = SUCCESS;

    LDST(callingProc);
}



HIDDEN void sys2(){
    if(emptyChild(currentProc)){
        //has no children
        outChild(currrentProc);
        freePcb(currentProc);
        --procCount;
    }
    else{
        //has children.... Must Kill them all
        killAllChildren(currentProc);
    }

    currentProc=NULL;
    scheduler();
}


HIDDEN void sys3(state_PTR callingProc){
    pcb_PTR tempProc=NULL;
    int* sem=(int*) callingProc -> /*register*/;
    sem=*sem+1;

    if(*sem <=0){
        //a proc is waiting on the sem
        tempProc = removeBlocked(sem);
        
        if(tempProc !=NULL){
            //put on ready queue
            insertProcQ(&readyQueue, tempProc);
        }
        //else nothing waiting
    }

    //send back to caller
    LDST(callingProc);
}



HIDDEN void sys4(state_PTR callingProc){
    int* sem = (int*) callingProc -> /*register*/;
    sem=*sem-1;

    if(*sem <0){
        //something controls sem
        insertBlocked(sem, currentProc);
        scheduler();
    }
    //nothing conrols sem
    LDST(callingProc); 

}



HIDDEN void sys5(state_PTR callingProc)
{
    switch(caller-> /*register*/)
    {
        case TLBTRAP:
            if(currentProc->TLB_NEW != NULL)
            {
                sys2(); //already called this once
            }
            currentProc->TLB_NEW = (state_PTR)callingProc-> /*register*/;
            currentProc->TLB_OLD = (state_PTR)callingProc-> /*register*/;
            break;

        case PROGTRAP: 
            if(currentProc -> PRGRM_NEW != NULL)
            {
                sys2(); //already called this once
            }
            currentProc->PRGRM_NEW = (state_PTR)callingProc -> /*register*/;
            currentProc->PRGRM_OLD = (state_PTR)callingProc -> /*register*/;
            break;

        case SYSTRAP: 
            if(currentProc-> SYS_OLD != NULL)
            {
                sys2(); //already called this once
            }
            currentProc->SYS_NEW = (state_PTR)callingProc-> /*register*/;
            currentProc->SYS_OLD = (state_PTR)callingProc-> /*register*/;
            break;
    }
    LDST(callingProc);
}



HIDDEN void sys6(state_PTR callingProc)
{
    //do later b/c time stuff
}



HIDDEN void sys7(state_PTR callingProc)
{
    //do later b/c time stuff
}



HIDDEN void sys8(state_PTR callingProc){
    int lineNumber, deviceNumber, read, index;
    int* sem;

    lineNumber= callingProc ->/*register*/;
    deviceNumber = callingProc ->/*register*/;
    read = callingProc -> /*register*/;

    if(lineNumber<DISK || lineNumber>UMMM){
        //Invalid request
        sys2(); 
    }

    if(lineNumber == UMMM && read==TRUE){
        index = /*indexing trouble*/;
    }
    else{
        index= /*indexing trouble*/;
    }
    sem=&(sem4[index]);
    sem=*sem-1;
    if (*sem < 0){
        insertBlocked(sem, currentProc);
        softBlockCount++;
        scheduler();
    }

    LDST(callingProc);
}




HIDDEN void killAllChildren(pcb_PTR top)
{
   //write later
}


HIDDEN void passUpOrDie(state_PTR callingProc, int cause){
    switch(cause){
        case SYSTRAP:  
            if(currentProc-> SYS_NEW != NULL){
                //sys trap called
                LDST(currentPRoc -> SYS_NEW);
            }
        break;
        case PRGRMTRAP:  
            if(currentProc->PRGRM_NEW != NULL){
                //sys trap called
                LDST(currentPRoc -> PRGRM_NEW);
            }
        break;
        case TLBTRAP:  
            if(currentProc -> TLB_NEW != NULL){
                //sys trap called
                LDST(currentPRoc -> TLB_NEW);
            }
        break;
    }
    sys2();
   
}

/*

tlb()
{
   passUpOrDie()
}

prgrmTrap()
{
   passUpOrDie()
}

passUpOrDie()
{
    if sys5 hasn't been called for the error type, nuke
    otherwise, load the handler that was set by sys5
}

sysCallHandler()
{
    send the caller to the right syscall
}

sys1()
{
    create a new node
    add it to readyqueue
    incremenent processCount
}

sys2()
{
    delete a node
    if that node has children, call killAllChildren()
    decrement procCount
}

killAllChildren(pcb_PTR parent)
{
    recursively decimate the given node's bloodline
    decrement procCount for every child killed

}

sys3(mutex)
{
    increment mutex
    if mutex <= 0
        unblock a process from mutex
        put it on the readyQueue

    return control to the caller
}

wait
sys4(mutex)
{
    decrement mutex
    if mutex < 0
        block currentProc on mutex
        call the scheduler

    otherwise, return control to the caller
}

sys5()
{
    if sys5 has already been called for the given error type, kill the process
    otherwise, give the current process a new exception handler to be used by passUpOrDie
}

sys6(pcb)
{
    calculate the difference between the start time and the current time
    add that time the requesting PCB's time 
}

sys7()
{
    time things we dont know
}

sys8()
{
    find the device that sent the request
    perform a wait operation on the device's semaphore
    increment softBlockedCnt
}
*/