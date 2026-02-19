/*+1========================================================================*/
/*   MODULE                       HASH.H                                    */
/*==========================================================================*/
/*   FUNCTION      Headerfile for module HASH.C (and user modules);
 *                 #include this file for access to the PUBLIC scatter storage
 *                 functions defined in HASH.C.
 *
 *   SYSTEM        Standard (ANSI/ISO) C
 *                 Tested on PC/MS DOS v3.3 (MSC 600A) & UNIX SYS V.3 (GNU GCC)
 *
 *   SEE ALSO      Modules : GENERAL.H, ACCESS.H, HASH.C
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Allan Dystrup, Feb. 1992
 *
 *   VERSION       $Header: d:/cwork/index/RCS/hash.h 0.1 92/07/06 09:43:14
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 --------------------------------------------------------
 *                 $Log:	hash.h $
 *                 Revision 0.1  92/07/06  09:43:14  Allan_Dystrup
 *                 PREREL (ALFA1)
 *
 *-1=========================================================================*/


/****************************************************************************/
/**************************** HEADER FILES **********************************/
/****************************************************************************/
#ifndef _HASH_H             /* Make sure this header is included only once */
#define _HASH_H             /* Matching #endif is at end of file */


/* Include header with general definitions */ 
#ifndef _GENERAL_H
#include "general.h"
#endif


/*--------------------------------------------------------------------------*/
/* Defines to handle declaration, allocation and initialization of global   */
/* data in a common header file.                                            */
/* The _HASH_INIT(x) macro won't work with aggregates because it interprets */
/* a comma as indicating a new macro parameter (not part of the current); - */
/* Aggregates are initialized inside a  #ifdef _HASH_ALLOC ... #endif       */
/*--------------------------------------------------------------------------*/
#ifdef _HASH_ALLOC               /* HASH.C: Allocate space for globals ---- */
#   define _HASH_CLASS           /*    ignore                               */
#   define _HASH_INIT(x) x       /*    initialize                           */
#   define _ACCESS_CLASS         /*    ignore                               */
#   define _ACCESS_ALLOC         /*    ignore                               */
#else                            /* EXTERN: no allocation in user modules)  */
#   define _HASH_CLASS   extern  /*    declare as extern                    */
#   define _HASH_INIT(x)         /*    don't initialize                     */
#   define _ACCESS_CLASS extern  /*    declare as extern                    */
#   undef  _ACCESS_ALLOC         /*    don't allocate                       */
#endif                           /* END _HASH_ALLOC ----------------------- */

/* Include header with error handling definitions for access modules */ 
#ifndef _ACCESS_H
#include "access.h"
#endif

/* Stamp/Tag ID in object module; - Grep for "TAG" to retrieve ID-stamp */
_HASH_CLASS const char HASH_ID[] 
_HASH_INIT(= "\nTAG: MOD[hash.h] VER[0.1.0 Pre] DATE[92/07/10] (C)[Allan Dystrup]");



/****************************************************************************/
/**************************** DATA STRUCTURES *******************************/
/****************************************************************************/


/*==========================================================================*/
/*                      Basic hashinfo (disk & incore)                      */
/*==========================================================================*/
/* This structure contains the basic hash index file size informations :    */
struct stHsize {
     WORD  wKsize;         /* size of key */
     DWORD dwIsize;        /* must be a prime for maximum performance */
     DWORD dwIused;        /* number used of Isize */
};

typedef struct stHsize SIZEINFO;


/*==========================================================================*/
/*                      Hashindex descriptor - incore                       */
/*==========================================================================*/
/* This structure defines a hashindex in RAM ("incore") : status, id, size  */
/* - It is used as the common "object" datastructure by the hash-functions  */
/* - It is normally accessed by its address (HASH*) or a handle (HASH**)    */
struct stHcore {
     enum  {   ICLOSED=0,  /* Hashindex is closed    */
               IOPEN=1     /* Hashindex is open      */
           } indexstatus;  /* Hashindex open status  */

     enum  {   RW=0,       /* Hashindex is ReadWrite */
               RO=1        /* Hashindex is ReadOnly  */
           } indexmode;    /* Hashindex open mode    */

     FILE *fd;             /* Hashindex file descriptor, - from fopen() */
     char *filename;       /* Hashindex file name    */

     SIZEINFO indexsize;   /* Hashindex file size informations */
};

typedef struct stHcore *HASH;  /* Address of a stHcore hashindex descriptor */


/*==========================================================================*/
/*                      Hashindex descriptor - disk                         */
/*==========================================================================*/
/* Structure stHdisk is located as the first record of a hash-index file.   */
/* It includes a checksum of the record to prevent use of a wrong index     */
struct stHdisk { 
     long integrity;       /* Indicates that index has shut down nicely */
     int checksum;         /* Indicates that index header is valid */

     SIZEINFO indexsize;   /* Hashindex file size informations */
};


/*==========================================================================*/
/*                      Hashindex keyrecord format                          */
/*==========================================================================*/
/* stHkey is the format located in every entry of the hashindex on disk.    */
struct stHkey {
     enum  {   VACANT,     /* Index record free for use */
               USED,       /* Index record occupied     */ 
               DELETED     /* Index record deleted, - preserve hash chain! */
           } status;       /* Status of index record    */

