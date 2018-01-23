# JaeOS-2017
Uni-processor operating system under the ARM architecture

Requires the uARM emulator, which can be found at https://github.com/mellotanica/uARM

Contains full support for exceptions, device interrupts, and syscalls 1-8.

Processes are represented as process controll blocks (PCBs) stored in circular doubly-linked list.

Mutual exclusion is facilitated with the use of semaphores, which are represented as integers and stored in a stack.
