/*
Initializes our 20(MAXPROC) PCBs
*/
void initpcbs()
{
  static pcb_t[MAXPROC];
  static pcb_PTR freePcb = mkEmptyProcQ(); //not sure about this

  for(1=0; i<MAXPROC; i++)
  {
    insertProcQ(freePcb, &(foobar[i]))
  }
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
  return (tp == NULL)
}

/*
Insert the ProcBlk pointed to by p into the process queue whose
tail-pointer is pointed to by tp. Note the double indirection through
tp to allow for the possible updating of the tail pointer as well.
 */
void insertProcQ(pcb_PTR* tp, pcb_PTR p)
{
  //If the queue is empty, just set the tail pointer to the new node
  if(emptyProcQ(tp))
  {
    p->p_next = p;
    *tp = p;
  }
  else
  {
    p->p_next = tp->p_next;
    tp->p_next = p;
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
  if emptyProcQ(tp) return NULL

  //If there's only one node, don't worry about tp's p_next
  if(tp->p_next == tp)
  {
    pcb * temp;
    temp = tp;
    tp = NULL;
    return temp;
  }

  pcb * temp;
  temp = tp->p_next;
  tp->p_next = temp->p_next;
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

}

/* Make the ProcBlk pointed to by p a child of the ProcBlk pointed
to by prnt. */
void insertChild(pcb_PTR prnt, pcb_PTR p)
{

}

/* Make the first child of the ProcBlk pointed to by p no longer a
child of p. Return NULL if initially there were no children of p.
Otherwise, return a pointer to this removed first child ProcBlk. */
pcb_PTR removeChild(pcb_PTR p)
{

}

/* Make the ProcBlk pointed to by p no longer the child of its parent.
If the ProcBlk pointed to by p has no parent, return NULL; otherwise,
return p. Note that the element pointed to by p need not be the first
child of its parent. */
pcb_PTR outChild(pcb_PTR p)
{

}

