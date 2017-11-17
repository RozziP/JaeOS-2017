/*==========================================================================*\
  pcb.c
  Manages the active and free process queues and their child trees.

  The queues are doubly linked, circular list.
  The child trees are singly linked linear lists.

  Any process control block that is not in use is stored on the free lists with tail pointers.
  Any process control block with that is in use is stored on the active list.
  
  Authors: Peter Rozzi and Patrick Gemperline
  Date: 9-13-17
\*==========================================================================*/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"

HIDDEN pcb_PTR pcbFreeList_h;

/*
Initializes our 20(MAXPROC) PCBs
*/
void initPcbs()
{
  pcbFreeList_h = mkEmptyProcQ();

  static pcb_t pcbTable[MAXPROC];
  
  for(int i=0; i<MAXPROC; i++)
  {
    freePcb(&pcbTable[i]);
  }
}

/*====================================PROCESS QUEUE MODIFIER FUNCTIONS==========================================*\
                  These functions access and modify the elements of our process queues
\*==============================================================================================================*/

/* 
Insert the element pointed to by p onto the pcbFree list. 
*/
void freePcb(pcb_PTR p)
{
  insertProcQ(&pcbFreeList_h, p);
}

/* 
Return NULL if the pcbFree list is empty. Otherwise, remove
an element from the pcbFree list, provide initial values for ALL
of the ProcBlk’s fields (i.e. NULL and/or 0) and then return a
pointer to the removed element. ProcBlk’s get reused, so it is
important that no previous values persist in a ProcBlk when it
gets reallocated. 
*/
pcb_PTR allocPcb()
{
  if (emptyProcQ(pcbFreeList_h)) return NULL;

  pcb_PTR temp = removeProcQ(&pcbFreeList_h);

  temp->p_next     = NULL;
  temp->p_prev     = NULL;

  temp->p_child    = NULL;
  temp->p_prnt     = NULL;
  temp->p_nextSib  = NULL;
  temp->p_prevSib  = NULL;

  temp->p_semAdd   = NULL;
  //temp->p_s      = ;

  return temp;

}

/* 
This function is used to initialize a variable to be tail pointer to a
process queue.
Return a pointer to the tail of an empty process queue; i.e. NULL. 
*/
pcb_PTR mkEmptyProcQ()
{
  return NULL;
}

/* 
Return TRUE if the queue whose tail is pointed to by tp is empty.
Return FALSE otherwise. 
*/
BOOL emptyProcQ(pcb_PTR tp)
{
  return (tp == NULL);
}

/*
Insert the ProcBlk pointed to by p into the process queue whose
tail-pointer is pointed to by tp. Note the double indirection through
tp to allow for the possible updating of the tail pointer as well.
 */
void insertProcQ(pcb_PTR* tp, pcb_PTR p)
{
  //If the queue is empty, just set the tail pointer to the new node
  if(emptyProcQ(*tp))
  {
    p->p_next = p;
    p->p_prev = p;
    *tp = p;
  }
  else
  {
    p->p_next = (*tp)->p_next;
    p->p_prev = *tp;
    (*tp)->p_next->p_prev = p;
    (*tp)->p_next = p;
    *tp = p;
  }
}

/*
Remove the first (i.e. head) element from the process queue whose
tail-pointer is pointed to by tp. Return NULL if the process queue
was initially empty; otherwise return the pointer to the removed element.
Update the process queue’s tail pointer if necessary. 
*/
pcb_PTR removeProcQ(pcb_PTR* tp)
{
  //Return null if the list is empty
  if (emptyProcQ(*tp)) return NULL;

  //If there's only one node, don't worry about tp's next
  if((*tp)->p_next == *tp)
  {
    pcb_PTR temp = *tp;
    *tp = NULL;   
    return  temp;
  }

  //save the head to return
  pcb_PTR temp = (*tp)->p_next;

  //restore the chain
  (*tp)->p_next->p_next->p_prev = *tp;    //head's next's previous points to tp
  (*tp)->p_next = (*tp)->p_next->p_next;  //tp's next points to head's next

  temp->p_next  = NULL;
  temp->p_prev  = NULL;
  
  return temp;
}

/* 
Remove the ProcBlk pointed to by p from the process queue whose
tail-pointer is pointed to by tp. Update the process queue’s tail
pointer if necessary. If the desired entry is not in the indicated queue
(an error condition), return NULL; otherwise, return p. Note that p
can point to any element of the process queue. 
*/

