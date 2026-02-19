/*+1========================================================================*/
/*   MODULE                       VA.H                                      */
/*==========================================================================*/
/*   FUNCTION      Header file for module VA.C (and user modules);
 *                 #include this file for access to PUBLIC virtual array (VA)
 *                 functions defined in VA.C.
 *
 *   SYSTEM        Standard (ANSI/ISO) C,
 *                 Tested on PC/MS DOS 3.3 (MSC 600A) & UNIX SYS V3 (GNU GCC)
 *
 *   SEE ALSO      Modules : GENERAL.H, ACCESS.H, VA.C
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Kommunedata I/S, Feb. 1992
 *
 *   VERSION       $Header: d:/cwork/index/RCS/va.h 0.1 92/07/06 09:41:24
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 ----------------------------------------------------------
 *                 $Log:	va.h $
 *                 Revision 0.1  92/07/06  09:41:24  Allan_Dystrup
 *                 PREREL (ALFA1)
 *
 *-1========================================================================*/


/****************************************************************************/
/**************************** HEADER FILES **********************************/
/****************************************************************************/
#ifndef _VA_H        /* Make sure this header is included only once */
#define _VA_H        /* Matching #endif is at end-of-file */


/* Include header with general definitions */
#ifndef _GENERAL_H
#include "general.h"
#endif

/*--------------------------------------------------------------------------*/
/* Defines to handle declaration, allocation and initialization of global   */
/* data in a common header file.                                            */
/* The _VA_INIT(x) macro won't work with aggregates because it interprets   */
/* a comma as indicating a new macro parameter (not part of the current); - */
/* Aggregates are initialized inside a  #ifdef _VA_ALLOC ... #endif         */
/*--------------------------------------------------------------------------*/
#ifdef _VA_ALLOC                   /* VA.C: Allocate space for globals ---- */
#   define _VA_CLASS               /*    ignore                             */
#   define _VA_INIT(x)     x       /*    initialize                         */
#   define _ACCESS_CLASS           /*    ignore                             */
#   define _ACCESS_ALLOC           /*    ignore                             */
#else                              /* EXTERN: no allocation in user modules */
#   define _VA_CLASS       extern  /*    declare as extern                  */
#   define _VA_INIT(x)             /*    don't initialize                   */
#   define _ACCESS_CLASS   extern  /*    declare as extern                  */
#   undef  _ACCESS_ALLOC           /*    don't allocate                     */
#endif                             /* END _VA_ALLOC ----------------------- */

/* Include header with error handling & generic API for access modules */ 
#ifndef _ACCESS_H
#include "access.h"
#endif

/* Stamp/Tag ID in object module; - Grep for "TAG" to retrieve ID-stamp */
_VA_CLASS const char VA_ID[]
_VA_INIT(="\nTAG: MOD[va.h] VER[0.1.0 Pre] DATE[92/07/10] (C)[Allan Dystrup KMD I/S]");



/****************************************************************************/
/**************************** DATA STRUCTURES *******************************/
/****************************************************************************/

/*==========================================================================*/
/*                      Basic VA-info (disk & incore)                       */
/*==========================================================================*/
/* This structure contains the basic VA file size informations : */
struct stVAsize {
    DWORD     dwArSize;       /* #array-elements total in VA file */
    DWORD     dwArUsed;       /* #array-elements used in VA file */
    WORD      wArElSize;      /* #bytes in each VA file element ( = record) */
    char      cFill;	      /* fill character for empty VA elements */
};

typedef struct stVAsize SIZEINFO;


/*==========================================================================*/
/*                      VA descriptor - incore                              */
/*==========================================================================*/
/* This structure defines a VA-Control-Block (VACB) in RAM/"incore" : */
/* - It is used as the common "object" datastructure by the VA-functions */
/* - It is normally accessed by its address (VACB*) or a handle (VACB **) */
struct stVACore {
    /* VA FILE ------------------------------------------------------------ */
    enum  {  RW=0,            /* VA-file is ReadWrite */        
             RO=1             /* VA-file is ReadOnly */
          }  indexmode;       /* VA-file open status */
    FILE     *pfArFile;       /* Handle for VA file (OS stream cntlstruct) */
    SIZEINFO  stSize;         /* VA file size information */

    /* VA CACHE BUFFER ---------------------------------------------------- */
    WORD      wBfSize;       /* #array elements in VA buffer */
    WORD      wBfElSize;     /* #bytes in each VA buf element (inc index) */
    char     *pcBf;          /* Ptr to VA buffer (cache) */
    char     *pcArElInit;    /* Ptr to "blank" rec for initializing VA elem */
};

typedef struct stVACore *VACB; /* Address of a stVACore VA-file descriptor */

#define    BFSIZE  100	       /* Size of VA buffer (# elements) */	


