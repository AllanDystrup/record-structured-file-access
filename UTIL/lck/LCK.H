/*+1========================================================================*/
/*  MODULE                        LCK.H                                     */
/*==========================================================================*/
/*  FUNCTION       Headerfile for module LCK.C
 *                 #include this file to access the PUBLIC functions for byte-
 *                 level file locking and semaphore operations defined in LCK.C
 *
 *  SYSTEM         !NB: NOT Standard C, - uses compiler specific routines NB!
 *                 Portable across UN*X (POSIX) and MSDOS (MS C compiler) :
 *                 - UNIX SVR3&4 (DG CC using standard POSIX system interface)
 *                 - PC/MSDOS V33&50 (MSC60A using MS specific ANSI C extensions)
 *
 *  SEE ALSO       Modules: LCK.C, MAKEFILE
 *
 *  PROGRAMMER     Allan Dystrup, KMD:DIVDEC/AD
 *
 *  COPYRIGHT      (c) Allan Dystrup, KommuneData I/S September 1992.
 *
 *  VERSION        $Header: d:/cwork/lck/RCS/lck.h 0.1 92/11/11 09:17:35
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 -----------------------------------------------------------
 *                 $Log:	lck.h $
 *                 Revision 0.1  92/11/11  09:17:35  Allan_Dystrup
 *                 PREREL (ALFA1)
 * 
 *-1========================================================================*/


/****************************************************************************/
/**************************** HEADER FILES **********************************/
/****************************************************************************/
#ifndef LCK_H              /* Make sure this header is included only once   */
#define LCK_H              /* Matching endif is at End-Of-File */

/* Include header with general definitions */
#ifndef _GENERAL_H
#include "../../general.h"
#endif

/*--------------------------------------------------------------------------*/
/*                           ALLOC & INIT                                   */
/* Defines to handle declaration, allocation and initialization of global   */
/* data in a common header file.                                            */
/* The _LCK_INIT(x) macro won't work with aggregates 'cause it interprets   */
/* a comma as indicating a new macro parameter (not part of the current); - */
/* Aggregates are initialized inside a  #ifdef _INDEX_ALLOC ... #endif      */

/* Allocate space for globals */
#ifdef _LCK_ALLOC                  /* LCK.C: Allocate space for globals --- */
#   define _LCK_CLASS              /*    ignore                             */
#   define _LCK_INIT(x)  x         /*    initialize                         */
#else                              /* EXTERN: no allocation in user modules */
#   define _LCK_CLASS    extern    /*    declare as extern                  */
#   define _LCK_INIT(x)            /*    don't initialize                   */  
#endif                             /* END _LCK_ALLOC ---------------------- */

/* Stamp/Tag ID in object module; - Grep for "TAG" to retrieve ID-stamp */
_LCK_CLASS const char INDEX_ID[]
_LCK_INIT(= "\nTAG: MOD[lck.h] VER[0.1.0 Pre] DATE[92/09/02] (C)[Allan Dystrup]");
/*-2*/ 


#ifdef UNIX
/****************************************************************************/
/********************** POSIX-X/Open XSI C HEADER ***************************/
/**********************    LowLevel File I/O      ***************************/
/****************************************************************************/


/* Chose a target machine, add definition for new machines as appropriate. */
/* This mumbo-jumbo is nessecary to allow inclusion of POSIX headers such  */
/* as <fcntl.h> by modules using a strict ANSI compilation (switch -ansi). */
#ifdef __STDC__                    /* Active if compiling under ANSI C     */
                                   /* ------------------------------------ */
#define DG   1                     /* Data General AViiON                  */
#undef  SCO                        /* Santa Cruz Operation i386/486 PC     */

#ifdef  DG                         /* DG: Data General UNIX SVR4           */
#undef  ___int_features_h          /* "Touch" inclusion switch to redefine */
#endif  /*DG*/                     /* visibility of POSIX-features.        */

#ifdef  SCO                        /* SCO UNIX                             */
/* reset namespace */              /* Do whatever is nessecary to allow    */
#endif  /*SCO*/                    /* inclusion of POSIX-headers in ANSI.  */

#endif /*__STDC__*/


/* Standard POSIX include files for LowLevel file I/O */

#define _POSIX_SOURCE  1           /* Hide vendor specific extensions       */
                                   /*---------------------------------------*/
#include <stdio.h>                 /* ANSI  file defs.: SEEK_xxx  etc       */
#include <stdarg.h>                /* ANSI  macro for var.args.: va_list    */
#include <stdlib.h>                /* ANSI  misc func: exit etc             */
#include <ctype.h>

