/*+1========================================================================*/
/*   MODULE                       ACCESS.H                                  */
/*==========================================================================*/
/*   FUNCTION      Error-handling macros for data access modules
 *                 #included in module VA.H and HASH.H
 *
 *   SYSTEM        Standard (ANSI/ISO) C
 *		   Tested on PC/MS DOS v.3.3 (MSC 600A) and UNIX SYS V.3 (GCC)
 *
 *   SEE ALSO      Modules : VA.H/C, HASH.H/C
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Allan Dystrup, Feb. 1992
 *
 *   VERSION       $Header: d:/cwork/index/RCS/access.h 0.1 92/07/02 11:54:41
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 -----------------------------------------------------------
 *                 $Log:	access.h $
 *                 Revision 0.1  92/07/02  11:54:41  Allan_Dystrup
 *                 PreRelease (ALFA1)
 * 
 *==========================================================================*/

#ifndef _ACCESS_H            /* Make sure this header is included only once */
#define _ACCESS_H            /* Matching #endif is at end-of-file */


/****************************************************************************/
/**************************** ERROR HANDLING ********************************/
/****************************************************************************/

/* eErrStatus is the different message-id's returned from access modules */
typedef enum { A_OK,
       A_NOTCREATE,
       A_FILEEXIST,
       A_FILEOPEN,
       A_FILECLOSE,
       A_WRONGFILE,
       A_NOFILE,
       A_BADALLOC,
       A_WRITE,
       A_READ,
       A_SEEK,
       A_DUPLICATE,
       A_NOTFOUND,
       A_NOTOPEN,
       A_ISOPEN,
       A_XPAND,
       A_MOREDATA,
       A_FULL,
       A_READONLY,
       A_OTHER
     } eErrStatus;


/* AERRMSG is the different messages returned from access modules */
_ACCESS_CLASS char *AERRMSG[]
#ifdef _ACCESS_ALLOC
     = {
       "STATUS  [A_OK]........:  everything went just ok",
       "ERROR   [A_NOTCREATE].:  could NOT create file, - disk full?",
       "ERROR   [A_FILEEXIST].:  index allready exists",
       "ERROR   [A_FILEOPEN]..:  could not open file",
       "ERROR   [A_FILECLOSE].:  could not close file",
       "ERROR   [A_WRONGFILE].:  error index CRC, - wrong file?",
       "ERROR   [A_NOFILE]....:  could'nt find index on media",
       "ERROR   [A_BADALLOC]..:  memory allocation error",
       "ERROR   [A_WRITE].....:  write-error (disk full?)",
       "ERROR   [A_READ]......:  read-error",
       "ERROR   [A_SEEK]......:  error trying to set file-ptr",
       "WARNING [A_DUPLICATE].:  key NOT unique,-not inserted",
       "WARNING [A_NOTFOUND]..:  item searched for NOT found",
       "ERROR   [A_NOTOPEN]...:  operation on not open index",
       "WARNING [A_ISOPEN]....:  index is allready open",
       "WARNING [A_XPAND].....:  expansion of index recommended",
       "WARNING [A_MOREDATA]..:  index processing interrupted",
       "WARNING [A_FULL]......:  index filled, - expand",
       "ERROR   [A_READONLY]..:  attempted write on RO index",
       "ERROR   [A_OTHER].....:  another fatal error has occured"
       }
#endif
;


/* Global error status INDICATORS */
_ACCESS_CLASS eErrStatus Astat;  /* General status code for last access function */    
_ACCESS_CLASS int Aid;           /* Unique tag for statement in last access func. */


/* Macro for SETTING error-indicators for any access function */
#define ARET_ERR(bol,msg,tag) { if (bol) { Aid=tag; Astat=msg; return(ERROR); } }
#define ARET_OK               { Aid=0; Astat=A_OK;  return(OK); }


/* Macro for CHECKING global error-indicators for an access function */
#define    A_CONT  0   /* Continue processing after the error message */
#define    A_STOP  1   /* Terminate the program after the error message */
#define    A_ABRT  2   /* Terminate with a "core" dump for UN*X debug */

#define ACHK_ERR(stmt, actn)  { stmt;  if(Astat != A_OK) {                           \
       fprintf(stderr, "\nMODUL: Fil[%s] - Linie[%d] ; VERSION: Dato[%s] - Tid[%s]", \
                      __FILE__, __LINE__, __DATE__, __TIME__);                       \
       fprintf(stderr, "\nID: [access-%d-%4d]\t%s\n", Astat, Aid, AERRMSG[Astat]);   \
       if (actn == A_STOP) return (ERROR);                                           \
       else                                                                          \
       if (actn == A_ABRT) abort(); }	}



#endif /* #ifndef  _ACCESS_H */
/* End of module access.h                                                   */
/*-1========================================================================*/
