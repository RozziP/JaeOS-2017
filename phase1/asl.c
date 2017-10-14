/*=======================================================================================*\
  asl.c
  Manages the active and free semaphore lists, which are
  both singly linked, linear lists and in sorted order.
  The active semaphore list contains two dummy nodes for ease of removal and insertion.

  Any semaphore descriptor that is not in use is stored on the free list.
  Any semaphore descriptor with a non-empty process queue is stored on the active list.
  
  Authors: Peter Rozzi and Patrick Gemperline
  Date: 9-13-17
\*=======================================================================================*/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"

HIDDEN semd_PTR semdActiveList_h, semdFreeList_h;

HIDDEN semd_PTR find(int* semAdd);
HIDDEN semd_PTR allocSemd();
HIDDEN void freeSemd(semd_PTR s);


/* 
Initialize the semdFree list to contain all the elements of the array
static semd_t semdTable[MAXPROC]
This method will be only called once during data structure initializa-
tion.
 */
 void initASL()
 {
    semdActiveList_h = NULL;
    semdFreeList_h   = NULL;

    static semd_t semdTable[MAXPROC+2];

    for(int i=0; i<MAXPROC; i++)
    {
      freeSemd(&semdTable[i]);
    }

    //Set up the dummy nodes with values 0 and MAX_INT
    semdTable[MAXPROC].s_semAdd = 0;
    semdTable[MAXPROC].s_next	= &semdTable[MAXPROC+1];
    semdTable[MAXPROC].s_tp	= mkEmptyProcQ();

    semdTable[MAXPROC+1].s_semAdd = (int*)MAX_INT;
    semdTable[MAXPROC+1].s_next	= NULL;
    semdTable[MAXPROC+1].s_tp	= mkEmptyProcQ();

    //place our dummy nodes into the active list
    semdActiveList_h = &semdTable[MAXPROC+1];
}

/*=========================================PROCESS BLOCK FUNCTIONS==============================================*\
                These functions modify or access the process queue of a semaphore descriptor.
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
BOOL insertBlocked(int* semAdd, pcb_PTR p)
{
   semd_PTR prnt = find(semAdd);
   semd_PTR child = prnt->s_next;

   if (child->s_semAdd == semAdd)
   {
   insertProcQ(&child->s_tp, p);
   }
   else //Allocate a new semaphore
   {
    semd_PTR semToAdd = allocSemd();
    if (semToAdd == NULL) return TRUE;

    //Insert the new semd into the ASL at the appopriate location
    semToAdd->s_next = child;
    prnt->s_next = semToAdd;
    semToAdd->s_semAdd = semAdd;
    insertProcQ(&(semToAdd->s_tp), p);
   }
   
   p->p_semAdd = semAdd;
   return FALSE;
}

/* 
Search the ASL for a descriptor of this semaphore. If none is
found, return NULL; otherwise, remove the first (i.e. head) ProcBlk
from the process queue of the found semaphore descriptor and re-
turn a pointer to it. If the process queue for this semaphore becomes
empty (emptyProcQ(s procq) is TRUE), remove the semaphore
descriptor from the ASL and return it to the semdFree list. 
*/
pcb_PTR removeBlocked(int* semAdd)
{
    pcb_PTR ret;
    semd_PTR prnt = find(semAdd);
    semd_PTR child = prnt->s_next;

    if (child->s_semAdd == semAdd)
    {
        //save the removed pcb to return
        ret = removeProcQ(&child->s_tp);

        //Was that the last pcb?
        if (emptyProcQ(child->s_tp))
        {
            //Remove the semd from the ASL and put it on the free list
            prnt->s_next = child->s_next;
            freeSemd(child);
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
   semd_PTR prnt  = find(p->p_semAdd);
   semd_PTR child = prnt->s_next;

   if (child->s_semAdd == p->p_semAdd)
   {
    //try to remove p and save a pointer to it
    ret = outProcQ(&child->s_tp, p);

    //Was p not there?
    if (ret == NULL) return NULL;

    //Is the process queue empty?
    if (emptyProcQ(child->s_tp))
    {
        //Remove the semd from the ASL and put it on the free list
        prnt->s_next   = child->s_next;
        freeSemd(child);
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
pcb_PTR headBlocked(int* semAdd)
{
    //Search the list for a semd holding the given semAdd
    semd_PTR temp = find(semAdd);

    //Return the head of the pcb at that semAdd
    if (temp->s_next->s_semAdd == semAdd)
    {
        return headProcQ(temp->s_next->s_tp);
    }

    //If we didn't find it...
    return NULL;
}

/*=========================================ACTIVE LIST FUNCTIONS================================================*\
                   These (this) function(s) access the semaphore directories on the active list
\*==============================================================================================================*/


/*
Searches the active semaphore list for a semaphore descriptor containing the given semAdd. 
If the semAdd is found, return the node above it
If the semAdd does not exist, return the node above where it would be.
*/
semd_PTR find(int* semAdd)
{
    semd_PTR temp = semdActiveList_h;
   //while the next value is less than or equal the target value, go to next node
   while(temp->s_next->s_semAdd < semAdd)
   {
       temp = temp->s_next;
   }
   return temp;
}


/*======================================FREE LIST MODIFIER FUNCTIONS============================================*\
                        These functions modify semaphore directories on the free list.
\*==============================================================================================================*/

/*
Push a node on the free list stack
*/
void freeSemd(semd_PTR s)
{  
    s->s_next = semdFreeList_h;
    semdFreeList_h = s;
}

/*
Pop a node off of the free list, null its values, and return it.
*/
semd_PTR allocSemd()
{
    semd_PTR temp = semdFreeList_h;

    if (temp == NULL) return NULL;

    semdFreeList_h = semdFreeList_h->s_next;

    temp->s_next   = NULL;
    temp->s_semAdd = NULL;
    temp->s_tp     = mkEmptyProcQ();

    return temp;
}