#include <errno.h>                 /* POSIX (&ANSI) error codes             */
#include <unistd.h>                /* POSIX misc. macro/func: SEEK_XXX etc  */
#include <sys/types.h>             /* POSIX fundamental types: pid_t etc    */
#include <fcntl.h>                 /* POSIX lowlevel file I/O: open etc     */



/*--------------------------------------------------------------------------*/
/*                           POPEN & PFCNTL                                 */
/* Point Portable LowLevel API directly to POSIX functions open() & fcntl() */
/*--------------------------------------------------------------------------*/
                                        /* Map generic acc.mode to POSIX    */
                                        /*----------------------------------*/
#define OFLAG       O_RDWR              /* Open mode: reading & writing     */

                                        /* Map generic acc.func. to POSIX   */
                                        /*----------------------------------*/
#define POPEN(path) open(path, OFLAG)   /* Map directly to POSIX equivalent */
#define PFCNTL      fcntl               /* Map directly to POSIX equivalent */

#define iChkEnv()   0                   /* Ignore DOS multiuser check       */

#endif  /* #ifdef UNIX */
/*-2*/ 



#ifdef MSDOS
/****************************************************************************/
/**********************  MICROSOFT C 6.0 HEADER   ***************************/
/**********************    LowLevel File I/O      ***************************/
/****************************************************************************/

/* Specific MSDOS include files for LowLevel file I/O */
#include <stdio.h>                 /* ANSI file definitions: SEEK_xxx  etc  */
#include <stdarg.h>                /* ANSI macro for var.args.: va_list etc */
#include <stdlib.h>                /* ANSI misc func: exit etc              */
#include <errno.h>                 /* ANSI error codes                      */

#include <sys/types.h>             /* MSC  some system level types: off_t   */
#include <fcntl.h>                 /* MSC  file ctl.options used by open()  */
#include <sys/stat.h>              /* MSC  file permissions: S_IREAD etc    */
#include <share.h>                 /* MSC  file sh.modes for sopen():SH_xxx */
#include <sys/locking.h>           /* MSC  flags for lck.func.: LK_RLCK etc */
#include <io.h>                    /* MSC  LowLevel fileI/O: sopen(),read() */
#include <dos.h>                   /* MSC  def's for MS-DOS OS: int86()     */


/*--------------------------------------------------------------------------*/
/*                           POPEN & PFCNTL                                 */
/* Point Portable LowLevel API to DOS-specific functions DOPEN() & DFCNTL() */
/*--------------------------------------------------------------------------*/
                                        /* Map generic acc.mode to DOS MSC  */
                                        /*----------------------------------*/
#define OACC        O_RDWR | O_TEXT     /* Access(OWN) : Read/Wrt (A=010)   */
#define OSHR        SH_DENYNO           /* Share(OTHER): Deny none (S=100)  */
#define OPERM       S_IREAD | S_IWRITE  /* Permissions : All (creat)        */
#define OFLAG       OACC,OSHR,OPERM     /* Combine above flags              */

                                        /* Map generic acc.func. to DOS MSC */
                                        /*----------------------------------*/
#define POPEN(file) DOPEN(file)         /* Map to API for DOS shared open   */
#define PFCNTL      DFCNTL              /* Map to POSIX API for MSDOS-C60   */
_LCK_CLASS int DOPEN   P((char *pzFile));
_LCK_CLASS int DFCNTL  P((int iFd, int iFcntlCmd, void *pstFlock));


/*--------------------------------------------------------------------------*/
/*                 Map MSC60 -> X/Open=POSIX fcntl interface                */
/*--------------------------------------------------------------------------*/
/* 1: Lock persistence - NonBlocking/Blocking */
#define F_SETLK    0                    /* Set a Non-blocking lock */
#define F_SETLKW   1                    /* Set a Blocking lock */
/* 2: Lock access - Read/Write/Unlock */
#define F_RDLCK    0                    /* Set a Read/Shared lock */
#define F_WRLCK    1                    /* Set a Write/Exclusive lock */
#define F_UNLCK    2                    /* Un-lock any type of lock */

_LCK_CLASS int F_MAP[2][3]
#ifdef _LCK_ALLOC 
= {
    /* MSC 6.0       ->           X/Open=POSIX       :: GENERIC */
    {  LK_NBLCK,               /* [F_SETLK,  F_RDLCK] = NREAD   */
       LK_NBRLCK,              /* [F_SETLK,  F_WRLCK] = NWRITE  */
       LK_UNLCK,               /* [F_SETLK,  F_UNLCK] = UNLOCK  */
    },
    {  LK_LOCK,                /* [F_SETLKW, F_RDLCK] = BREAD   */
       LK_RLCK,                /* [F_SETLKW, F_WRLCK] = BWRITE  */
       LK_UNLCK                /*      !catch undefined!        */
    }
  }
#endif /*_LCK_ALLOC*/
;

