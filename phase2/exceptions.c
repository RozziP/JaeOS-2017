/*
exceptions.c

*/

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/interrupts.e"

#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"

HIDDEN void sys1(state_t* callingProc);
HIDDEN void sys2();
HIDDEN void sys3(state_t* callingProc);
HIDDEN void sys4(state_t* callingProc);
HIDDEN void sys5(state_t* callingProc);
HIDDEN void sys6(state_t* callingProc);
HIDDEN void sys7(state_t* callingProc);
HIDDEN void sys8(state_t* callingProc);
HIDDEN void killAllChildren(pcb_PTR top);
HIDDEN void passUpOrDie(int cause);

int lineNumber;
int deviceNumber;

HIDDEN void pthB(){
    return;
}

void tlbHandler()
{

    passUpOrDie(TLBTRAP);
    
}

void prgrmTrapHandler()
{
    passUpOrDie(PRGRMTRAP);
}

void sysCallHandler(){
    state_t* program;
    int requestedSysCall; 
    
    state_t* callingProc = (state_t*) SYS_OLD; //this is a re-declaration but everything breaks if i dont do it
    requestedSysCall = callingProc->a1;


    //Unauthorized access, shut it down
    
    if((callingProc->cpsr & USRMODE)==0)
    {
        copyState((state_t*) SYS_OLD , (state_t*) PRGRM_OLD);

        ((state_t*) PRGRM_OLD )-> CP15_Cause = RI;

        passUpOrDie(SYSTRAP);

    }
    //Direct to syscall
    switch(requestedSysCall){
        case BIRTH:
            sys1(callingProc);
        break;

        case DEATH:
            sys2();
        break;

        case VERHOGEN:
            sys3(callingProc);
        break; 

        case PASSEREN:
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
            passUpOrDie(SYSTRAP);
        break;
    }

    //maybe pass up or die?

    //Critical Failure
    PANIC();


}

HIDDEN void sys1(state_t* callingProc){

    pcb_PTR newPCB = allocPcb();
    
    if(newPCB == NULL){
        //no free pcbs
        callingProc->a1 = FAILURE;
    }
    else
    {
        callingProc->a1 = SUCCESS;

        ++procCount;

        //Make new process a progeny of the callingProcess
        insertChild(currentProc, newPCB);
        //put it on the ready queue
        insertProcQ(&readyQueue, newPCB);
        //copy the old state into the new process
        copyState((state_t *)callingProc->a2, &(newPCB->p_s));
    }
    //load the current/new process
    loadState(&(currentProc->p_s));
}



HIDDEN void sys2(){
    //end the bloodline
    killAllChildren(currentProc);
    currentProc=NULL;
    scheduler();
}


HIDDEN void sys3(state_t* callingProc){

    pcb_PTR tempProc = NULL;
    int* sem = (int*)callingProc->a2;
    *sem = *sem+1;

    //if a process is waiting on this semaphore...
    if(*sem <= 0)
    {
        //unblock the process
        tempProc = removeBlocked(sem);
        
        
        if(tempProc != NULL)
        {
            //put on ready queue
            tempProc -> p_semAdd = NULL;
            insertProcQ(&readyQueue, tempProc);
        }
        //else nothing waiting
    }

    //send back to caller
    loadState(callingProc);
}



HIDDEN void sys4(state_t* callingProc){
    int* sem = (int*)callingProc->a2;
    *sem = *sem-1;

    if(*sem < 0)
    {
        //TODO keep track of elapsed time

        //something controls sem
        pthB();
        insertBlocked(sem, currentProc);
        pthB();
        scheduler();
    }
    //nothing controls sem
    loadState(callingProc); 

}

HIDDEN void sys5(state_t* callingProc)
{
    switch(callingProc->a2)
    {
        case TLBTRAP:
            if(currentProc->tlbNew != NULL)
            {
                killAllChildren(currentProc); //already called this once
            }
            currentProc->sysCallNew=(state_t*)callingProc->a4;
            currentProc->sysCallOld=(state_t*)callingProc->a3;
            break;

        case PRGRMTRAP: 
            if(currentProc->prgrmTrapNew != NULL)
            {
                killAllChildren(currentProc); //already called this once
            }
            currentProc->prgrmTrapNew = (state_t*)callingProc->a4;
            currentProc->prgrmTrapOld = (state_t*)callingProc->a3;
            break;

        case SYSTRAP: 
            if(currentProc-> sysCallNew != NULL)
            {
                killAllChildren(currentProc); //already called this once
            }
            currentProc->tlbNew=(state_t*) callingProc -> a4;
            currentProc->tlbOld=(state_t*) callingProc -> a3;
            break;
    }
    loadState(callingProc);
}



