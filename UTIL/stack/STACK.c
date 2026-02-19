/*+1========================================================================*/ 
/*  MODULE                     STACK.C                                      */ 
/*==========================================================================*/ 
/*  FUNCTION      Testdriver for stack macro's in header-file STACK.H	    */
/*									    */
/*  REVISION	  Revision 1.2 2025/12/02 11:00:00	 Allan_Dystrup	    */
/*		  Port to UBUNTU Linux on Win.10/WSL, Using CLion	    */
/*-1========================================================================*/  
#include <stdio.h> 
#include "./stack.h"     


#ifdef MAIN
/*+3 MODULE STACK.C --------------------------------------------------------*/ 
/*   NAME   00                     main()                                   */ 
/*-- SYNOPSIS --------------------------------------------------------------*/ 
void main() { 
/*   DESCRIPTION  
 *   Simple testdriver for header file ERROR.H, with a full set of macro defs.  
 *   for stack operations.  
 *-3*/           
	int       i;     

	stack_dcl(postfix, int, 50);  
   
	stack_clear(postfix);  
   
	for (i = 0; i < 50; i++)     
		push(postfix, i);     
		

	for (i = 0; i < 51; i++)     
		printf("stack[%d]=%d\n", i, pop(postfix));
}   


/*+3 MODULE STACK.C --------------------------------------------------------*/ 
/*   NAME   01                     vError()                                 */ 
/*-- SYNOPSIS --------------------------------------------------------------*/ 
void 
vError(type)    
	int type; 
{ 
/*   DESCRIPTION  
 *   Each user module must define an appropriate error handling function  
 *   called 'vError' to catch stack over/under-flow exceptions  
 *-3*/         

	printf("Stack error %d\n", type);
}
#endif

/* END module STACK.C                                                       */
/*==========================================================================*/
