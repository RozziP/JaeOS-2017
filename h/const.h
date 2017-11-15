#ifndef CONSTS
#define CONSTS

/**************************************************************************** 
 *
 * This header file contains utility constants & macro definitions.
 * 
 ****************************************************************************/
#include "/usr/include/uarm/arch.h"
#include "/usr/include/uarm/uARMconst.h"


#define MAXPROC  		 20
#define BOOL             int //Got this idea from Neal
#define MAX_INT          0xFFFFFFFF
#define FAILURE			 -1
#define SUCCESS			 0



/*Number of device semaphores
8 for disk lines
8 for printer lines
8 for tape lines
16 for terminal lines
1 for the clock
8 dummy nodes to make access more simple*/
#define DEVICES          49 
#define QUANTUM          5000               


//Values for configuring control registers
#define RAMTOP           0x000002D4
#define ALLOFF           0x00000000
#define SYSMODE          0x0000001F
#define USRMODE          0x00000010
#define INTS_OFF         0x000000C0
#define INTS_ON          0x00000000

//Locations of new and old states
#define INT_OLD          0x00007000
#define INT_NEW          0x00007058

#define TLB_OLD          0x000070B0
#define TLB_NEW          0x00007108

#define PRGRM_OLD        0x00007160
#define PRGRM_NEW        0x000071B8

#define SYS_OLD          0x00007210
#define SYS_NEW          0x00007268

//sysCall values
#define BIRTH        	 1
#define DEATH            2
#define VERHOGEN   		 3
#define PASSEREN		 4
#define ESV         	 5 //GIVE THIS A BETTER NAME
#define CPUTIME 		 6
#define CLOCKWAIT 		 7
#define IOWAIT 			 8


//error cases
#define TLBTRAP          0
#define PRGRMTRAP        1
#define SYSTRAP		     2

#define NULLLINES        3
#define DISK			 3
#define TAPE    		 4
#define NETWORK    		 5
#define PRINTER 		 6
#define TERMINAL		 7

#define LINE1             0x00000001
#define LINE2             0x00000002
#define LINE3			 0x00000004
#define LINE4			 0x00000008
#define LINE5			 0x00000010
#define LINE6			 0x00000020
#define LINE7   	    	 0x00000040
#define LINE8   	    	 0x00000080

#define DEVICEFRONT        0x00000001   

#define INTMAP           0x00006FE0

#define DEVICEREGSTART   0X00000040
#define DEVICEREGSIZE    4
#define DEVICEPERLINE    8

#define TIMEOFDAYLOW    0x000002E0
#define TIMESCALE       0x000002E8

#define ACK             1



#endif