HIDDEN void sys6(state_t* callingProc)
{
    //get current time
    endTimeOfDay = getTODLO();
    
    //calculate and save the time that has been used
    timeUsed = endTimeOfDay - startTimeOfDay;
    currentProc->p_time = currentProc->p_time + timeUsed;
    
    //return resulting process time		
    currentProc->p_s.a1 = currentProc->p_time;

    startTimeOfDay = getTODLO();
    
    //return the to process
    loadState((state_t*)SYS_OLD);
}


/*
Make the clock wait, and also save the elapsed time
*/
HIDDEN void sys7(state_t* callingProc)
{
    int clockSem = DEVICES-1;
    //decrement clock semaphore
    sema4[clockSem] = sema4[clockSem] - 1;

    if(sema4[clockSem] < 0)
    {                      
        //Store ending time of day
        endTimeOfDay=getTODLO();
        
        //Store elapsed time
        timeUsed = endTimeOfDay - startTimeOfDay;
        currentProc -> p_time = currentProc -> p_time + timeUsed;
        timeLeft = timeLeft - timeUsed;
        
        //Block the process
        insertBlocked(&(sema4[clockSem]), currentProc);
        currentProc = NULL;
        softBlockCnt++;
        
        scheduler();
    }
    //Shouldn't get here			
    PANIC();
}


/*
Tell a device to wait
*/
HIDDEN void sys8(state_t* callingProc){
    int isRead, index, sem;

    lineNumber = callingProc->a2;
    deviceNumber = callingProc->a3;
    isRead = callingProc->a4;


    //If the terminal is reading...
    if((lineNumber == TERMINAL) && isRead)
    {
        semIndex= 8*(lineNumber-3) + deviceNumber + 8;
    }
    //else the terminal is writing so add 8 to get the terminal device
    else
    {
        semIndex= 8*(lineNumber-3) + deviceNumber + 8;
    }

    sem = &(sema4[semIndex]);
    *sem = *sem-1;

    if (*sem < 0){

        //Save the final time of process
        endTimeOfDay=getTODLO();
        
        //Save the time used since last recording
        timeUsed = endTimeOfDay - startTimeOfDay;
        currentProc -> p_time = currentProc -> p_time + timeUsed;

        insertBlocked(sem, currentProc);
        softBlockCnt++;
        currentProc=NULL;
        scheduler();
    }
    //currentProc -> p_s.a1 = sema4[]
    loadState(callingProc);
}




HIDDEN void killAllChildren(pcb_PTR top)
{
   while(!emptyChild(top))
   {
       //drink the punch
       killAllChildren(removeChild(top));
   }

   //is our node the current process?
   if(top == currentProc)
   {
        outChild(currentProc); //not anymore
   }
   //if not, then it's on the readyqueue
   else
   {
        outProcQ(&readyQueue, top);
   }

   //remove the node from any semaphores
   if(top->p_semAdd != NULL)
   {
        int* sem = top->p_semAdd;
        outBlocked(top);

        //is it on a device semaphore?
        if(sem >= &(sema4[0]) && sem <= &(sema4[DEVICES-1]))
        {
            softBlockCnt--; //not anymore
        }
        //else, it's not softblocked
        else
        {
            *sem++;
        }
   }

   //RIP
   --procCount;
   freePcb(top);
}


HIDDEN void passUpOrDie(int cause){

    switch(cause){
        case SYSTRAP:  
            if(currentProc->sysCallOld != NULL){
                //systrap called
                copyState((state_t*)SYS_OLD, (state_t*) currentProc->sysCallOld);
                copyState((state_t*) SYS_NEW, &currentProc -> p_s);
                loadState(currentProc->sysCallNew);
            }
        break;
        case PRGRMTRAP:  
            if(currentProc->prgrmTrapOld != NULL){
                //prgrmTrap called
                copyState((state_t*) PRGRM_OLD, (state_t*) currentProc->prgrmTrapOld);
                copyState((state_t*) PRGRM_NEW,  &currentProc -> p_s);
                loadState(currentProc->prgrmTrapNew);
            }
        break;
        case TLBTRAP:  
            if(currentProc -> tlbOld != NULL)
            {
                //tlbTrap called
                copyState((state_t*) TLB_OLD, (state_t*) currentProc->tlbOld);
                copyState((state_t*) TLB_NEW,  &currentProc -> p_s);
                loadState(currentProc->tlbNew);
            }
        break;
    }
    
    //there was no handler to pass up to... guess I'll die.
    killAllChildren(currentProc);
}




