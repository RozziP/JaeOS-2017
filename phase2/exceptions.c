/*==========================================================================*\
  exceptions.c

  Contains functions to handle program traps, TLB traps, and system calls in the OS
  
  Syscalls 1-8 are only handled if the calling state is in system mode, otherwise
  we treat the call as a program trap
  Syscalls above 8 are killed.

  Sys1: Creates a new process and makes it a child of the current process
  Sys2: Recursively kills the given process and all of its children
  Sys3: Verhogen. Signals the semaphore of the current process
  Sys4: Passeren. Caused the current process to wait on its semaphore
  Sys5: Sets up secondary exception handlers
  Sys6: Returns the time used by the current process
  Sys7: Performs a wait operation on the clock's semaphore
  Sys8: Performs a wait operaton on a device semaphore

  Authors: Peter Rozzi and Patrick Gemperline
  Date: 11-23-17
\*==========================================================================*/

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../e/interrupts.e"

#include "../h/const.h"
#include "../h/types.h"
#include "/usr/include/uarm/libuarm.h"

HIDDEN void sys1();
HIDDEN void sys2();
HIDDEN void sys3();
HIDDEN void sys4();
HIDDEN void sys5();
HIDDEN void sys6();
HIDDEN void sys7();
HIDDEN void sys8();
HIDDEN void killAllChildren(pcb_PTR top);
HIDDEN void passUpOrDie(int cause);

int lineNumber;
int deviceNumber;

cput_t endTimeOfDay;
cput_t timeUsed;

HIDDEN void exBreak(unsigned int x){
    return;
}

/*
*Pass up a TLB trap to it's secondary exception vector.
*If it has no secondary exception vector, it is killed.
*/
void tlbHandler()
{
    passUpOrDie(TLBTRAP);
}

/*
*Pass up a program trap to it's secondary exception vector.
*If it has no secondary exception vector, it is killed.
*/
void prgrmTrapHandler()
{
    passUpOrDie(PRGRMTRAP);
}

/*
*Calls a different function based on the requested syscall
*If syscalls 1-8 were called in user mode, triggers a program trap
*if syscalls 9-255 were called at all, triggers a sys trap
*/
void sysCallHandler(){
    state_t* program;
    int requestedSysCall; 
    
    state_t* callingProc = (state_t*) SYS_OLD; 
    

    copyState((state_t*) SYS_OLD, &(currentProc -> p_s));

    requestedSysCall = currentProc->p_s.a1;

    //Unauthorized access, shut it down
    
    if((callingProc->cpsr & SYSMODE)!=SYSMODE)
    {
        copyState((state_t*) SYS_OLD , (state_t*) PRGRM_OLD);

        ((state_t*) PRGRM_OLD )-> CP15_Cause = RI;

        exBreak(0x800000);

        passUpOrDie(PRGRM_OLD);

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
            exBreak(0x900000);
            passUpOrDie(SYSTRAP);
        break;
    }

    
    passUpOrDie(SYSTRAP);

    //Critical Failure
    PANIC();


}

/*
 * Attempts to allocate a new PCB.
 * If there are no free PCBs, we return a failure code
 * Otherwise, we make the new PCB a child of the current process, 
 * put it on the ready queue, and return a success code.
 */
HIDDEN void sys1(){

    pcb_PTR newPCB = allocPcb();
    
    if(newPCB == NULL){
        //no free pcbs
        currentProc->p_s.a1 = FAILURE;
    }
    else
    {
        currentProc->p_s.a1 = SUCCESS;

        procCount=procCount+1;

        //Make new process a progeny of the callingProcess
        insertChild(currentProc, newPCB);
        //put it on the ready queue
        insertProcQ(&readyQueue, newPCB);
        //copy the old state into the new process
        copyState((state_t *)currentProc->p_s.a2, &(newPCB->p_s));
    }
    //load the current/new process
    loadState(&(currentProc->p_s));
}


/*
 * Kills the current process and all of its children
 * then calls the scheduler
 */
HIDDEN void sys2(){
    //end the bloodline
    exBreak(procCount);
    exBreak(softBlockCnt);
    killAllChildren(currentProc);
    exBreak(procCount);
    exBreak(softBlockCnt);
    currentProc=NULL;
    scheduler();
}

/*
 * Signal the semaphore that the current process is blocked on. 
 * If the semaphore is less than or equal to 0, unblock a process,
 * add it to the ready queue and then return to the current process
 */
HIDDEN void sys3(){

    pcb_PTR tempProc = NULL;
    int* sem = (int*)currentProc->p_s.a2;
    *sem = *sem+1;

    //if a process is waiting on this semaphore...
    if(*sem <= 0)
    {
        //unblock the process
        tempProc = removeBlocked(sem);
        
        //put on ready queue
        tempProc -> p_semAdd = NULL;
        insertProcQ(&(readyQueue), tempProc);
        softBlockCnt=softBlockCnt-1;
        endTimeOfDay=getTODLO();

    }

    //send back to caller
    loadState(&(currentProc->p_s));
}


/*
 * Perform a wait on the current process' semaphore.
 * if the semaphore is less than 0, store the time used by the process,
 * block the process, then call the scheduler.
 * otherwise, return to the current process.
 */
HIDDEN void sys4(){
    int* sem = (int*)currentProc->p_s.a2;
    *sem = *sem-1;

    if(*sem < 0)
    {
        //keep track of elapsed time
        endTimeOfDay=getTODLO();
        timeUsed = endTimeOfDay - startTimeOfDay;
        currentProc->p_time = currentProc->p_time + timeUsed;

        //something controls sem
        insertBlocked(sem, currentProc);
        softBlockCnt=softBlockCnt+1;
        scheduler();
    }
    //nothing controls sem
    loadState(&(currentProc->p_s)); 

}

