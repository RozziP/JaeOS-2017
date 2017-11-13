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
HIDDEN void passUpOrDie(int cause);
HIDDEN void reloadCurrentProc();
HIDDEN void callScheduler()

void tlbHandler()
{
    state_t* callingProc = TLB_OLD;
    passUpOrDie(TLBTRAP);
    
}

void prgrmTrapHandler()
{
    state_t* callingProc = PRGRM_OLD;
    passUpOrDie(PRGRMTRAP);
}

cput_t getCurrentTime(){
    cput_t* currentTime = * (cput_t *) TIMEOFDAYLOW / * (cput_t *) TIMESCALE
}


void sysHandler(){
    state_t* callingProc;
    state_t* program;
    int requestedSysCall; 
    
    state_t* callingProc = (state_t*) SYS_OLD; //this is a re-declaration but everything breaks if i dont do it
    requestedSysCall = callingProc->a1;


    //Unauthorized access, shut it down
    if((requestedSysCall > 0) && (requestedSysCall < 9) && (callingProc->cpsr = USRMODE)
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
            PassUpOrDie(callingProc, SYSTRAP);
        break;
    }

    //Critical Failure
    PANIC();

}

HIDDEN void sys1(state_t* callingProc){
    pcb_PTR newPCB = allocPcb();
    
    if(newPCB == NULL){
        //no free pcbs
        callingProc->a1 = FAILURE;
        reloadCurrentProc(callingProc);
    }
    ++procCount;

    //Make new process a progeny of the callingProcess
    insertChild(currentProc, newPCB);

    //put it on the ready queue
    insertProcQ(&readyQueue, newPCB);
    copyState(callingProc->a2, &(newPCB->p_s));

    callingProc->a1 = SUCCESS;
    reloadCurrentProc(callingProc);
}



HIDDEN void sys2(){
    //the way kill all children is written it kills the process we pass in. So we dont really need the if statement
    if(emptyChild(currentProc)){
        //has no children
        outChild(currentProc);
        freePcb(currentProc);
        --procCount;
    }
    else{
        //has children.... Must Kill them all
        killAllChildren(currentProc);
    }

    currentProc=NULL;
    callScheduler();
}


HIDDEN void sys3(state_t* callingProc){

    pcb_PTR tempProc = NULL;
    int* sem = (int*)callingProc->a2;
    *sem = *sem+1;

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
    reloadCurrentProc(callingProc);
}



HIDDEN void sys4(state_t* callingProc){
    int* sem = (int*)callingProc->a2;
    *sem = *sem-1;

    if(*sem < 0){
        //something controls sem
        insertBlocked(sem, currentProc);
        callScheduler();
    }
    //nothing controls sem
    reloadCurrentProc(callingProc); 

}



HIDDEN void sys5(state_t* callingProc)
{
    switch(callingProc->a2)
    {
        case TLBTRAP:
            if(currentProc->tlbNew != NULL)
            {
                killAllChildren(); //already called this once
            }
            currentProc->sysCallNew=(state_t*)callingProc->a4;
            currentProc->sysCallOld=(state_t*)callingProc->a3;
            break;

        case PRGRMTRAP: 
            if(currentProc->prgrmTrapNew != NULL)
            {
                killAllChildren(); //already called this once
            }
            currentProc->prgrmTrapNew = (state_t*)callingProc->a4;
            currentProc->prgrmTrapOld = (state_t*)callingProc->a3;
            break;

        case SYSTRAP: 
            if(currentProc-> sysCallNew != NULL)
            {
                killAllChildren(); //already called this once
            }
            currentProc->tlbNew=(state_t*) callingProc -> a4;
            currentProc->tlbOld=(state_t*) callingProc -> a3;
            break;
    }
    reloadCurrentProc(callingProc);
}



HIDDEN void sys6(state_t* callingProc)
{
    cpu_t currentTime;

    //get current time
     currentTime= getCurrentTime();
    
    //calculate and save the time that has been used
    timeUsed = currentTime - currentProc -> startTime;

    currentProc->p_time = currentProc->p_time + timeUsed;

    currentProc -> startTime = currentTime;
    
    //return resulting process time		
    currentProc->p_s.a1 = currentProc->p_time;
    
    /*Return to previous process*/
    reloadCurrentProc((state_t*)SYS_OLD);
}



HIDDEN void sys7(state_t* callingProc)
{
    sema4[DEVICES] = sema4[DEVICES] - 1;
    if(sema4[DEVICES] < 0)
    {                      
        //Store ending time of day
        STCK(endTimeOfDay);
        
        //Store elapsed time
        elapsedTime = endTimeOfDay - startTimeOfDay;
        currentProc->p_time = currentProc->p_time+elapsedTime;
        remainingTime = remainingTime-elapsedTime;
        
        //Block the process
        insertBlocked(&(sema4[DEVICES]),currentProc);
        currentProc = NULL;
        softBlockCnt++;
        
        callScheduler();
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
    *sem = *sem-1;

    if (*sem < 0){
        insertBlocked(sem, currentProc);
        softBlockCnt++;
        callScheduler();
    }

    reloadCurrentProc(callingProc);
}




HIDDEN void killAllChildren(pcb_PTR top)
{
   while(!emptyChild(top)){
       //drink the punch
       killAllChildren(removeChild(top));
   }

   //is our node the current process?
   if(top == currentProc){
        outChild(currentProc); //not anymore
   }
   //if not, then it's on the readyqueue
   else{
        outProcQ(&readyQueue, top);
   }

   //remove the node fromm any semaphores
   if(top->p_semAdd != NULL){
        int* sem = top->p_semAdd;
        outBlocked(top);

        //is it on a device semaphore?
        if(sem >= &(sema4[0]) && sem <= &(sema4[DEVICES-1])){
            softBlockCnt--; //not anymore
        }
        else{
        *sem++;
        }
   }

   //RIP
   --procCount;
   freePcb(top);
   

}


HIDDEN void passUpOrDie(int cause){


    //I think that in the if statements it should be sysCallOld
    //I changed them but I'm putting this here so we know I did that
    switch(cause){
        case SYSTRAP:  
            if(currentProc->sysCallOld != NULL){
                //sys trap called

                copyState(SYS_OLD,currentProc->sysCallOld);
                reloadCurrentProc(currentProc -> sysCallNew);
            }
        break;
        case PRGRMTRAP:  
            if(currentProc->prgrmTrapOld != NULL){
                //programTrap called
                copyState(PRGRM_OLD,currentProc->prgrmTrapOld)
                reloadCurrentProc(currentProc -> prgrmTrapNew);
            }
        break;
        case TLBTRAP:  
            if(currentProc -> tlbOld != NULL){
                //tlb  Trap called
                copyState(TLB_OLD,currentProc->tlbOld)
                reloadCurrentProc(currentProc -> tlbNew);
            }
        break;
    }
    
    killAllChildren(currentProc);
    //sys2();
    // Think we need to call killAllChildren Directly from here instead 
    //of making a sys call inside a sys call
}


//loads current process 
void reloadCurrentProc(state_t* stateToLoad){

    LDST(&stateToLoad);

}

//call the Scheduler
void callScheduler(){
    scheduler();
}

