/*
exceptions.c

*/

#include "../e/asl.e"
#include "../e/pcb.e"
#include "../e/initial.e"
#include "../e/scheduler.e"
#include "../h/globals.h"
#include "../h/const.h"
#include "../h/types.h"

HIDDEN void sys1(state_t* callingProc);
HIDDEN void sys2();
HIDDEN void sys3(state_t* callingProc);
HIDDEN void sys4(state_t* callingProc);
HIDDEN void sys5(state_t* callingProc);
HIDDEN void sys6(state_t* callingProc);
HIDDEN void sys7(state_t* callingProc);
HIDDEN void sys8(state_t* callingProc);
HIDDEN void killAllChildren(pcb_PTR top);
HIDDEN void passUpOrDie(state_t* callingProc, int cause);

void tlbHandler()
{
    state_t* caller = TLB_OLD;
    passUpOrDie(caller, TLBTRAP);
}

void prgrmTrapHandler()
{
    state_t* caller = PRGRM_OLD;
    passUpOrDie(caller, PRGRMTRAP);
}


void sysHandler(){
    state_t* callingProc;
    state_t* program;
    int requestedSysCall;
    
    state_t* callingProc = (state_t*) SYS_OLD;
    requestedSysCall = callingProc->a1;

    //Unauthorized access, shut it down
    if((requestedSysCall > 0) && (requestedSysCall < 9) && (callingProc->cpsr = USRMODE))
    {
        prgramTrapHandler();
    }

    //Direct to syscall
    switch(requestedSysCall){
        case BIRTH:
            sys1(callingProc);
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
            PassUpOrDie(callingProc, SYSTRAP);
        break;
    }

    //Critical Failure
    PANIC();

}

HIDDEN void sys1(state_t* callingProc){
    pcb_PTR temp = allocPcb();
    
    if(temp == NULL){
        //no free pcbs
        callingProc->a1 = FAILURE;
        LDST(callingProc);
    }
    ++procCount;

    //Make new process a progeny of the callingProcess
    insertChild(currentProc, temp);

    //put it on the ready queue
    insertProcQ(&readyQueue, temp);
    copyState(callingProc, &(temp->p_s));

    callingProc->a1 = SUCCESS;
    LDST(callingProc);
}



HIDDEN void sys2(){
    if(emptyChild(currentProc)){
        //has no children
        outChild(currentProc);
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


HIDDEN void sys3(state_t* callingProc){
    pcb_PTR tempProc = NULL;
    int* sem = (int*)callingProc->a2;
    sem = *sem+1;

    if(*sem <= 0){
        //a proc is waiting on the sem
        tempProc = removeBlocked(sem);
        
        if(tempProc != NULL){
            //put on ready queue
            insertProcQ(&readyQueue, tempProc);
        }
        //else nothing waiting
    }

    //send back to caller
    LDST(callingProc);
}



HIDDEN void sys4(state_t* callingProc){
    int* sem = (int*)callingProc->a2;
    sem = *sem-1;

    if(*sem < 0){
        //something controls sem
        insertBlocked(sem, currentProc);
        scheduler();
    }
    //nothing controls sem
    LDST(callingProc); 

}



HIDDEN void sys5(state_t* callingProc)
{
    switch(callingProc->a1)
    {
        case TLBTRAP:
            if(currentProc->tlbNew != NULL)
            {
                sys2(); //already called this once
            }
            currentProc->sysCallNew=(state_t*)callingProc->a4;
            currentProc->sysCallOld=(state_t*)callingProc->a3;
            break;

        case PRGRMTRAP: 
            if(currentProc->prgrmTrapNew != NULL)
            {
                sys2(); //already called this once
            }
            currentProc->prgrmTrapNew = (state_t*)callingProc->a4;
            currentProc->prgrmTrapOld = (state_t*)callingProc->a3;
            break;

        case SYSTRAP: 
            if(currentProc-> sysCallNew != NULL)
            {
                sys2(); //already called this once
            }
            currentProc->tlbNew=(state_t*) callingProc -> a4;
            currentProc->tlbOld=(state_t*) callingProc -> a3;
            break;
    }
    LDST(callingProc);
}



HIDDEN void sys6(state_t* callingProc)
{
    //Store ending time of day
    STCK(endTOD);
    
    //Store elapsed time
    elapsedTime = endTOD - startTOD;
    currentProc->p_time = currentProc->p_time + elapsedTime;
    remainingTime = remainingTime - elapsedTime;
    
    /*Copy current process time into return register*/		
    currentProc->p_s.a1 = currentProc->p_time;
    
    /*Store starting TOD*/
    STCK(startTOD);
    
    /*Return to previous process*/
    LDST((state_t*)SYS_OLD);
}



HIDDEN void sys7(state_t* callingProc)
{
    sema4[DEVICES] = sema4[DEVICES] - 1;
    if(sema4[DEVICES] < 0)
    {                      
        //Store ending time of day
        STCK(endTOD);
        
        //Store elapsed time
        elapsedTime = endTOD - startTOD;
        currentProc->p_time = currentProc->p_time+elapsedTime;
        remainingTime = remainingTime-elapsedTime;
        
        //Block the process
        insertBlocked(&(sema4[DEVICES]),currentProc);
        currentProc = NULL;
        softBlockCnt++;
        
        scheduler();
    }
    //Shouldn't get here			
    PANIC();
}



HIDDEN void sys8(state_t* callingProc){
    int lineNumber, deviceNumber, read, index;
    int* sem;

    lineNumber = callingProc->a2;
    deviceNumber = callingProc->a3;
    read = callingProc->a4;

    if(lineNumber < DISK || lineNumber > NULLLINES)
    {
        //Invalid request
        sys2(); 
    }

    if(lineNumber == TERMINAL && read == TRUE){
        index = (deviceNumber*8) + (lineNumber + 8);
    }
    else{
        index = index = (deviceNumber*8) + lineNumber;
    }
    sem = &(sema4[index]);
    sem = *sem-1;

    if (*sem < 0){
        insertBlocked(sem, currentProc);
        softBlockCnt++;
        scheduler();
    }

    LDST(callingProc);
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

   //remove the node fromm any semaphores
   if(top->p_semAdd != NULL)
   {
        int* sem = top->p_semAdd;
        outBlocked(top);

        //is it on a device semaphore?
        if(sem >= &(sema4[0]) && sem <= &(sema4[DEVICES-1]))
        {
            softBlockCnt--; //not anymore
        }
        else
        {
        *sem++;
        }
   }

   //RIP
   freePcb(top);
   --procCount;

}


HIDDEN void passUpOrDie(state_t* callingProc,int cause){
    switch(cause){
        case SYSTRAP:  
            if(currentProc->sysCallNew != NULL){
                //sys trap called
                LDST(currentProc -> sysCallNew);
            }
        break;
        case PRGRMTRAP:  
            if(currentProc->prgrmTrapNew != NULL){
                //sys trap called
                LDST(currentProc -> prgrmTrapNew);
            }
        break;
        case TLBTRAP:  
            if(currentProc -> tlbNew != NULL){
                //sys trap called
                LDST(currentProc -> tlbNew);
            }
        break;
    }
    sys2();
}