/*
 * Replaces an exception handler state with a custom, parameterized state
 */
HIDDEN void sys5()
{
    switch(currentProc->p_s.a2)
    {
        case TLBTRAP:
            if(currentProc->tlbNew != NULL)
            {
                sys2(currentProc); //already called this once
            }
            currentProc->sysCallNew = (state_t*)currentProc->p_s.a4;
            currentProc->sysCallOld = (state_t*)currentProc->p_s.a3;
            break;

        case PRGRMTRAP: 
            if(currentProc->prgrmTrapNew != NULL)
            {
                sys2(currentProc); //already called this once
            }
            currentProc->prgrmTrapNew = (state_t*)currentProc->p_s.a4;
            currentProc->prgrmTrapOld = (state_t*)currentProc->p_s.a3;
            break;

        case SYSTRAP: 
            if(currentProc-> sysCallNew != NULL)
            {
                sys2(currentProc); //already called this once
            }
            currentProc->tlbNew = (state_t*)currentProc->p_s.a4;
            currentProc->tlbOld = (state_t*)currentProc->p_s.a3;
            break;
    }
    loadState(&(currentProc->p_s));
}


/*
 * Returns the CPU time that the current process has used sine its birth
 */
HIDDEN void sys6()
{
    //get current time
    endTimeOfDay = getTODLO();
    
    //calculate and save the time that has been used
    timeUsed = endTimeOfDay - startTimeOfDay;

    currentProc->p_time = currentProc->p_time + timeUsed;
    
    //return resulting process time		
    currentProc->p_s.a1 = currentProc->p_time;

    startTimeOfDay = endTimeOfDay;
    
    //return the to process
    loadState(&(currentProc->p_s));
}


/*
Make the clock wait, and also save the elapsed time
*/
HIDDEN void sys7()
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
HIDDEN void sys8(){
    int isRead, index;
    int* sem;

    lineNumber = currentProc->p_s.a2;
    deviceNumber = currentProc->p_s.a3;
    isRead = currentProc->p_s.a4;

    //If the terminal is reading...
    if((lineNumber == TERMINAL) && isRead)
    {
        index = DEVICEPERLINE * (lineNumber - NULLLINES) + deviceNumber;
    }
    //else the terminal is writing so add 8 to get the terminal device
    index = DEVICEPERLINE * (lineNumber - NULLLINES) + deviceNumber + DEVICEPERLINE;


    sem = &(sema4[index]);
    *sem = *sem-1;

    if (*sem < 0){

        //Save the final time of process
        endTimeOfDay = getTODLO();
        
        //Save the time used since last recording
        timeUsed = endTimeOfDay - startTimeOfDay;
        currentProc -> p_time = currentProc -> p_time + timeUsed;

        insertBlocked(sem, currentProc);
        softBlockCnt++;
        currentProc=NULL;
        scheduler();
    }
    currentProc->p_s.a1 = semaStat[index];
    loadState(&(currentProc->p_s));
}



/*
 * Sys2 helper function. 
 * Recursively deletes a node and all of its children
 */
HIDDEN void killAllChildren(pcb_PTR top)
{
   int* sem = top->p_semAdd;
   while(!emptyChild(top))
   {
       //drink the punch
       killAllChildren(removeChild(top));
   }
   if(sem != NULL) //remove the node from any semaphores
   {
        outBlocked(top);
        //is it on a device semaphore?
        if((sem >= &(sema4[0])) && (sem <= &(sema4[DEVICES-1])))
        {
            softBlockCnt--; //not anymore
        }
        else //it's not softblocked
        {
            *sem = *sem+1;
        }
   }
   else if(top == currentProc) //is our node the current process?
   {
        outChild(top); //not anymore
   }
   else //then it's on the readyqueue
   {
        outProcQ(&(readyQueue), top);
   }

   //RIP
   freePcb(top);
   procCount = procCount-1;
   
}

/*
 * Passes an exception to its appropriate handler defined by Sys5
 * if Sys5 was not called for the given exception type, kill the current
 * process and all of its children.
 */
HIDDEN void passUpOrDie(int cause){

    switch(cause){
        case SYSTRAP:  
            if(currentProc->sysCallOld != NULL){
                //systrap called
                copyState((state_t*)SYS_OLD, (state_t*)currentProc->sysCallOld);
                copyState((state_t*)sysCallNew, &(currentProc->p_s));
                loadState(currentProc->sysCallNew);
            }
        break;
        case PRGRMTRAP:  
            if(currentProc->prgrmTrapOld != NULL){
                //prgrmTrap called
                copyState((state_t*)PRGRM_OLD, (state_t*)currentProc->prgrmTrapOld);
                copyState((state_t*)prgrmTrapNew,  &(currentProc->p_s));
                loadState(currentProc->prgrmTrapNew);
            }
        break;
        case TLBTRAP:  
            if(currentProc -> tlbOld != NULL)
            {
                //tlbTrap called
                copyState((state_t*)TLB_OLD, (state_t*)currentProc->tlbOld);
                copyState((state_t*)tlbNew,  &(currentProc->p_s));
                loadState(currentProc->tlbNew);
            }
        break;
    }
    
    //there was no handler to pass up to... guess I'll die.
    sys2();
}