#ifdef DEBUG
_LCK_CLASS char *F_OP[5]
#ifdef _LCK_ALLOC 
= {     /* Flags for locking() function, MSC60 locking.h */
    "0:[LK_UNLCK]",            /* [F_SETLK,  F_UNLCK] = UNLOCK */
    "1:[LK_LOCK]",             /* [F_SETLKW, F_RDLCK] = BREAD  */
    "2:[LK_NBLCK]",            /* [F_SETLK,  F_RDLCK] = NREAD  */
    "3:[LK_RLCK]",             /* [F_SETLKW, F_WRLCK] = BWRITE */
    "4:[LK_NBRLCK]"            /* [F_SETLK,  F_WRLCK] = NWRITE */
  }
#endif /*_LCK_ALLOC*/
;
#endif  /* DEBUG */


/*--------------------------------------------------------------------------*/
/*                      Define the fcntl locking structure                  */
/*--------------------------------------------------------------------------*/
typedef unsigned long pid_t;

struct flock {
    short     l_type;          /* F_RDLCK, F_WRLCK, F_UNLCK */
    short     l_whence;        /* SEEK_SET, SEEK_CUR, SEEK_END */
    off_t     l_start;         /* Start offset for lock */
    off_t     l_len;           /* Length (#byte) for lock */
    pid_t     l_pid;           /* Process-id for owner of lock */
    /* short  l_sysid;         //DGUX specific, not POSIX */
    /* short  l_pad1;          //DGUX specific, not POSIX */
};

#define EDEADLK  EDEADLOCK     /* POSIX -> MSC60 */
#define ENOLCK   1000          /* POSIX, not avail in MSC60 */

#endif  /* #ifdef MSDOS */
/*-2*/ 



/****************************************************************************/
/********************** POSIX-X/Open & MSC60 HEADER *************************/
/**********************    HighLevel File I/O      **************************/
/****************************************************************************/


/*--------------------------------------------------------------------------*/
/*                           iPLock()                                       */
/*      Map our High-Level API -> Portable LowLevel Interface: PFCNTL       */
/*--------------------------------------------------------------------------*/
#define NREAD  0               /* Non-blocking read/shared lock */
#define NWRITE 1               /* Non-blocking write/exclusive lock */
#define UNLOCK 2               /* Unlock */
#define BREAD  3               /* Blocking read/shared lock */
#define BWRITE 4               /* Blocking write/exclusive lock */

typedef struct flock stFlock_t;    /* Define a type for POSIX std. struct */

_LCK_CLASS int iPLock  P((int iFd, long lStart, long lLen, int iType));


_LCK_CLASS int L_MAP[5][2]
#ifdef _LCK_ALLOC 
= {
    /* X/Open=POSIX   ->          MSC 6.0  ::  GENERIC */
    {F_SETLK,  F_RDLCK},       /* LK_NBLCK     NREAD   */
    {F_SETLK,  F_WRLCK},       /* LK_NBRLCK    NWRITE  */
    {F_SETLK,  F_UNLCK},       /* LK_UNLCK     UNLOCK  */
    {F_SETLKW, F_RDLCK},       /* LK_LOCK      BREAD   */
    {F_SETLKW, F_WRLCK}        /* LK_RLCK      BWRITE  */
/*  {F_SETLKW, F_UNLCK}                !undefined!     */
  }
#endif /*_LCK_ALLOC*/
;

#ifdef DEBUG
_LCK_CLASS char *L_OP[5]
#ifdef _LCK_ALLOC 
= {
    "0:NREAD=(F_SETLK,F_RDLCK)=(LK_NBLCK)",
    "1:NWRITE=(F_SETLK,F_WRLCK)=(LK_NBRLCK)",
    "2:UNLOCK=(F_SETLK,F_UNLCK)=(LK_UNLCK)",
    "3:BREAD=(F_SETLKW,F_RDLCK)=(LK_LOCK)",
    "4:BWRITE=(F_SETLKW,F_WRLCK)=(LK_RLCK)"
  }
#endif /*_LCK_ALLOC*/
;
#endif  /* DEBUG */


/*--------------------------------------------------------------------------*/
/*                                iPSem()                                   */
/*       Map our Very High-Level API -> HighLevel API: iPLock               */
/*--------------------------------------------------------------------------*/
#define   SEMUP        '!'         /* SEMaphore UP operation and state      */
#define   SEMDOWN      '.'         /* SEMaphore DOWN operation and state    */
#define   SEMTEST      'T'         /* SEMaphore TEST operation (no state)   */

_LCK_CLASS int iPSem   P((int iFd, long lStart, int iOp));



#endif  /* #ifndef LCK_H */
/* End module LCK.H                                                         */
/*-2========================================================================*/
