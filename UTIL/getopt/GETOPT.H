/*+1========================================================================*/
/*   MODULE                       GETOPT.H                                  */
/*==========================================================================*/
/*   FUNCTION      Header file for module GETOPT.C (and user modules);
 *                 #include this file for access to the PUBLIC function
 *                 for command-line option-parsing defined in getopt.c.
 *
 *   SYSTEM        Standard (ANSI/ISO) C,
 *                 Tested on PC/MS DOS v3.3 (MSC600A) & UNIX SVR3, SVR4 (CC)
 *
 *   SEE ALSO      Modules : GENERAL.H, GETOPT.C
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Feb. 1992
 *
 *   VERSION       $Header: d:/cwork/index/RCS/getopt.h 0.1 92/08/18 16:00:27
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 -----------------------------------------------------------
 *                 $Log:	getopt.h $
 *                 Revision 0.1  92/08/18  16:00:27  Allan_Dystrup
 *                 PREREL (ALFA1)
 * 
 *-1========================================================================*/



/****************************************************************************/
/**************************** HEADER FILES **********************************/
/****************************************************************************/
#ifndef _GETOPT_H              /* Make sure this header is included only once */
#define _GETOPT_H              /* Matching #endif is at end-of-file */


/*--------------------------- GENERAL DEF's --------------------------------*/
#ifndef _GENERAL_H
#include "../../general.h"
#endif


/*--------------------------- ALLOC & INIT ---------------------------------*/
/* Defines to handle declaration, allocation and initialization of global   */
/* data in a common header file.                                            */
/* The _GETOPT_INIT(x) macro won't work with aggregates : it interprets a   */
/* comma as indicating a new macro parameter (not part of the current); -   */
/* Aggregates are initialized inside a  #ifdef _VA_ALLOC ... #endif         */
#ifdef _GETOPT_ALLOC               /* GETOPT.H: Allocate space for globals--*/
#   define _GETOPT_CLASS           /*    ignore                             */
#   define _GETOPT_INIT(x) x       /*    initialize                         */
#else                              /* EXTERN: no allocation in user modules */
#   define _GETOPT_CLASS   extern  /*    declare as extern                  */
#   define _GETOPT_INIT(x)         /*    don't initialize                   */
#endif                             /* END _GETOPT_ALLOC --------------------*/



/****************************************************************************/
/**************************** DATA STRUCTURES *******************************/
/****************************************************************************/

/* Non-Option - returned by GETOPT when it encounters a non-option argument */
#define  NONOPT  (-1)


/* Set by GETOPT for options expecting arguments */
_GETOPT_CLASS PUBLIC char* optarg
_GETOPT_INIT(= NULL)
;


/* Set by GETOPT: index of next ARGV to be processed */
_GETOPT_CLASS PUBLIC int optind
_GETOPT_INIT(= 0)
;


/* Disable (== 0) or enable (!= 0) error messages written to standard error */
_GETOPT_CLASS PUBLIC int opterr
_GETOPT_INIT(= -1)
;



/****************************************************************************/
/**************************** FUNCTION PROTOTYPES ***************************/
/****************************************************************************/

/* Function to get command line options */
_GETOPT_CLASS PUBLIC int
       getopt  P((int argc, char **argv, char *optstring));



#endif  /* #ifndef _GETOPT_H */
/* End of module GETOPT.H                                                   */
/*-1========================================================================*/
