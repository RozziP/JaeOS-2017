#ifndef TYPES
#define TYPES

/**************************************************************************** 
 *
 * This header file contains utility types definitions.
 * 
 ****************************************************************************/

#include "/usr/include/uarm/uARMtypes.h"

typedef int bool; //Got this idea from Neal
typedef unsigned int cput_t;

/* process table entry type */
typedef struct pcb_t
{
    struct pcb_t
    /* process queue fields */
        *p_next,		// ptr to next entry 
        *p_prev;        // ptr to previous entry 
    struct pcb_t

    /* process tree fields */ 
        *p_prnt,        // ptr to parent  
        *p_child,       // ptr to 1st child 
        *p_prevSib,     // ptr to previous sibling		
        *p_nextSib;	    // ptr to next sibling


    /* process status information */
    state_t p_s;		//processor state
    int*	p_semAdd;   //ptr to semaphore on
    cput_t  p_time; 

    /* which proc is blocked */
    state_t* sysCallNew;
    state_t* sysCallOld;
    state_t* prgrmTrapNew;
    state_t* prgrmTrapOld;
    state_t* tlbNew;
    state_t* tlbOld;
    

}
pcb_t, *pcb_PTR;

typedef struct semd_t
{
    struct semd_t* s_next;
    int*    s_semAdd;
    pcb_PTR s_tp;
}
semd_t, *semd_PTR;

typedef struct {
	unsigned int d_status;
	unsigned int d_command;
	unsigned int d_data0;
	unsigned int d_data1;
} device_t;

#endif