pcb_PTR outProcQ(pcb_PTR* tp, pcb_PTR p)
{
  //Anyone home? No?
 if (emptyProcQ(*tp)) return NULL;

 pcb_PTR temp = *tp;

 //Is tp what we're looking for?
 if (*tp == p)
 {
   //is tp the only element?
  if ((*tp)->p_next == *tp)
  {
    *tp = NULL;
  }
  //tp is not the only element and we need to maintain the pointer
  else
  {
    /*
    temp and tp point to the same node at this point, but
    I think it's clearer to use temp when we're changing fields,
    and *tp when we're changing the pointer
    */
    temp->p_prev->p_next = temp->p_next;
    temp->p_next->p_prev = temp->p_prev;
    //set the new tail pointer
    *tp = temp->p_prev;
  }
  return temp;
 }
 //Looping through the process queue
 while(temp->p_next != *tp)
 {
   //Looking for p
  if (temp->p_next == p)
  {
    temp = temp->p_next;
    //Restore the chain
    temp->p_prev->p_next = temp->p_next;
    temp->p_next->p_prev = temp->p_prev;

    temp->p_next = NULL;
    temp->p_prev = NULL;

    return temp;
  }
  temp = temp->p_next;
 }
 return NULL;
}

/*
Return a pointer to the first ProcBlk from the process queue whose
tail is pointed to by tp. Do not remove this ProcBlkfrom the process
queue. Return NULL if the process queue is empty. 
*/
pcb_PTR headProcQ(pcb_PTR tp)
{
 return tp->p_next;
}

/*==========================================CHILD TREE FUNCTIONS================================================*\
                    These functions access and modify the pcb elements of the child tree
\*==============================================================================================================*/

/* 
Return TRUE if the ProcBlk pointed to by p has no children. Return
FALSE otherwise.
 */
BOOL emptyChild(pcb_PTR p)
{
  return (p->p_child == NULL);
}

/* Make the ProcBlk pointed to by p a child of the ProcBlk pointed
to by prnt. */
void insertChild(pcb_PTR prnt, pcb_PTR p)
{
  //Check if prnt already has a child, then do... something
  if (!emptyChild(prnt))
  {
    p->p_nextSib = prnt->p_child;
    p->p_nextSib->p_prevSib=p;
    prnt->p_child = p;
    p->p_prnt = prnt;
  }
  else
  {
    prnt->p_child = p;
    p->p_prnt = prnt;
  }
}

/*
Recursively removes all descendants of the given parent node p
*****NOT CURRENTLY USED, MAYBE NEVER USED****
*/
void killAllChildren(pcb_PTR p)
{
  if (emptyChild(p)) return;

  while (!(emptyChild(p->p_child)))
  {
    pcb_PTR temp = p->p_child;
    killAllChildren(p->p_child);
    p->p_child = p->p_child->p_nextSib;
    p->p_child->p_prevSib = NULL;

    temp->p_prnt = NULL;
    temp->p_nextSib  = NULL;
    freePcb(temp);
  }  
}

/* Make the first child of the ProcBlk pointed to by p no longer a
child of p. Return NULL if initially there were no children of p.
Otherwise, return a pointer to this removed first child ProcBlk. */
pcb_PTR removeChild(pcb_PTR p)
{
  if (emptyChild(p)) return NULL;

  pcb_PTR temp   = p->p_child;
  p->p_child     = temp->p_nextSib;
  temp->p_prevSib = NULL;
  temp->p_prnt     = NULL;
  
  //killAllChildren(temp); no don't

  return temp;
}


/* Make the ProcBlk pointed to by p no longer the child of its parent.
If the ProcBlk pointed to by p has no parent, return NULL; otherwise,
return p. Note that the element pointed to by p need not be the first
child of its parent. */
pcb_PTR outChild(pcb_PTR p)
{
  if (p->p_prnt == NULL) return NULL;


  if(p->p_prevSib) p->p_prevSib->p_nextSib = p->p_nextSib;
  if(p->p_nextSib)    p->p_nextSib->p_prevSib = p->p_prevSib;

  if (p->p_prevSib == NULL) p->p_prnt->p_child = p->p_nextSib;

  //killAllChildren(p); no don't
  p->p_nextSib    = NULL;
  p->p_prevSib = NULL;
  p->p_prnt   = NULL;
  return p;
}

