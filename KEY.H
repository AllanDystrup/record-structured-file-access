/*+1 =======================================================================*/
/*   MODULE                       KEY.H                                     */
/*==========================================================================*/
/*   FUNCTION      Headerfile for module KEY.C (and user modules);
 *                 #include this file to access the PUBLIC functions in key.c
 *                 for opening, reading and closing "database" objects.
 *
 *   SYSTEM        Standard (ANSI/ISO) C, tested on PC/MS DOS v.3.3
 *
 *   SEE ALSO      Module KEY.C
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Kommunedata I/S, Feb. 1992
 *
 *   VERSION       $Header: d:/cwork/index/RCS/key.h 0.1 92/08/14 07:59:49
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 -----------------------------------------------------------
 *                 $Log:	key.h $
 *                 Revision 0.1  92/08/14  07:59:49  Allan_Dystrup
 *                 PREREL (ALFA1)
 * 
 *-1========================================================================*/



/****************************************************************************/
/**************************** HEADER FILES **********************************/
/****************************************************************************/
#ifndef _KEY_H             /* Make sure this header is included only once */
#define _KEY_H             /* Matching endif is at end of file */

/* Include header with general definitions */ 
#ifndef _GENERAL_H
#include "general.h"
#endif

#include <limits.h>        /* Needed below in this key.h headerfile */


/****************************************************************************/
/**************************** ACCESS METHOD *********************************/
/****************************************************************************/

/* Setup for either HASH or VA :
 * Define a generic (data abstract) interface for accessing an indexfile,
 * and attach the generic function-pointers to the set of "real" functions
 * chosen by the compiler switch : -DVA or -DSS
 */
#ifdef VA                  /* Compile with -DVA for Virtual Array */
#include "va.h"
#endif /* VA */
#ifdef SS                  /* Compile with -DSS for Scatter Storage */
#include "ss.h"
#endif /* SS */


/*--------------------------------------------------------------------------*/
/* Defines to handle declaration, allocation and initialization of global   */
/* data in a common header file.                                            */
/* The _KEY_INIT(x) macro won't work with aggregates because it interprets  */
/* a comma as indicating a new macro parameter (not part of the current); - */
/* Aggregates are initialized inside a  #ifdef _KEY_ALLOC ... #endif        */
/*--------------------------------------------------------------------------*/
#ifdef _KEY_ALLOC              /* KEY.C: Allocate space for globals ------- */
#   define _KEY_CLASS          /*    ignore                                 */
#   define _KEY_INIT(x) x      /*    initialize                             */
#else                          /* EXTERN: no allocation in user modules --- */
#   define _KEY_CLASS extern   /*    declare as extern                      */
#   define _KEY_INIT(x)        /*    don't initialize                       */
#endif                         /* END _KEY_ALLOC -------------------------- */

/* Stamp/Tag ID in object module; - Grep for "TAG" to retrieve ID-stamp */
_KEY_CLASS const char KEY_ID[]
_KEY_INIT(= "\nTAG: MOD[key.h] VER[0.1.0 Pre] DATE[92/07/10] (C)[Allan Dystrup KMD I/S]");



/****************************************************************************/
/**************************** SEARCH METHOD *********************************/
/****************************************************************************/
/* Include headers for access to search methods (eg. BM, Aho-Corasic, NFA).
 * ALL headers for the required search methods should be included (as opposed
 * to access methods, where you include only ONE of the available headers); -
 * The preferred search method may then be cosen DYNAMICALLY, as a function
 * of the current search expression, eg. :
 *    one word/sentence    -> BM
 *    boolean expression   -> Aho-Corasic (specific NFA)
 *    wildcards (reg.expr) -> general NFA	
 */
#include "UTIL/find/tbm/tbm.h"

/* Point generic search API to Tuned-Boyer-Moore (TBM) as default */
_KEY_CLASS void
           (*pvBldSearch)   P((BYTE *pat, int len))
           _KEY_INIT(=vBuildTBM);

_KEY_CLASS int
           (*piRunSearch)   P((BYTE *txt, int len))
           _KEY_INIT(=iRunTBM);



/****************************************************************************/
/**************************** ERROR HANDLING ********************************/
/****************************************************************************/

/* eKeyStatus is the different error-codes, that may be returned */
typedef enum {
    K_OK,
    K_BADALLOC,
    K_BADARGS,
    K_BADLIST,
    K_BADREAD,
    K_BUFOVFL,
    K_CACEMPTY,
    K_DATOPEN,
    K_DATSEEK,
    K_DATREAD,
    K_DATEOF,
    K_DATCLOSE,
    K_IDXOPEN,
    K_IDXSEEK,
    K_IDXREAD,
    K_IDXCLOSE,
    K_EOL
} eKeyStatus;


/* KERRMSG is the different messages returned for corresponding error-codes */
_KEY_CLASS char *KERRMSG[]
#ifdef _KEY_ALLOC
= {
    "STAT [K_OK].......: funktion udf�rt uden fejl",
    "FEJL [K_BADALLOC].: fejl i lager allokering",
    "FEJL [K_BADARGS]..: fejl i funktions aktuelle parametre",
    "FEJL [K_BADLIST]..: syntaksfejl i liste af n�glev�rdier",
    "FEJL [K_BADREAD]..: fejl ved l�sning af fil",
    "FEJL [K_BUFOVFL]..: databuffer overflow",
    "FEJL [K_CACEMPTY].: fors�g p� l�sning af tom cache",
    "FEJL [K_DATOPEN]..: fejl ved �bning af datafil",
    "FEJL [K_DATSEEK]..: fejl ved positionering i datafil",
    "FEJL [K_DATREAD]..: fejl ved l�sning fra datafil",
    "FEJL [K_DATEOF]...: End-Of-File ved l�sning fra datafil",
    "FEJL [K_DATCLOSE].: fejl ved lukning af datafil",
    "FEJL [K_IDXOPEN]..: fejl ved �bning af indexfil",
    "FEJL [K_IDXSEEK]..: fejl ved positionering i indexfil",
    "FEJL [K_IDXREAD]..: fejl ved l�sning fra indexfil",
    "FEJL [K_IDXCLOSE].: fejl ved lukning af indexfil",
    "WARN [K_EOL]......: ikke flere n�glev�rdier i liste"
}
#endif
;


