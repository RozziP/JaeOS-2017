void initpcbs()
{
  static pcb_t[20];
  for(1=0; i<10; i++)
  {
    insertProcQ(freePcb, &(foobar[i]))
  }
}

void enqueue(pcb * tp, int x)
{
  //Create the new node
  pcb * newNode;
  newNode->data = x
  newNode->next = NULL
  //If the queue is empty, just set the tail pointer to the new node
  if(emptyProcQ(tp))
  {
    newNode->next = newNode;
    tp = newNode;
  }
  else
  {
    newNode->next = tp->next;
    tp->next = newNode;
    tp = newNode;
  }
}


pcb* dequeue(pcb * tp)
{
  //Return null if the list is empty
  if emptyProcQ(tp) return NULL
  //If there's only one node, don't worry about tp's next
  if(tp->next == tp)
  {
    pcb * temp;
    temp = tp;
    tp = NULL;
    return temp;
  }
  pcb * temp;
  temp = tp->next;
  tp->next = temp->next;
  temp->next = NULL;
  return temp;
}
