#ifndef TYPES
#define TYPES

/**************************************************************************** 
 *
 * This header file contains utility types definitions.
 * 
 ****************************************************************************/

#include "/usr/include/uarm/uARMtypes.h"

/* process table entry type */
typedef struct pcb_t
{
    struct pcb_t
    /* process queue fields */
        *p_next,		// ptr to next entry 
        *p_prev;       // ptr to previous entry 
    struct pcb_t
    /* process tree fields */ 
        *p_prnt,        // ptr to parent  
        *p_child,       // ptr to 1st child 
        *p_prevSib,      // ptr to previous sibling		
        *p_nextSib;	        // ptr to next sibling

    /* process status information */
    state_t p_s;		//processor state
    int*	p_semAdd;   //ptr to semaphore on

    /* which proc is blocked */
}
pcb_t, *pcb_PTR;

typedef struct semd_t
{
    struct semd_t* s_next;
    int*    s_semAdd;
    pcb_PTR s_tp;
}
semd_t, *semd_PTR;

#endif
