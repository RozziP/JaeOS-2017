/*
Currently have some concerns about initPcbs, FreePcb, allocPcb, inserProcQ, and removeProcQ.
Mainly with getting pointers to pointers to interact and compile without error. 

initPcbs and freePcb call insterProcQ with the address of pcbFree (&pcbFree)
allocPcb does the same with removeProcQ
insert and remove require the dereferencing of every ues of tp, which seems like the wrong thing to do.
*/

#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"

HIDDEN pcb_PTR pcbFree_h;

/*
Initializes our 20(MAXPROC) PCBs
*/
void initPcbs()
{
  static pcb_PTR initNodes[MAXPROC];
  
  for(int i=0; i<MAXPROC; i++)
  {
    insertProcQ(&pcbFree_h, (initNodes[i]));
  }
}

/* 
Insert the element pointed to by p onto the pcbFree list. 
*/
void freePcb(pcb_PTR p)
{
  insertProcQ(&pcbFree_h, p);
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
  pcb_PTR temp = removeProcQ(&pcbFree_h);

  temp->p_next  = NULL;
  temp->p_prev  = NULL;
  temp->p_child = NULL;
  temp->p_prnt  = NULL;
  temp->p_sib   = NULL;
  return temp;

}

/* 
This method is used to initialize a variable to be tail pointer to a
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
bool emptyProcQ(pcb_PTR tp)
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

  //If there's only one node, don't worry about tp's p_next
  if((*tp)->p_next == *tp)
  {
    pcb_PTR temp;
    temp  = *tp;
    tp    = NULL;
    return  temp;
  }
  //If there are only two nodes, we need to set tp's previous to itself after removal
  if((*tp)->p_next == (*tp)->p_prev)
  {
    pcb_PTR temp;
    temp  = (*tp)->p_next;
    (*tp)->p_next = *tp;
    (*tp)->p_prev = *tp;
    temp->p_next  = NULL;
    return temp;
  }

  pcb_PTR temp;
  temp = (*tp)->p_next;
  (*tp)->p_next = temp->p_next;
  temp->p_next = NULL;
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
 //TODO
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

/* 
Return TRUE if the ProcBlk pointed to by p has no children. Return
FALSE otherwise.
 */
bool emptyChild(pcb_PTR p)
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
    //TODO
    //I guess we have to make a sibling?
  }
  else
  {
    prnt->p_child = p;
  }
}

/* Make the first child of the ProcBlk pointed to by p no longer a
child of p. Return NULL if initially there were no children of p.
Otherwise, return a pointer to this removed first child ProcBlk. */
pcb_PTR removeChild(pcb_PTR p)
{
  if (p->p_child == NULL) return NULL;
  p->p_child == NULL;
}

/* Make the ProcBlk pointed to by p no longer the child of its parent.
If the ProcBlk pointed to by p has no parent, return NULL; otherwise,
return p. Note that the element pointed to by p need not be the first
child of its parent. */
pcb_PTR outChild(pcb_PTR p)
{
  //TODO
}

