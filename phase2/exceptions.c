/*
exceptions.c

*/

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/globals.h"
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/uARMtypes.h"



/*Globals*/

extern int softBlockCnt;
extern int procCount;
extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;
extern int sem4[DEVICES];



HIDDEN void sys1(state_t callingProc);
HIDDEN void sys2();
HIDDEN void sys3(state_t callingProc);
HIDDEN void sys4(state_t callingProc);
HIDDEN void sys5(state_t callingProc);
HIDDEN void sys6(state_t callingProc);
HIDDEN void sys7(state_t callingProc);
HIDDEN void sys8(state_t callingProc);
HIDDEN void KILLALLTHECHILDREN(pcb_PTR top);
HIDDEN void passUpOrDie(state_t callingProc,int cause);
HIDDEN void programTrapHandler();
HIDDEN void tlbManager();

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

void sysHandler(){
    state_t callingProc;
    state_t program;
    int requestedSysCall;
    
    callingProc = (state_PTR) SYS_OLD;
    requestedSysCall = callingProc-> a1;

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
            sys3(callingProc);
        break;

        case WAIT:
            sys4(callingProc);
        break;

        case ESV:
            sys5(callingProc);
        break;

        case CPUTIME:
            sys6(callingProc);
        break;

        case CLOCKWAIT:
            sys7(callingProc);
        break;

        case IOWAIT:
            sys8(callingProc);
        break;

        default: //everything else
            PassUpOrDie(callingProc, SYSTRAPHAND);
        break;
    }

    //Critical Failure
    PANIC();

}

HIDDEN void sys1(state_t callingProc){
    pcb_PTR temp= allocPcb();
    
    if(temp == NULL){
        //no free pcbs
        callingProc -> a1 = FAILURE;
        LDST(callingProc);
    }
    ++procCount

    //Make new process a progeny of the callingProcess
    insertChild(currentProc, temp)

    //put it on the ready queue
    insertProcQ(&readyQueue, temp);

    callingProc -> a1 = SUCCESS;

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
        KILLALLTHECHILDREN(currentProc);
    }

    currentProc=NULL;
    scheduler();
}


HIDDEN void sys3(state_t callingProc){
    pcb_PTR tempProc=NULL;
    int* sem=(int*) callingProc -> a2;
    sem=*sem+1;

    if(*sem <=0){
        //a proc is waiting on the sem
        tempProc=removeBlocked(sem);
        
        if(tempProc !=NULL){
            //put on ready queue
            insertProcQ(&readyQueue, tempProc);
        }
        //else nothing waiting
    }

    //send back to caller
    LDST(callingProc);
}



HIDDEN void sys4(state_t callingProc){
    int* sem=(int*) callingProc -> a2;
    sem=*sem-1;

    if(*sem <0){
        //something controls sem
        insertBlocked(sem, currentProc);
        scheduler();
    }
    //nothing controls sem
    LDST(callingProc); 

}



HIDDEN void sys5(state_t callingProc){
    switch(callingProc -> a2){
        case SYSTRAPHAND: 
            if(currentProc -> sysCallNew != NULL){
                sys2(); //already called this once
            }
            currentProc -> sysCallNew=(state_t) callingProc -> a4;
            currentProc -> sysCallOld=(state_t) callingProc -> a3;
            break;

        case PROGTRAPHAND: 
            if(currentProc -> programTrapNew != NULL)
            {
                sys2(); //already called this once
            }
            currentProc -> programTrapNew=(state_t) callingProc -> a4;
            currentProc -> programTrapOld=(state_t) callingProc -> a3;
            break;

        case SYSTRAPHAND: 
            if(currentProc-> SYS_OLD != NULL)
            {
                sys2(); //already called this once
            }
            currentProc -> tlbNew=(state_t) callingProc -> a4;
            currentProc -> tlbOld=(state_t) callingProc -> a3;
            break;
    }
    LDST(callingProc);
}



HIDDEN void sys6(state_t callingProc){
    //do later b/c time stuff
}



HIDDEN void sys7(state_t callingProc){
    //do later b/c time stuff
}



HIDDEN void sys8(state_t callingProc){
    int lineNumber, deviceNumber, read, index;
    int* sem;

    lineNumber= callingProc -> a2;
    deviceNumber = callingProc ->a3;
    read = callingProc -> a4;

    if(lineNumber<DISK || lineNumber>UMMM){
        //Invalid request
        sys2(); 
    }

    if(lineNumber == TERMINAL && read==TRUE){
        index = deviceNumber*8+lineNumber+8;
    }
    else{
        index= index = deviceNumber*8+lineNumber;
    }
    sem=&(sem4[index]);
    sem=*sem-1;
    if (*sem<0){
        insertBlocked(sem, currentProc);
        softBlockCount++;
        scheduler();
    }

    LDST(callingProc);
}




HIDDEN void KILLALLCHILDREN(pcb_PTR top)
{
   //write later
}


HIDDEN void passUpOrDie(state_t callingProc,int cause){
    switch(cause){
        case SYSTRAPHAND:  
            if(currentProc -> sysCallNew != NULL){
                //sys trap called
                LDST(currentPRoc -> sysCallNew);
            }
        break;
        case PROGTRAPHAND:  
            if(currentProc -> programTrapNew != NULL){
                //sys trap called
                LDST(currentPRoc -> programTrapNew);
            }
        break;
        case TLBTRAPHAND:  
            if(currentProc -> tlbNew != NULL){
                //sys trap called
                LDST(currentPRoc -> tlbNew);
            }
        break;
    }
    sys2();
}




HIDDEN void programTrapHandler(){
    state_t callingProc = (state_t) TLB_OLD;
	PassUpOrDie(callingProc, TLBTRAPHAND);
}



HIDDEN void tlbManager(){
    state_PTR callingProc = (state_PTR) PRGRM_OLD;
	PassUpOrDie(callingProc, PROGTRAPHAND;
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