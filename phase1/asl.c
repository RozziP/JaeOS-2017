#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"

HIDDEN semd_t* semdActiveList_h, *semdFreeList_h;

/* 
Initialize the semdFree list to contain all the elements of the array
static semd_t semdTable[MAXPROC]
This method will be only called once during data structure initializa-
tion.
 */
 void initASL()
 {
    static semd_t* semdTable[MAXPROC+2];

    for(int i=0; i<MAXPROC; i++)
    {
      semdTable[i]->s_semAdd = &semdTable[i];
      freePcb(semdTable[i]);
    }

    semdTable[MAXPROC+1]->s_semAdd = 0;
    freePcb(semdTable[MAXPROC+1]);

    semdTable[MAXPROC+1]->s_semAdd = MAX_INT;
    freePcb(semdTable[MAXPROC+2]);
}
 
/* 
Insert the ProcBlk pointed to by p at the tail of the process queue
associated with the semaphore whose physical address is semAdd
and set the semaphore address of p to semAdd. If the semaphore is
currently not active (i.e. there is no descriptor for it in the ASL), allo-
cate a new descriptor from the semdFree list, insert it in the ASL (atthe appropriate position), initialize all of the fields (i.e. set s semAdd
to semAdd, and s procq to mkEmptyProcQ()), and proceed as
above. If a new semaphore descriptor needs to be allocated and the
semdFree list is empty, return TRUE. In all other cases return FALSE.
*/
bool insertBlocked(int *semAdd, pcb_PTR p)
{
   semd_t* temp = find(semAdd);
   if (temp->semAdd != semAdd)
   {
       //TODO
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
    pcb_PTR temp = find(semAdd)
    pcb_PTR ret;

    if (temp == semAdd)
    {
        ret = removeProcQ(temp->s_tp);
        //Was that the last pcb?
        if (emptyProcQ(s_tp)
        {
            //Free up the semd
            freeSemd(temp)
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
    //TODO
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
    pcb_PTR temp = find(semAdd);
    //Return the head of the pcb at that semAdd
    if (temp == semAdd)
    {
        return temp->s_tp->p_next;
    }

    //If we didn't find it...
    return NULL;
}

semd_t* find(int* semAdd)
{
    //we don't like wild goose chases
    if (semAdd == NULL) return NULL;

    semd_t* temp = semdActiveList_h;

   /*
   While we are not at the end of the list, move down the list
   (I'm assuming it's in ascending order)
   */
   while(temp->s_next->s_semAdd < semAdd)
   {
       temp = temp->s_next;
   }
   return temp;
}

/*
analogous to pcb_t's freePcb because we need a way to free up semd structs
*/
void freeSemd(semd_t* s)
{
    //TODO!!
}

/*
Analogous to pcb_t's allocPcb because we need a way to pull semd structs from the free list
*/
semd_t* allocSemd()
{
    //TODO
}