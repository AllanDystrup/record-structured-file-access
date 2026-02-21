/*+1========================================================================*/
/*   MODULE                       INDEX.H                                   */
/*==========================================================================*/
/*   FUNCTION      Headerfile for module INDEX.C
 *                 #include this file to access the PUBLIC index functions
 *                 defined in INDEX.C
 *
 *   SYSTEM        Standard (ANSI/ISO) C
 *                 Tested on PC/MS DOS 3.3 (MSC 600A) & UNIX SYS V3
 *
 *   SEE ALSO      Module INDEX.C
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Kommunedata I/S, Aug. 1991
 *
 *   VERSION       $Header: d:/cwork/index/RCS/index.h 0.1 92/07/10 10:53:08
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 ---------------------------------------------------------
 *                 $Log:	index.h $
 *                 Revision 0.1  92/07/10  10:53:08  Allan_Dystrup
 *                 PREREL (ALFA1)
 *
 *-1========================================================================*/


/****************************************************************************/
/**************************** HEADER FILES **********************************/
/****************************************************************************/
#ifndef _INDEX_H	   /* Make sure this header is included only once */
#define _INDEX_H           /* Matching endif is at end of file */


/*--------------------------------------------------------------------------*/
/*                           GENERAL DEF's                                  */
/* Include header with general definitions */ 
#ifndef _GENERAL_H
#include "general.h"
#endif


/*--------------------------------------------------------------------------*/
/*                           ACCESS METHOD                                  */
/* Include access header depending on compiler switch [-DVA | -DSS] :       */
/* Define a generic (data abstract) interface for accessing an indexfile,   */
/* and attach the generic function-pointers to the set of real functions.   */
#ifdef VA                 /* Compile with -DVA for Virtual Array */
#include "va.h"
#endif /* VA */

#ifdef SS                 /* Compile with -DSS for Scatter Storage */
#include "ss.h"
#endif /* SS */


/*--------------------------------------------------------------------------*/
/*                           ALLOC & INIT                                   */
/* Defines to handle declaration, allocation and initialization of global   */
/* data in a common header file.                                            */
/* The _INDEX_INIT(x) macro won't work with aggregates 'cause it interprets */
/* a comma as indicating a new macro parameter (not part of the current); - */
/* Aggregates are initialized inside a  #ifdef _INDEX_ALLOC ... #endif      */

/* Allocate space for globals */
#ifdef _INDEX_ALLOC                /* INDEX.C: Allocate space for globals - */
#   define _INDEX_CLASS            /*    ignore                             */
#   define _INDEX_INIT(x)  x       /*    initialize                         */
#else                              /* EXTERN: no allocation in user modules */
#   define _INDEX_CLASS    extern  /*    declare as extern                  */
#   define _INDEX_INIT(x)          /*    don't initialize                   */	
#endif                             /* END _INDEX_ALLOC -------------------- */

/* Stamp/Tag ID in object module; - Grep for "TAG" to retrieve ID-stamp */
_INDEX_CLASS const char INDEX_ID[]
_INDEX_INIT(= "\nTAG: MOD[index.h] VER[0.1.0 Pre] DATE[92/07/10] (C)[Allan Dystrup KMD I/S]");
 


/****************************************************************************/
/**************************** ERROR HANDLING ********************************/
/****************************************************************************/

/* eIndex_status is the different messages that may be returned */
typedef enum {
       I_OK,
       I_DATOPEN,
       I_DATSEEK,
       I_DATCLOSE,
       I_INDEX,
       I_OTHER
} eIndexStatus;


/* IERRMSG is the different messages returned from access modules */
_INDEX_CLASS char *IERRMSG[]
#ifdef _INDEX_ALLOC
= {
       "STATUS [I_OK].......: Funktion udf�rt uden fejl",
       "ERROR  [I_DATOPEN]..: Fejl ved �bning af datafil",
       "ERROR  [I_DATSEEK]..: Fejl ved positionering i datafil",
       "ERROR  [I_DATCLOSE].: Fejl ved lukning af datafil",
       "ERROR  [I_INDEX]....: Fejl index-operation, DISK FULD?",
       "ERROR  [I_OTHER]....: Anden fatal fejl"
}
#endif
;


/* Global error status INDICATORS */
_INDEX_CLASS eIndexStatus Istat;     /* General status code for last index function */    
_INDEX_CLASS int          Iid;       /* Unique tag for statement in last index func */


/* Macro for SETTING error-indicators for any index.c function */
#define IRET_ERR(bol,msg,tag) { if (bol) { Iid=tag; Istat=msg; return(ERROR); } }
#define IRET_OK               { Iid=0; Istat=I_OK;  return(OK); }


/* Macro for CHECKING global error-indicators from any index.c function */
#define	I_CONT	0       /* Continue processing after the error message */
#define	I_STOP	1       /* Terminate the program after the error message */

#define ICHK_ERR(stmt, actn)  { stmt; if(Istat != I_OK) {                              \
       fprintf(stderr, "\nMODUL: Fil[%s] - Linie[%d] ; VERSION: Dato[%s] - Tid[%s]",   \
                       __FILE__, __LINE__, __DATE__, __TIME__);                        \
       fprintf(stderr, "\nID: [index.c-%d-%4d]\t%s\n\a", Istat, Iid, IERRMSG[Istat]);  \
       if (actn == I_STOP) return (ERROR); } }



/****************************************************************************/
/**************************** FUNCTION PROTOTYPES ***************************/
/****************************************************************************/


/* -------------------- Public Indexmodule functions ---------------------- */

_INDEX_CLASS PUBLIC void
          vIdxDefine P((int argc, char *argv[], char *optstring));

_INDEX_CLASS PUBLIC eRetType
          eIdxMake P((ITYPE *hIdx, char *pzDatFile, WORD wKeyMark,
                      WORD wKeySize, char *pzIdxFile, DWORD dwIdxSize)); 

_INDEX_CLASS PUBLIC eRetType
          eIdxTest P((ITYPE *hIdx, char *pzDatFile, WORD wKeyMark));



#endif    /* #ifdef _INDEX_H */
/* End of module index.h                                                    */
/*-1========================================================================*/