/* Global error status INDICATORS */
eKeyStatus Kstat;          /* General status code for last key function  */
int        Kid;            /* Unique tag identifying statement for Kstat */


/* Macro for SETTING error-indicators for any key.c function */
#define KRET_ERR(bol,msg,tag)  { if (bol) { Kid=tag; Kstat=msg; return(ERROR); } }
#define KRET_OK                { Kid=0; Kstat=K_OK; return(OK); } 


/* Macro for CHECKING global error-indicators for a key.c function */
#define	K_STOP	0
#define	K_CONT	1
#define KCHK_ERR(stmt, actn)  { stmt; if(Kstat != K_OK) {                            \
       fprintf(stderr, "\nMODUL: Fil[%s] - Linie[%d] ; VERSION: Dato[%s] - Tid[%s]", \
                       __FILE__, __LINE__, __DATE__, __TIME__);                      \
       fprintf(stderr, "\nID: [key.c-%02d-%04d]  %s\n", Kstat, Kid, KERRMSG[Kstat]); \
       if (actn == K_STOP) return (ERROR); } }



/****************************************************************************/
/**************************** DATA STRUCTURES *******************************/
/****************************************************************************/

/*==========================================================================*/
/*                         CACHE - Fileoffset Cache                         */
/*==========================================================================*/
struct stKeyCache {          /* Datastruct. for cache of datafile offests:  */
    DWORD     (*padwData)[]; /* - Ptr. to array of (malloc'ed) dw-offsets   */
    DWORD      dwCsize;      /* - Current #MAX entries in the cache array   */
    DWORD      dwCused;      /* - Current #USED entries in the cache array  */
    DWORD      dwCbwin[2];   /* - IndexRange of padwData[] currently in buf */
                             /* dwCbwin[0] : index of first dataoffset      */
                             /* dwCbwin[1] : index of last dataofset        */
};

typedef struct stKeyCache CACHE;


/*==========================================================================*/
/*                         DBASE - DataBase                                 */
/*==========================================================================*/
struct stDataBase {        /* Datastruct. for cache'd & index'ed database : */
    CACHE     stCache;     /* - Cache datastructure (cf. struct stCache)    */
    FILE     *fdData;      /* - Data file pointer datastruct. (cf. stdio.h) */
    ITYPE     pIndex;      /* - Index handle datastruct. (cf. ss.h, va.h)   */
                           /*   ITYPE = [HASH | VA]                         */
};

typedef struct stDataBase DBASE;


/*==========================================================================*/
/*                         BUFFER - DataBuffer                              */
/*==========================================================================*/
struct stDataBuffer {      /* Datastructure for databuffer :      */
    char     *pzBaddr;     /* - Pointer to buffer area            */
    WORD      wBsize;      /* - Length of buffer area (in # byte) */
};

typedef struct stDataBuffer BUFFER;


/*==========================================================================*/
/*                         Public CONSTANTS                                 */
/*==========================================================================*/
#define    KEYMARK   ''           /* Unique tag in datafile marking a key */
#define    KEYMAX    50            /* Max allowed #chars in a KeyValue */
#define    KEYLEN     5            /* Default keylength for VA method */
#define    BLKSIZ   512            /* Blocksize (# byte) for datafile read */

/* Set up cache */
#define    K_LIST    -1000L        /* Init. cache from list of key values */
#define    K_EXPR    -2000L        /* Init. cache from search expression  */

/* Set cache bufferwindow */
#define    K_FIRST   -3000L        /* Position buf.win. on first cache slot */
#define    K_PREV    -4000L        /* Position buf.win. on previous cache slot */
#define    K_CURR    -5000L        /* Position buf.win. on current cache slot */
#define    K_NEXT    -6000L        /* Position buf.win. on next cache slot */
#define    K_LAST    -7000L        /* Position buf.win. on last cache slot */

/* Set #cache slots */
#define    K_ALL  ((INT_MAX)/2)    /* Retrieve all datarecords in cache */
#define    NEXT    ULONG_MAX       /* Read next record from datafile */



/****************************************************************************/
/************************* FUNCTION PROTOTYPES ******************************/
/****************************************************************************/
_KEY_CLASS PUBLIC eRetType 
           eKeyDBOpen  P((DBASE *pstDBx, char *pzDatFile, char *pzIdxFile,
                          BUFFER *pstBF, char *pzDatBuf, int iBufLen));

_KEY_CLASS PUBLIC eRetType 
           eKeyDBClose P((DBASE *pstDBx, BUFFER *pstBF));

_KEY_CLASS PUBLIC eRetType
           eKeyDBRead P((DBASE *pstDBx, BUFFER *pstBF,
                         long lSetPos, long lSetSiz));

_KEY_CLASS PUBLIC eRetType
           eKeyBufDump P((BUFFER *pstBF, FLAG fAll));



#endif /* #ifndef _KEY_H */
/* End of module key.h                                                      */
/*-1========================================================================*/