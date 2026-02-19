/*+1 =======================================================================*/
/*   MODULE                       STCK.H                                    */
/*==========================================================================*/
/*   FUNCTION      Headerfile for module STCK.C
 *                 #include this file to access the PUBLIC functions in stck.c
 *
 *   SYSTEM        Standard (ANSI/ISO) C, tested on PC/MS DOS v.3.3
 *
 *   SEE ALSO      Module STCK.C
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Aug. 1991
 *
 *   VERSION       $Header: d:/cwork/soul1/RCS/stck.h 0.1 92/09/29 16:43:00
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 -----------------------------------------------------------
 *                 $Log:	stck.h $
 *                 Revision 0.1  92/09/29  16:43:00  Allan_Dystrup
 *                 EXPERIMENTAL
 *
 *-1========================================================================*/



/*+2------------------------------------------------------------------------*/
/* Make sure stck.h is included only once                                   */
/*--------------------------------------------------------------------------*/
#ifndef _STCK_H	            /* Matching endif is at end of file */
#   define _STCK_H             


/*-------------------------------------------------------------------------*/
/* Include headers with general definitions                                */ 
/*-------------------------------------------------------------------------*/
#ifndef _GENERAL_H
#   include "../../general.h"
#endif


/*--------------------------------------------------------------------------*/
/* Defines to handle declaration, allocation and initialization of global   */
/* data in a common header file.                                            */
/* The _STCK_INIT(x) macro won't work with aggregates because it interprets */
/* a comma as indicating a new macro parameter (not part of the current); - */
/* Aggregates are initialized inside a  #ifdef _STCK_ALLOC ... #endif       */
/*--------------------------------------------------------------------------*/
#ifdef _STCK_ALLOC
#   define _STCK_CLASS
#   define _STCK_INIT(x)   x
#else
#   define _STCK_CLASS     extern
#   define _STCK_INIT(x)
#endif

#define MAX_STACK_DEPTH    4000 /* Fails if > this # of frame pointers */


/*--------------------------------------------------------------------------*/
/* Switch on stack check, if SDEBUG flag set                                */
/*--------------------------------------------------------------------------*/
#ifdef SDEBUG
_STCK_CLASS PUBLIC int iCheckStack P((char *calling_id));
   # define STCK(x)     if (iCheckStack(x)) exit(1); else printf("%s ok\n", x)
   # define REAL_MAIN    real_main
#else
   # define STCK(x)
   # define REAL_MAIN	main
#endif


#endif /* #ifdef _stck_h */
/* End of module STCK.H                                                     */
/*-2========================================================================*/
