# JaeOS-2017
### Uni-processor operating system under the ARM architecture
### Requires the uARM emulator, which can be found at https://github.com/mellotanica/uARM

Contains full support for exceptions, device interrupts, and syscalls 1-8.

Processes are represented as process control blocks(PCBs) stored in circular doubly-linked lists

Mutual exclusion is facilitated with the use of semaphores, which are represented as integers and stored in stacks.

### JaeOS manuals here -> https://github.com/RozziP/JaeOS-2017/tree/master/info

### SysCall 2 Video here -> https://www.youtube.com/watch?v=aB96hdTeYZY
