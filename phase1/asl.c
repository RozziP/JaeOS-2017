#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"

HIDDEN semd_t* semdActiveList_h, *semdFreeList_h;
HIDDEN semd_t* find(int* semAdd);
HIDDEN semd_t* allocSemd();

HIDDEN void freeSemd(semd_t* s);
HIDDEN void insertSemd(semd_t* head, semd_t* s);
HIDDEN semd_t* outSemd(semd_t* head, semd_t* s); 



/* 
Initialize the semdFree list to contain all the elements of the array
static semd_t semdTable[MAXPROC]
This method will be only called once during data structure initializa-
tion.
 */
 void initASL()
 {
    static semd_t semdTable[MAXPROC+2];

    for(int i=0; i<MAXPROC; i++)
    {
      freeSemd(&semdTable[i]);
    }

    semdTable[MAXPROC+1].s_semAdd = 0;
    semdTable[MAXPROC+1].s_next	= &semdTable[MAXPROC+2];
    semdTable[MAXPROC+1].s_prev	= NULL;
    semdTable[MAXPROC+1].s_tp	= NULL;
    semdActiveList_h = &semdTable[MAXPROC+1];

    semdTable[MAXPROC+2].s_semAdd = (int*)MAX_INT;
    semdTable[MAXPROC+2].s_next	= NULL;
    semdTable[MAXPROC+2].s_prev	= &semdTable[MAXPROC+1];
    semdTable[MAXPROC+2].s_tp	= NULL;
    semdActiveList_h->s_next 	= &semdTable[MAXPROC+2];
}

/*=========================================PROCESS BLOCK FUNCTIONS==============================================*\
\*==============================================================================================================*/
 
/* 
Insert the ProcBlk pointed to by p at the tail of the process queue
associated with the semaphore whose physical address is semAdd
and set the semaphore address of p to semAdd. If the semaphore is
currently not active (i.e. there is no descriptor for it in the ASL), allo-
cate a new descriptor from the semdFree list, insert it in the ASL 
(at the appropriate position), initialize all of the fields (i.e. set s semAdd
to semAdd, and s procq to mkEmptyProcQ()), and proceed as
above. If a new semaphore descriptor needs to be allocated and the
semdFree list is empty, return TRUE. In all other cases return FALSE.
*/
bool insertBlocked(int* semAdd, pcb_PTR p)
{
   semd_t* temp = find(semAdd);

   if (temp->s_semAdd == semAdd)
   {
   insertProcQ(&temp->s_tp, p);
   p->p_semAdd = semAdd;

   return FALSE;
   }
   //Allocate a new semaphore
   else
   {
    semd_t* semToAdd = allocSemd();
    if (temp == NULL) return TRUE;

    //Insert the new semd into the ASL at the appopriate location
    semToAdd->s_prev = temp->s_prev;
    semToAdd->s_next = temp;
    temp->s_prev->s_next = semToAdd;
    temp->s_prev = semToAdd;

    semToAdd->s_semAdd = semAdd;
    insertProcQ(&(semToAdd->s_tp), p);
    p->p_semAdd = semAdd;
    
    return FALSE;
   }
}

/* 
Search the ASL for a descriptor of this semaphore. If none is
found, return NULL; otherwise, remove the first (i.e. head) ProcBlk
from the process queue of the found semaphore descriptor and re-
turn a pointer to it. If the process queue for this semaphore becomes
empty (emptyProcQ(s procq) is TRUE), remove the semaphore
descriptor from the ASL and return it to the semdFree list. 
*/
pcb_PTR removeBlocked(int *semAdd)
{
    pcb_PTR ret;
    semd_t* temp = find(semAdd);

    if (temp->s_semAdd == semAdd)
    {
        //save the removed pcb to return
        ret = removeProcQ(&temp->s_tp);
        //Was that the last pcb?
        if (emptyProcQ(temp->s_tp))
        {
            //Remove the semd from the ASL and put it on the free list
            temp->s_next->s_prev = temp->s_prev;
            temp->s_prev->s_next = temp->s_next;
            temp->s_next = NULL;
            temp->s_prev = NULL;

            freeSemd(temp);
        }
        return ret;
    }
    //If we didn't find it...
    return NULL;
}

/* 
Remove the ProcBlk pointed to by p from the process queue asso-
ciated with p’s semaphore (p→ p semAdd) on the ASL. If ProcBlk
pointed to by p does not appear in the process queue associated with
p’s semaphore, which is an error condition, return NULL; otherwise,
return p. 
*/
pcb_PTR outBlocked(pcb_PTR p)
{

   pcb_PTR ret;
   semd_t* temp = find(p->p_semAdd);

   if (temp->s_semAdd == p->p_semAdd)
   {
    //remove p and save a pointer to it
    ret = outProcQ(&temp->s_tp, p);

    //Was the process queue empty?
    if (ret == NULL) return NULL;

    //Is it empty now?
    if (emptyProcQ(temp->s_tp))
    {
        //Remove the semd from the ASL and put it on the free list
        temp->s_next->s_prev = temp->s_prev;
        temp->s_prev->s_next = temp->s_next;
        temp->s_next = NULL;
        temp->s_prev = NULL;

        freeSemd(temp);
    }
    return ret;
   }
return NULL;
}

/* 
Return a pointer to the ProcBlk that is at the head of the pro-
cess queue associated with the semaphore semAdd. Return NULL
if semAdd is not found on the ASL or if the process queue associ-
ated with semAdd is empty.
 */
pcb_PTR headBlocked(int *semAdd)
{
    //Search the list for a semd holding the given semAdd
    semd_t* temp = find(semAdd);
    if (temp == NULL) return NULL;

    //Return the head of the pcb at that semAdd
    if (temp->s_semAdd == semAdd)
    {
        return temp->s_tp->p_next;
    }

    //If we didn't find it...
    return NULL;
}

/*=========================================ACTIVE LIST FUNCTIONS================================================*\
\*==============================================================================================================*/


/*
Searches the active semaphore list for a semaphore directory containing the given semAdd. 
If the semAdd is null, return NULL.
If the semAdd is found, return its containing semaphore directory.
If the semAdd does not exist, return the last non-dummy element of the list.
*/
semd_t* find(int* semAdd)
{
    if (semAdd == NULL) return NULL;

    semd_t* temp = semdActiveList_h;
   //while the next value is less than or equal the target value, go to next node
   while(temp->s_next->s_semAdd <= semAdd)
   {
       temp = temp->s_next;
   }
    // for (int i = 0; i < MAXPROC; i++)
    // {
    //     if(temp->s_semAdd == semAdd) break;
    //     temp = temp->s_next;
    // }
   return temp;
}

/*======================================FREE LIST MODIFIER FUNCTIONS============================================*\
\*==============================================================================================================*/

/*
Put a node on the free list
*/
void freeSemd(semd_t* s)
{
    s->s_next = semdFreeList_h;
    semdFreeList_h->s_prev = s;
    semdFreeList_h = s;
}

/*
Pull a node off of the free list, null its values and return it.
*/
semd_t* allocSemd()
{
    semd_t* temp = semdFreeList_h;

    if (temp == NULL) return NULL;

    semdFreeList_h->s_prev = NULL;
    semdFreeList_h = semdFreeList_h->s_next;

    temp->s_next   = NULL;
    temp->s_prev   = NULL;
    temp->s_semAdd = NULL;
    temp->s_tp     = NULL;

    return temp;
}