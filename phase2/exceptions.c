/*
exceptions.c

*/

/*

tlb()
{
   passUpOrDie()
}

prgrmTrap()
{
   passUpOrDie()
}

passUpOrDie()
{
    if sys5 hasn't been called for the error type, nuke
    otherwise, load the handler that was set by sys5
}

sysCallHandler()
{
    send the caller to the right syscall
}

sys1()
{
    create a new node
    add it to readyqueue
    incremenent processCount
}

sys2()
{
    delete a node
    if that node has children, call killAllChildren()
    decrement procCount
}

killAllChildren(pcb_PTR parent)
{
    recursively decimate the given node's bloodline
    decrement procCount for every child killed

}

sys3(mutex)
{
    increment mutex
    if mutex <= 0
        unblock a process from mutex
        put it on the readyQueue

    return control to the caller
}

wait
sys4(mutex)
{
    decrement mutex
    if mutex < 0
        block currentProc on mutex
        call the scheduler

    otherwise, return control to the caller
}

sys5()
{
    if sys5 has already been called for the given error type, kill the process
    otherwise, give the current process a new exception handler to be used by passUpOrDie
}

sys6(pcb)
{
    calculate the difference between the start time and the current time
    add that time the requesting PCB's time 
}

sys7()
{
    time things we dont know
}

sys8()
{
    find the device that sent the request
    perform a wait operation on the device's semaphore
    increment softBlockedCnt
}
*/