     char *key;            /* Ptr to Keystring hash'ed to this index-record */
     DWORD dwDatOffset;    /* Offset in datafile of record identified by key */
};

typedef struct stHkey KEY; /* Type of entry/record in hashindex file */



/****************************************************************************/
/**************************** FUNCTION PROTOTYPES ***************************/
/****************************************************************************/


/*--------------------- Hash & Rehash transformations ----------------------*/

/* Define valid HashFunction (HF) transformations */
typedef enum {
       HF_FLL,             /* FLL: First+Last+Length */
       HF_ADD,             /* ADD: Sum-of-chars      */
       HF_PJW              /* PJW: "P.J.Weinburger"  */
} hash_func_type;          /* Hashing transformation */

/* Choose a hashing function */
_HASH_CLASS hash_func_type HF_TYPE _HASH_INIT(= HF_PJW);


/* Define valid RehashFunctions (RH) for collision resolution */
typedef enum {
       RF_LIN,             /* LIN: Linear probing    */
       RF_QAD,             /* QAD: Quadratic probing */
       RF_DBL              /* Double Hash probing    */
} rehash_func_type;        /* Rehash for collision resolution */

/* Choose a rehash function */
_HASH_CLASS rehash_func_type RF_TYPE _HASH_INIT(= RF_DBL);



/*---------------------- HIGH-LEVEL INDEX ----------------------------------*/

_HASH_CLASS PUBLIC eRetType
       eHashIdxCreate P((HASH *pH, char *pzIdxFile, WORD wKsize, DWORD dwIsize));

_HASH_CLASS PUBLIC eRetType
       eHashIdxOpen P((HASH *pH, char *pzIdxFile, char *pzAccess));

_HASH_CLASS PUBLIC eRetType
       eHashIdxClose P((HASH *pH));

_HASH_CLASS PUBLIC eRetType
       eHashIdxGetSize P((HASH *pH, DWORD *pdwSize, DWORD *pdwUsed));

_HASH_CLASS PUBLIC eRetType
       eHashIdxGetLoad P((HASH *pH, WORD *pwLoad));

_HASH_CLASS PUBLIC eRetType
       eHashIdxResize  P((HASH *pH, int iResize));

_HASH_CLASS PUBLIC eRetType
       eHashIdxProcess P((HASH *pH, WORD (*ufunc)(char *key, DWORD dwDatOffset)));


/*---------------------- HIGH-LEVEL KEY ------------------------------------*/

_HASH_CLASS PUBLIC eRetType
       eHashKeyInsert  P((HASH *pH, char *key, DWORD dwDatOffset));

_HASH_CLASS PUBLIC eRetType
       eHashKeyDelete  P((HASH *pH, char *key));

_HASH_CLASS PUBLIC eRetType
       eHashKeyFind P((HASH *pH, char *key, DWORD *pdwDatOffset));


/*---------------------- GENERIC INDEX/KEY ---------------------------------*/

typedef HASH ITYPE;

_HASH_CLASS PUBLIC eRetType
       (*peIdxCreate)  P((ITYPE *, char *, WORD, DWORD))
       _HASH_INIT(=eHashIdxCreate);

_HASH_CLASS PUBLIC eRetType
       (*peIdxOpen)    P((ITYPE *, char *, char *))
       _HASH_INIT(=eHashIdxOpen);

_HASH_CLASS PUBLIC eRetType
       (*peIdxClose)   P((ITYPE *))
       _HASH_INIT(=eHashIdxClose);

_HASH_CLASS PUBLIC eRetType
       (*peIdxRead)    P((ITYPE *, char *, DWORD *))
       _HASH_INIT(=eHashKeyFind);

_HASH_CLASS PUBLIC eRetType
       (*peKeyInsert)  P((ITYPE *, char *, DWORD))
       _HASH_INIT(=eHashKeyInsert);

_HASH_CLASS PUBLIC eRetType
       (*peKeyDelete)  P((ITYPE *, char *))
       _HASH_INIT(=eHashKeyDelete);

_HASH_CLASS PUBLIC eRetType
       (*peKeyFind)    P((ITYPE *, char *, DWORD *))
       _HASH_INIT(=eHashKeyFind);

_HASH_CLASS PUBLIC eRetType
       (*peIdxGetSize) P((ITYPE *, DWORD *, DWORD *))
       _HASH_INIT(=eHashIdxGetSize);

_HASH_CLASS PUBLIC eRetType
       (*peIdxGetLoad) P((ITYPE *, WORD *))
       _HASH_INIT(=eHashIdxGetLoad);

_HASH_CLASS PUBLIC eRetType
       (*peIdxResize)  P((ITYPE *, int))
       _HASH_INIT(=eHashIdxResize);

_HASH_CLASS PUBLIC eRetType
       (*peIdxProcess) P( (ITYPE *, WORD (*)(char *, DWORD)) )
       _HASH_INIT(=eHashIdxProcess);


#endif   /* #ifndef _HASH_H */
/* End of module hash.h                                                     */
/*-1========================================================================*/
