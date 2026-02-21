/*+1 =======================================================================*/
/*   MODULE                       HPCK.H                                    */
/*==========================================================================*/
/*   FUNCTION      Headerfile for module HPCK.C
 *                 #include this file to access the PUBLIC functions in hpck.c
 *
 *   SYSTEM        Standard (ANSI/ISO) C, tested on PC/MS DOS v.3.3
 *
 *   SEE ALSO      Module HPCK.C
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Kommunedata I/S, Aug. 1991
 *
 *   VERSION       $Header: d:/cwork/soul1/RCS/hpck.h 0.1 92/09/29 16:50:46
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 -----------------------------------------------------------
 *                 $Log:	hpck.h $
 *                 Revision 0.1  92/09/29  16:50:46  Allan_Dystrup
 *                 EXPERIMENTAL
 * 
 *-1========================================================================*/



/*+2------------------------------------------------------------------------*/
/* Make sure hpck.h is included only once                                   */
/*--------------------------------------------------------------------------*/
#ifndef _HPCK_H	            /* Matching endif is at end of file */
#   define _HPCK_H
#   define __STDC__


/*-------------------------------------------------------------------------*/
/* Include headers with general definitions                                */ 
/*-------------------------------------------------------------------------*/
#ifndef _GENERAL_H
#   include "../../../general.h"
#endif


/*--------------------------------------------------------------------------*/
/* Switch on heap check, if DEBUG flag set                                  */
/*--------------------------------------------------------------------------*/

PUBLIC char *vmalloc  P((size_t size));
PUBLIC void *vcalloc  P((size_t n, size_t size));
PUBLIC void vfree     P((char *buffer));
PUBLIC char *vrealloc P((char *buffer, size_t size));

PUBLIC void vverify   P((char *id));
PUBLIC void vdump     P((char *id));


/* NB don't define HDEBUG when compiling HPCK.C itself! */
#ifdef HDEBUG
#define malloc(a)      vmalloc(a)
#define free(a)        vfree(a)
#define realloc(a, b)  vrealloc(a, b)
#define calloc(a, b)   vcalloc(a, b)
#endif



#endif /* #ifdef _hpck_h */
/* End of module HPCK.H                                                     */
/*-2========================================================================*/
