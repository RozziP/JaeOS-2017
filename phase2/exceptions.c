/*
exceptions.c

*/

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/globals.e"
#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/uARMtypes.h"



/*Globals*/

extern int softBlockCnt;
extern int procCount;
extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;
extern int sem4[DEVICES];



HIDDEN void sys1(state_PTR callingProc);
HIDDEN void sys2();
HIDDEN void sys3(state_PTR callingProc);
HIDDEN void sys4(state_PTR callingProc);
HIDDEN void sys5(state_PTR callingProc);
HIDDEN void sys6(state_PTR callingProc);
HIDDEN void sys7(state_PTR callingProc);
HIDDEN void sys8(state_PTR callingProc);
HIDDEN void passUpOrDie(state_PTR callingProc,int cause);
HIDDEN void copyState(state_PTR source, state_PTR destination);
HIDDEN void programTrapHandler();
HIDDEN void tlbManager();

void sysHandler(){
    state_PTR callingProc;
    state_PTR program;
    int requestedSysCall;
    unsigned int callingProcStatus;
    unsigned int temp;
    
    callingProc = (state_PTR) SYS_OLD;
    requestedSysCall=callingProc -> //Which Register?
    callingProcStatus= callingProc -> //status register?

    if(requestedSysCall>0 && requestedSysCall<9 /* && 
        not in kernel mode*/){
        program= (state_PTR) PRGRM_OLD;

        copyState(callingProc, program);

        //cause= privileged instruction
        temp=program->CP15_CAUSE &
        program-> CP15_Cause= 

        programTrapHandler();

    }

    callingProc->pc=callingProc->pc+4;


    //Direct to syscall
    switch(requestedSysCall){
        case NEWCHILDPROC:
            sys1(caller);
        break;
        case KILLPROC:
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
            PassUpOrDie(caller, SYSTRAPHAND);
        break;
    }

    //Critical Failure
    PANIC();

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