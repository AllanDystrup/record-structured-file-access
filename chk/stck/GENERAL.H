/*+1 =======================================================================*/
/*   MODULE                       GENERAL.H                                 */
/*==========================================================================*/
/*   FUNCTION      Headerfile with declarations of general interest.
 *
 *   SYSTEM        Standard (ANSI/ISO) C,
 *                 Tested on PC/MS DOS v.3.3 (MSC600A) & UN*X SYS V3 (GNU GCC)
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Kommunedata I/S, Aug. 1991
 *
 *   VERSION       $Header: d:/cwork/index/RCS/general.h 0.1 92/07/02 11:47:55
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 -----------------------------------------------------------
 *                 $Log:	general.h $
 *                 Revision 0.1  92/07/02  14:38:46  Allan_Dystrup
 *                 PreRelease (ALFA1)
 *
 *==========================================================================*/

#ifndef _GENERAL_H	       /* Makes sure general.h is included once */
#define _GENERAL_H	       /* Matching endif is at end of file */


/*==========================================================================*/
/*                     Setup for debugging [ON|OFF]                         */
/*==========================================================================*/
#ifdef   DEBUG               /* If DEBUG ---------------------------------- */
#define  PRIVATE             /*    ignore static class                      */
#define  D(x) x              /*    define debug trace                       */
#ifdef   __STDC__            /*    If ANSI -------------------------------- */
#include <assert.h>          /*       define assert                         */
#else                        /*    NOANSI                                   */
#define  assert(x)           /*       ignore assert                         */
#endif                       /*    END If ANSI                              */
#else                        /* NODEBUG ----------------------------------- */
#define  PRIVATE  static     /*    define static class                      */
#define  D(x)                /*    ignore debug trace                       */
#define  assert(x)           /*    ignore assertions                        */
#endif                       /* END If DEBUG ------------------------------ */

#define PUBLIC               /* Ignore */


/*==========================================================================*/
/*                     Setup for compiler type [ANSI|K&R]                   */
/*==========================================================================*/
#define _POSIX_SOURCE  1     /* Assure POSIX compatibility */

#ifdef __STDC__              /* If ANSI ----------------------------------- */
#define P(x)  x              /*    put arg lists into function prototypes.  */
#define VA_LIST  ...         /*    and use ellipsis if var. number of args  */
#define VOID  void           /*    for use as parameter                     */
#define CONST const          /*    use ANSI const keyword                   */
#define _far                 /*    Port to C11 std  (MS proprietary _far)   */
#else                        /* NON-ANSI ---------------------------------- */
#define P(x)  ()             /*    discard argument lists and translate     */
#define VA_LIST _a_r_g_s     /*    don't use ellipsis                       */
#define void  int            /*    for use in casts                         */
#define VOID                 /*    for use as parameter                     */
#define const                /*    ignore ANSI const keyword                */ 
#define volatile             /*    ignore ANSI volatile keyword             */
#endif	                     /* END If ANSI ------------------------------- */


/*==========================================================================*/
/*                     Range test macros                                    */
/*==========================================================================*/
#define NUMELE(a)          (sizeof(a)/sizeof(*(a)))
#define LASTELE(a)         ((a) + (NUMELE(a)-1))
#define TOOHIGH(a,p)       ((p) - (a) > (NUMELE(a) - 1))
#define TOOLOW(a,p)        ((p) - (a) <  0 )
#define INBOUNDS(a,p)      ( ! (TOOHIGH(a,p) || TOOLOW(a,p)) )
#define _IS(t,x)           (((t)1 << (x))!=0)
#define NBITS(t)           (4 * (1 + _IS(t, 4) + _IS(t, 8) + _IS(t,12) \
                                   + _IS(t,16) + _IS(t,20) + _IS(t,24) \
                                   + _IS(t,28) + _IS(t,32) ) )
#define MAXINT             (((unsigned)~0) >> 1)
#define RANGE(a,b,c)       ( (a) <= (b) && (b) <= (c) )
#define ISEVEN(n)          ( ((n) & 01) > 0 ? 0 : 1 )
/*
#define MAX(a,b)           ( ((a) > (b)) ? (a) : (b))
#define MIN(a,b)           ( ((a) < (b)) ? (a) : (b))
*/


/*==========================================================================*/
/*                     Derived types, cf. MS Windows & OS/2                 */
/*==========================================================================*/
typedef unsigned char  BYTE;
typedef unsigned short USHORT;
typedef unsigned int   WORD;
typedef int            FLAG;     /* int (not USHORT) when param. on UNIX! */
typedef long           LONG;
typedef unsigned long  DWORD;


/*==========================================================================*/
/*                     Some boolean values                                  */
/*==========================================================================*/
#ifndef TRUE                               /* Basic BOOLEAN values */
#   define TRUE  (FLAG) 1
#   define FALSE (FLAG) 0
#endif

typedef enum { ERROR = 0, OK } eRetType;   /* Function return value */
typedef enum { NO = 0, YES }   eAnsType;   /* Answer value [Y|N] */



#endif	/* #ifdef _GENERAL_H */
/* END of module general.h                                                  */
/*-1========================================================================*/