/*==========================================================================*/
/*                      VA descriptor - disk                                */
/*==========================================================================*/
/* Define layout of VA file-header : sizeof(struct stVAsize), excl. padding */
/* This header is located as the first record of a VA file on disk */
#define  HEADER         (2 * sizeof(DWORD) + sizeof(WORD) + sizeof(char))


/*==========================================================================*/
/*                      VA keyrecord format                                 */
/*==========================================================================*/
/* This structure defines a (default) VA file key-record;
 * The record consists of only one field holding a datafile-offset
 * for building a simple VA indexfile (with key-value = VA-index).
 * (You may construct a more complex record layout suitable for the needs
 * of your application, and define your own macro's for easy access)
 */
struct stVArec {
    DWORD      dwOffset;     /* VA[key-value] = datafile offset */
};

typedef struct stVArec REC;  /* Type of entry/record in VA-file */


/* Set up Macro's to access the VA file key-record field(s) */
#define pAREC(pVA, dw)       ((REC*) pvVAaccess(pVA, dw))
#define dwAOFF(pVA, dw)      pAREC(pVA, dw)->dwOffset

#define pRREC(pVA, dw)      ((REC*) pvVAread(pVA, dw))
#define dwROFF(pVA, dw)     pRREC(pVA, dw)->dwOffset



/****************************************************************************/
/**************************** FUNCTION PROTOTYPES ***************************/
/****************************************************************************/

/*------------------------ HIGH-LEVEL INDEX --------------------------------*/

_VA_CLASS PUBLIC eRetType
          eVAIdxCreate P((VACB * ppVA, char *pzIdxFile, WORD dummy1, DWORD dummy2));

_VA_CLASS PUBLIC eRetType
          eVAIdxOpen P((VACB * ppVA, char *pzIdxFile, char *pzAccess));

_VA_CLASS PUBLIC eRetType
          eVAIdxClose  P((VACB * ppVA));

_VA_CLASS PUBLIC eRetType
	  eVAIdxGetSize P((VACB *ppVA, DWORD *pdwSize, DWORD *pdwUsed));

_VA_CLASS PUBLIC eRetType
	  eVAIdxGetLoad P((VACB *ppVA, WORD *pwLoad));

/*------------------------ HIGH-LEVEL KEY ----------------------------------*/

_VA_CLASS PUBLIC eRetType
          eVAKeyInsert P((VACB * ppVA, char * pcKey, DWORD dwDatOffset));

_VA_CLASS PUBLIC eRetType
          eVAKeyDelete P((VACB * ppVA, char * pcKey));

_VA_CLASS PUBLIC eRetType
          eVAKeyFind P((VACB * ppVA, char * pcKey, DWORD * pdwDatOffset));

/*------------------------ LOW-LEVEL VA ------------------------------------*/

_VA_CLASS PUBLIC int
          iVAinit  P((char *pzIdxFile, WORD wElSize, int iFill));

_VA_CLASS PUBLIC VACB
          pVAopen  P((char *pzIdxFile, char *pzAccess));

_VA_CLASS PUBLIC int
          iVAclose P((VACB pVA));

/*------------------------ GENERIC INDEX/KEY -------------------------------*/

typedef   VACB ITYPE;

_VA_CLASS PUBLIC eRetType
          (*peIdxCreate)  P((ITYPE *, char *, WORD, DWORD))
          _VA_INIT(=eVAIdxCreate);

_VA_CLASS PUBLIC eRetType
          (*peIdxOpen)    P((ITYPE *, char *, char *))
          _VA_INIT(=eVAIdxOpen);

_VA_CLASS PUBLIC eRetType
          (*peIdxClose)   P((ITYPE *))
          _VA_INIT(=eVAIdxClose);

_VA_CLASS PUBLIC eRetType
          (*peIdxRead)    P((ITYPE *, char *, DWORD *))
          _VA_INIT(=eVAKeyFind);

_VA_CLASS PUBLIC eRetType
          (*peKeyInsert)  P((ITYPE *, char *, DWORD))
          _VA_INIT(=eVAKeyInsert);

_VA_CLASS PUBLIC eRetType
          (*peKeyDelete)  P((ITYPE *, char *))
          _VA_INIT(=eVAKeyDelete);

_VA_CLASS PUBLIC eRetType
          (*peKeyFind)    P((ITYPE *, char *, DWORD *))
          _VA_INIT(=eVAKeyFind);

_VA_CLASS PUBLIC eRetType
          (*peIdxGetSize) P((ITYPE *, DWORD *, DWORD *))
          _VA_INIT(=eVAIdxGetSize);

_VA_CLASS PUBLIC eRetType
          (*peIdxGetLoad) P((ITYPE *, WORD *))
          _VA_INIT(=eVAIdxGetLoad);


#endif	  /* #ifndef _VA_H */
/* End of module va.h                                                       */
/*-1========================================================================*/
