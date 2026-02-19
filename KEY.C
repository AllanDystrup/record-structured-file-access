/*+1========================================================================*/
/*   MODULE                       KEY.C                                     */
/*==========================================================================*/
/*   FUNCTION  Provides key-based access to key'ed, flat datafiles.
 *
 *             The module offers an object oriented program interface (API)
 *             to BASIC INFORMATION stored on disk in one or more flat
 *             DATAFILEs (sequential records of variable length, identified
 *             by a unique key: a fixed length string starting in 1. record-
 *             position), each with an associated INDEXFILE (mapping a key-
 *             value to a datafile offset for each datarecord); An keyfile
 *             may be generated from a datafile using the module key.c
 *
 *             The purpose of the key.c module is twofold :
 *              1  to implement a set of fast and reliable routines for the
 *                 general "low level" ACCESS to basic disk-information (ie.
 *                 file-access, caching and buffering operations), which is
 *                 typically a common requirement for several modules in the
 *                 same application (keyword: reuse).
 *              2  to hide the technical details of the implementation below
 *                 a general, easy-to-use application program interface / API
 *                 (keyword: data abstraction); - The key.c API allows the
 *                 user to access the basic information as very simple "data-
 *                 base(s)", using :
 *                  -  an abstract datastructure to represent each database
 *                     OBJECT (consisting of a data- & key-file w. attached
 *                     "in core" descriptor structures, plus a cache-array for
 *                     datarecord offsets and a buffer object for datarecords)
 *                  -  public interface FUNCTIONS for open-, read- and close-
 *                     operations on the database objects.
 *
 *              Some good reasons for NOT using a more general database product
 *              to satisfy these simple objectives are :
 *               1  commercial database products (as for instance "c-tree" by
 *                  FairCom Corp.) typically have a much "broader" functio-
 *                  nality, - which does also imply a more complex design,
 *                  a greater appetite for RAM and an inferior performance.
 *               2  why should your application depend on yet another product
 *                  (technically and economically) to fulfill just a simple
 *                  dataaccess requirement ?
 *
 *   SYSTEM     Standard (ANSI/ISO) C,
 *              Tested on PC/MS DOS V3.3 (MSC 600A) & UNIX SYS V.3 (GNU GCC).
 *
 *   SEE ALSO   Modules : GENERAL.H, KEY.H
 *
 *   PROGRAMMER Allan Dystrup
 *
 *   COPYRIGHT  (c) Allan Dystrup, Kommunedata I/S, may 1992
 *
 *   VERSION    $Header: d:/cwork/soul1/RCS/key.c 0.1 92/09/14 10:05:58
 *              Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *              --------------------------------------------------------------
 *              $Log:	key.c $
 *              Revision 0.1  92/09/14  10:05:58  Allan_Dystrup
 *              PREREL (ALFA1)
 *
 *   REFERENCES The concept of abstract data types is probably familiar; -
 *              If you want an introduction, consider :
 *              Azmoodeh, Manoochehr[1990]: "Abstract Data Types and Algorithms",
 *                 Macmillan Education Ltd., London (2.Ed) 
 *
 *   USAGE      Module key.c provides a "high level" interface for fast
 *              access to simple "DATABASE OBJECTS", each consisting of : 
 *              1: a DATAFILE on disk, - a flat/sequential file with variable
 *                 length records identified by unique key-values,
 *              2: an INDEXFILE on disk, - a table mapping each keyvalue to
 *                 the corresponding record-offset in the datafile,
 *              3: a DESCRIPTOR structure in memory, - a datastructure
 *                 maintained by module key.c for each database object,
 *                 coupling the data- & key-file with a cache-array for
 *                 datafile-offsets and a buffer for retrieving datarecords.
 *                 (The buffer may be shared by multiple DESCRIPTOR structs.)
 *
 *              The API offered by module key.c consists of 3 functions for
 *              accessing a key'ed flat datafile through a key.c DESCRIPTOR :
 *                 eKeyDBOpen()  // Open a database object (instantiate descr.)
 *                 eKeyDBClose() // Close a database object (release descr.)
 *                 eKeyDBRead()  // Read a database object in two steps :
 *                               // 1: build an offset-cache from a key-list.
 *                               // 2: fill/scroll a data-buffer from the cache
 *
 *              The user is expected to provide any application-specific
 *              functions such as displaying the databuffer contents on VDU
 *              (within the framework of the application's presentation method,
 *              be it text-based or a more sophisticated GUI as Cscape or Panel).
 *
 * DOC         Documentation is incorporated into the module and may be
 *             selectively extracted (using a utility such as ex.awk) :
 *                Level 1: Module documentation (history, design, testdriver)
 *                Level 2: PUBLIC functions (module program interface, "API")
 *                Level 3: major PRIVATE functions (design)
 *                Level 4: minor PRIVATE functions (support)
 *
 * BUGS        The module is coded in STANDARD C (ANSI/ISO), which is a
 *             feature! -  It is however prepared to compile under "old"
 *             K&R C (if you prefer bugs ...)
 *
 *
 * ========================== MODULE STRUCTURE ===============================
 *
 *                            Data model
 *   To define a database object in memory we need a descriptor datastructure
 *   for connecting the data- & key-fileptr's with a cache array of datafile
 *   offsets. The buffer for datarecords is defined as a separate structure,
 *   so the same buffer may be reused by several "databases".
 *
 *   (1) Database descriptor :
 *
 *                           RAM (incore)
 *                           struct stDataBase DBASE
 *                           +------------------+
 *                 +---------- CACHE   stCache  | cache datastructure
 *                 |         | FILE    *fpData  | data file pointer
 *                 |         | ITYPE   pIndex  -----------+ handle for key 
 *                 |         +------------------+         | 
 *                 |                                      | Compile switch : 
 *     RAM (incore)                                       | -DVA: ITYPE= VACB
 *     struct stKeyCache CACHE                            | -DSS: ITYPE= HASH
 *     +-------------------+ Ptr to array of offsets      | 
 *     | DWORD (*padwData)[]--->[ malloc'ed area ]        |
 *     |                   |                              |
 *     | DWORD  dwCsize    | max. entries in array        |             
 *     | DWORD  dwCused    | used entries in array        |          
 *     | DWORD  dwCbwin[2] | range of array in buffer     |                  
 *     +-------------------+                              |
 *                                                        |
 *                                               RAM (incore)
 *                                               struct ... ITYPE 
 *                                               +---------------------+
 *                            open mode [RO|RW]  | enum      keymode   |
 *                            key file pointer   | FILE      *fpIndex  | 
 *                                     +---------- SIZEINFO  stSize    |
 *                                     |         |<Cf. va.h and ss.h>  |
 *                                     |         +---------------------+
 *                         RAM (incore)
 *                         struct ... SIZEINFO
 *                         +---------------------+
 *                         |<Cf. va.h and ss.h>  |
 *                         +---------------------+
 *
 *
 *
 *   (2) Data buffer:
 *                           RAM (incore)
 *                           struct stDataBuffer BUFFER
 *                           +----------------+
 *                           | char  *pzBaddr | Ptr to buffer area 
 *                           | WORD  wBsize   | Size of buffer area
 *                           +----------------+  
 *
 *
 *
 *                           Function decomposition
 *   The key-module functions to access these datastructures may be grouped
 *   into two levels and three seperate categories :
 *   ( 1 ) API functions providing an external interface for calling modules
 *   (2.1) Functions operating internally on the CACHE datastructure,
 *   (2.2) Functions operating internally on the BUFFER datastructure.
 *   Thus the low-level CACHE & BUFFER functions (level 2) handle the direct
 *   access to files and "incore" descriptor structures, but these internal
 *   functions are wrapped in a high-level API (level 1) offering the basic
 *   operations of opening, reading and closing an abstract "database" object.
 *   An application (level 0) will call on the KEY API for data access, and
 *   typically introduce further functionality for user interaction (ie. data
 *   selection and buffer display).
 *
 *   The main calling hierachy is outlined in the following diagram, where
 *   the signatures -> and <- indicate input resp. output parameters, and
 *   the signature (__) represents repetition.
 *
 *   .........................................................................
 *   LEVEL 0
 *   APPLICATION
 *   (example)                  main  
 *                                |
 *             +------------------+-------------------+
 *             |                  |                   |
 *             |             eKeyDBAccess             |
 *             |                  |                   |
 *             |            eKeyListScroll            |
 *             |             |    |    |              |
 *             |            (___________)             |
 *             |             |    |   |               |
 *             |        getchar   |  eKeyBufDump      |
 *             |                  |                   |
 *   ..........|..................|...................|.......................
 *   LEVEL 1   |                  |                   |
 *   KEY API   |                  |                   |
 *             |                  |                   |
 *      eKeyDBOpen            eKeyDBRead          eKeyDBClose
 *                              /     \
 *                             |       |
 *      -> [K_LIST | K_EXPR]   |       |  -> (initialized cache)
 *      <- (initialized cache) |       |  <- (filled buffer)
 *                          __/\__     |___________________
 *                          | \/ |                        |
 *                   K_EXPR |    | K_LIST                 |
 *   .......................|....|........................|...................
 *   LEVEL 2                |    |                        | 
 *   KEY LOW-LEVEL          |    |                        | 
 *                          |    |                        | 
 *          +<--------------+    |                        |
 *          |                    |                        |
 *          |   +<---------------+                        |
 *          |   |                                         |
 *          |   |  (2.1) CACHE              (2.2) BUFFER  |
 *          |   |                                         |
 *          |   |  ->(keylist)                            |
 *          |   |  <-(cache)                              |
 *          |   +->eKeyCacheFill------------>eKeyBufScan  |
 *          |                |                            |
 *          |   +------------+                            |
 *          |   |                                         |
 *          |   +->eKeyCacheFree                          |
 *          |   |                                         |
 *          |   +->eKeyCacheAlloc                         |
 *          |                                        eKeyBufFill
 *          |
 *          |      ->(cache,pattern)
 *          |      <-(new cache)
 *          +----->eKeyCacheSearch --------->eKeyBufRead
 *                           |                          
 *   ........................|................................................
 *   LEVEL 3                 |
 *   SEARCH SERVICE          |
 *                           |               GENERIC SEARCH              
 *                           |
 *                           +-------------->(*pvBldSearch)()
 *                                           (*pfRunSearch)()
 *                                           (*pvDelSearch)()
 *                                           ----------------
 *                                                  |
 *                                           SPECIFIC SEARCH
 *                                        one of several methods
 *                                          cf. search modules
 *
 *-1========================================================================*/



/*==========================================================================*/
/*                                Includes                                  */
/*==========================================================================*/

/* ANSI X3J11 / ISO JTC1 SC22 WG14 Standard C headerfiles */
#include   <stdlib.h>
#include   <stdio.h>
#include   <malloc.h>
#include   <signal.h>
#include   <string.h>                                         
#include   <ctype.h>
#include   <math.h>

/* #define S/H-DEBUG: runtime check of stack- and heap on DOS  */
/* Relies on PC/MS DOS V.3.3 system files which are deprecated */
//#include "UTIL/chk/stck/stck.h"
   # define STCK(x)
   # define REAL_MAIN	main
//#include "UTIL/chk/hpck/hpck.h"


/* Project headerfile, w. PUBLIC errormacros, datastructures & functions */
#define    _KEY_ALLOC
#include   "key.h"


/*==========================================================================*/
/*                      (PRIVATE) Function prototypes                       */
/*==========================================================================*/

/* Main */
#ifdef MAIN
PRIVATE   eRetType
          eKeyDBAccess P((DBASE *pstDBx, BUFFER *pstBF, char *pzKlist));

PRIVATE   eRetType
          eKeyListScroll P((DBASE * pstDBx, BUFFER * pstBF));

PRIVATE   void 
          vSigCatch P((int vSigNum));
#endif


/* Cache */
PRIVATE   eRetType
          eKeyCacheFill P((DBASE * pstDBx, char *pzKeyList));

PRIVATE   eRetType
          eKeyCacheSearch P((DBASE * pstDBx, BUFFER * pstBF));

PRIVATE   eRetType
          eKeyCacheAlloc P((CACHE * pstCache, double rResize));

PRIVATE   eRetType
          eKeyCacheFree P((CACHE * pstCache));


/* Buffer */
PRIVATE   eRetType
          eKeyBufScan P((DBASE * pstDBx, char **ppzKeyList, char **ppzKeyStr));

PRIVATE   eRetType
          eKeyBufFill P((DBASE * pstDBx, BUFFER * pstBF, DWORD(*dwCbwinNew)[]));

PRIVATE   eRetType
          eKeyBufRead P((FILE *fpData, BUFFER *pstBF, DWORD *dwDRec));


/* Utility/Debug */
#ifdef DEBUG
PRIVATE   eRetType
          eKeyDBDump P((DBASE * pstDBx, BUFFER * pstBF));

PRIVATE   eRetType
          eKeyIndexDump P((DBASE * pstDBx));

PRIVATE   eRetType							    
          eKeyCacheDump P((DBASE * pstDBx, int fAll));

PRIVATE   eRetType
          eKeyRecDump P((char *key, DWORD dwFileOffset));

PRIVATE   eRetType
          eKeyDatDump P((FILE * fdDataFile, DWORD dwFileOffset));
#endif /* DEBUG */




#ifdef MAIN
/****************************************************************************/
/******************************** MAIN **************************************/
/****************************************************************************/
/*+1*/

/* 1: Set up nessecary variables (cf. datastructures defined in key.h)  */

/* Hard coded file names for data- & key-file (test):   */
/*    Format of data file name: must be [xxxxxxxx.dat]  */
/*    Default index file is then format [xxxxxxxx.idx]  */
PRIVATE char pzDat12[] = "soul.dat";	/* Name of datafile for DB 1&2 */
PRIVATE char pzIdx2[] = "soul2.idx";	/* Name of keyfile for DB 2 */

/* Two incore descriptors, one for each database (here DB1 & DB2) */
PRIVATE DBASE stDB1;           /* "Incore" descriptor for database1 */
PRIVATE DBASE stDB2;           /* "Incore" descriptor for database2 */

/* One incore descriptor for buffer, shared by database 1 & 2 */
PRIVATE BUFFER stBF;           /* "Incore" descriptor for (shared) buffer */
/* #define BUFLEN (5*1024) */  /* Length of realistic buffer for data-rec's */
#define BUFLEN    (512)        /* Length of test buffer for data-records */

/* Define "signon message" for key lookup testdriver */
PRIVATE const char SIGNON[] =
   "\nKMD Index access testdriver, Version 0.1.0\n"
   "MOD[key.c] VER[0.1.0 Pre] DAT[92/07/10] DEV[ad dec]\n"
   "Copyright (c) KommuneData I/S 1992\n\n";




/*== MODULE KEY.C ==========================================================*/
/*   NAME   01                    REAL_MAIN()                               */
/*== SYNOPSIS ==============================================================*/
int 
REAL_MAIN(argc, argv, envp)
  int argc;                    /* Argument count */
  char *argv[];                /* Argument vector */
  char *envp[];                /* Environment pointer */
{
/* DESCRIPTION
 *    Demoprogram and testdriver for module "key.c".
 *    The function demonstrates the proper use of datastructures and
 *    public functions declared in key.h and defined in key.c.
 *    (REAL_MAIN() is called from main() in stck.obj, - a module providing
 *    stack checking services; - you can safely ignore this debugging feature)
 *
 *    1: *DECLARE* variables for use as params to the functions in module key.c;
 *       For each "database" x (ie. key'ed flat data-file) to use, declare :
 *        - one  DBASE stDBx    to access the cache, key-file, and data-file
 *        - one  char pzDatx[]  initialized to name of the datafile, eg "datax.dat"
 *        - one  char pzIdxx[]  initialized to name of the keyfile;
 *                              If pzIdxx==NULL, the name is default "datax.idx"
 *                              otherwise a filename must be passed as parameter.
 *       For retrieving data-records from the datafile, declare :
 *        - one  BUFFER stBF    to hold the data-records in RAM; This variable
 *                              may be shared between multiple "databases".
 *
 *       The variables of the demo-program are declared with global scope,
 *       to make them accessible from the exception handler : vSigCatch()
 *
 *    2: *OPEN* each "database" before access by calling :
 *       eKeyDBOpen(&stDBx, pzDatx, pzIdxx, &stBF, <pzDatBuf>, <iBufLen>)
 *       This will allocate and initialize "incore" descriptors for the
 *       "database's" buffer, cache, key-file and data-file, as well as
 *       open the key- and data-file.
 *       The initialization of a databuffer (stBF) may be done in three ways :
 *       As the third pameter to eKeyDBOpen() you may pass :
 *        - &stBF  the address of a BUFFER struct. variable to initialize;
 *                 In this case the last two param's may take on the values :
 *                  - NULL,  BUFLEN  enthrust eKeyDBOpen() to allocate dataarea
 *                  - pzBuf, BUFLEN  pass ptr to an already allocated dataarea
 *                 In both cases BUFLEN is a #define'd constant for the length
 *                 of the buffer dataarea (# byte).
 *        - NULL   to omit initialization of a new buffer structure.
 *                 (if your intention is to reuse an already allocated buffer)
 *
 *    3: You may now *ACCESS* the "databases", that have been opened;
 *        1 first copy a comma-separated key-list to the buffer
 *        2 then call eKeyDBRead(&stDBx, &stBF, K_LIST, 0) to set up a cache,
 *          ie. array of datafile offests, for the keylist (using the key)
 *        3 finally call eKeyDBRead(&stDBx, &stBF, <position>, <records>) to
 *          read datarecords from the datafile to your buffer using cache-
 *          lookup operations; in the last two parameters to eKeyDBRead you
 *          pass an key-range (a "window") in the cache-array specifying the
 *          datarecords to retrieve: cache[position] - cache[position+records-1]
 *        4 Several symbolic constants have been #define'd to simplify scrol-
 *          ling the buffer-window through the cache (cf. eKeyListScroll()).
 *       This retrieval of datarecords via the cache, as well the whole
 *       procedure of keylist-/cache-setup and data read may be repeated
 *       as required by your application.
 *
 *    4: When you don't need a "database" any more (for instance before
 *       exiting your program!), you MUST *CLOSE* the "database" gracefully :
 *        - call eKeyDBClose(&stDBx, &stBF) to deallocate all "incore"
 *          descriptors and close the key- and data-files.
 *          If &stBF is set to NULL, eKeyDBClose() will not deallocate the
 *          databuffer structure, which may thus "survive" for subsequent use
 *          by other open "databases".
 *
 * RETURN
 *   REAL_MAIN() is a testdriver and not intented to interface with
 *   any calling program. The return value from REAL_MAIN() is thus
 *   insignificant in this context.
 *   You should however notice the checking of error-conditions on return
 *   from each call to a PUBLIC function defined in key.c; - This practice
 *   should also be followed in your application to "catch" and diagnose any
 *   malfunction in the system or in the services provided by this module.
 *   (You will probably want to write your own error-handling, though).
 *
 * EXAMPLE
 *   The contents of function REAL_MAIN() and the support-functions 
 *   eKeyDBAccess() and eKeyListScroll() constitute a self-contained
 *   main-program demonstrating the proper use of public data-structures
 *   and interface-functions in key.c.
 *
 * SEE ALSO
 *   - key.h for a detailed description of symbolic constants, macro's,
 *     data structures, return codes and error codes,
 *   - key.c/h  for a description of how the keyfile is generated from
 *     the datafile, using one of several available access methods.
 */

    /* 1: *DECLARE* : setup of variables for data- & key-access */
    /* DONE IN GLOBAL SCOPE */
    stDB1.fdData = stDB2.fdData = NULL;
    stDB1.pIndex = stDB2.pIndex = NULL;


    /* Signon and Setup to redirect interrupt signals to our own handler */
    fputs(SIGNON, stdout);

    (void) signal(SIGINT, vSigCatch);
    (void) signal(SIGTERM, vSigCatch);


    /*-DBASE 1--------------------------------------------------------------*/
    /* 2: *OPEN* database 1 (instantiate incore descriptor stDB1)           */
    /* Use default keyname (soul.idx), and let system allocate bufferarea */
    KCHK_ERR(eKeyDBOpen(&stDB1, pzDat12, NULL, &stBF, NULL, BUFLEN), K_STOP)
    D(eKeyIndexDump(&stDB1));

    /* 3: *ACCESS* database 1; define keylist, sHetup cache, retrieve rec's. */
    eKeyDBAccess(&stDB1, &stBF, "20200-20202,20203,20204-20206,20207,2099#-");


    /*-DBASE 2--------------------------------------------------------------*/
    /* 2: *OPEN* database 2 (instantiate incore descriptor stDB2)           */
    /* Use keyname "soul2.idx",and reuse already allocated buffer struct. */
    KCHK_ERR(eKeyDBOpen(&stDB2, pzDat12, pzIdx2, (BUFFER *) NULL, (char *) NULL, 0), K_STOP)
    D(eKeyIndexDump(&stDB2));

    /* 3: *ACCESS* database 2; define keylist, setup cache, retrieve rec's. */
    eKeyDBAccess(&stDB2, &stBF, "50###-");

    /* 4: *CLOSE* database 2, but let the buffer survive for use by stDB1 */
    KCHK_ERR(eKeyDBClose(&stDB2, NULL), K_STOP)
    D(eKeyDBDump(&stDB2, &stBF));


    /*-DBASE 1--------------------------------------------------------------*/
    /* 3: *ACCESS* database 1; define keylist, setup cache, retrieve recs */
    eKeyDBAccess(&stDB1, &stBF, "60000-69999");

    /* 4: *CLOSE* database 1, including the buffer (prev shared with stDB2) */
    KCHK_ERR(eKeyDBClose(&stDB1, &stBF), K_STOP)
    D(eKeyDBDump(&stDB1, &stBF));


    /* Function complete : return ok */
    STCK("REAL_MAIN");
    return (OK);

} /* END function REAL_MAIN() */
/*-1*/



/*+4 MODULE KEY.C ----------------------------------------------------------*/
/*   NAME   02.1               vSigCatch                                    */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE  void 
vSigCatch(iSigNum)
   int iSigNum;
{
/* DESCRIPTION
 *    Support function for REAL_MAIN() test driver, - (cf. REAL_MAIN, pt.1).
 *    Signal handler set up to catch the "break" signals : SIGINT (asynch.
 *    interactive attention) & SIGTERM (asynch. interactive termination).
 * RETURN
 *    Open databases closed & program terminated with exit code 'EXIT_FAILURE'.
 * BUGS
 *    The REAL_MAIN() test driver is dialog intensive, so we can't prompt the
 *    user for an abortion confirmation (interrupting a C-library(DOS) I/O-
 *    routine with a new I/O-request will *NOT* work due to non-reentrancy).
 *-4*/
      /* Close all open databases */
      fputs("\nINTERRUPT:\n\a", stderr); 
      fprintf(stderr, "\tSignal [%d] received\n", iSigNum);
      fputs("\tCleaning up...\n", stderr);
      eKeyDBClose(&stDB1, &stBF);
      eKeyDBClose(&stDB2, NULL);

      /* Quit test driver */
      fputs("\tBailing out...\n", stderr);
      exit(EXIT_FAILURE);

} /* END function vSigCatch() */



/*+1 MODULE KEY.C ----------------------------------------------------------*/
/*   NAME   02.2               eKeyDBAccess                                 */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE eRetType
eKeyDBAccess(pstDBx, pstBF, pzKlist)
   DBASE  *pstDBx;             /* Ptr to DBASE structure (w. cache array) */
   BUFFER *pstBF;              /* Ptr to BUFFER structure for data-rec's  */
   char   *pzKlist;            /* Ptr to list of key values for retrieval */
{
/* DESCRIPTION
 *    Support function for REAL_MAIN() test driver, - (cf. REAL_MAIN, pt.3).
 *    Access the database <pstDBx> using buffer <pstBF> and keylist <pzKlist>; 
 *    1-2: Parse keylist <pzKlist> and setup cache using eKeyDBRead(). 
 *     3 : Retrieve datarecords from file to buffer <pstBF> using eKeyDBRead().
 *     4 : Display cach'd datarecords using the simple eKeyListScroll() func.;
 *         The display func. will call eKeyDBRead() as needed to refill <pstBF>
 * RETURN
 *    Void (but errors checked inside function using the KCHK_ERR macro)
 */
    /* 1: Copy comma-separated keylist <pzKlist> to start of buffer */
    (void) strcpy(pstBF->pzBaddr, pzKlist);
    D(eKeyBufDump(pstBF, TRUE));

    /* 2: Setup dataoffset cache for <pstDBx> (from keylist in buffer) */
    KCHK_ERR(eKeyDBRead(pstDBx, pstBF, K_LIST, 0L), K_STOP)
    D(eKeyCacheDump(pstDBx, TRUE));

    /* 3: Retrieve all datarecords from the cache (up to the buffer limit) */
    KCHK_ERR(eKeyDBRead(pstDBx, pstBF, 1L, K_ALL), K_CONT)
    D(eKeyBufDump(pstBF, TRUE));

    /* 4: Scroll through the cache (ie. keylist), using simple KBD commands */
    KCHK_ERR(eKeyListScroll(pstDBx, pstBF), K_CONT)
    KRET_OK
} /* END function eKeyDBAccess() */
/*-1*/



/*+1 MODULE KEY.C ----------------------------------------------------------*/
/*   NAME   02.3               eKeyListScroll                               */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   eRetType
eKeyListScroll(pstDBx, pstBF)
    DBASE    *pstDBx;	       /* Pointer to DBASE structure w. cache array  */
    BUFFER   *pstBF;	       /* Pointer to BUFFER structure for data-rec's */
{
/* DESCRIPTION
 *    Support function for REAL_MAIN() test driver, called from eKeyDBAccess().
 *    Demonstrate and test the various parameters allowed by eKeyDBRead() for
 *    setting up and scrolling through a cache array.
 *
 *    LOOP prompt & get input ...
 *    0:  Set up "shorthand" var.'s for easy access to the cache bufferwindow.
 *    1:  Prompt for input (a char code) defining a reset of the bufferwindow.
 *    2:  Get input (a char code) from stdin, and dispatch to eKeyDBRead() to
 *        perform the actual cache reset, buffferwindow reset and databuffer
 *        fill operations
 *    3:  Call eKeyBufDump() to write the contents of the databuffer to stdout.
 *    WHILE (input not "QUIT")
 *
 * RETURN
 *    Side effects ........: Depending on input-codes from user/stdin ...
 *     - reset of cache-array (user entering a new key-list or key-expression)
 *     - reset of cache buf.window (user entering new window-pos. &/ -size)
 *     - fill of the buffer area (cf. cache slots currently in the buf.window)
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 */
    register int iAnswer= 'H'; /* Answer entered by user (via keyboard) */
    DWORD      dwWin[2];       /* Buffer window into cache array : TOP-BOT */
    DWORD      dwSiz    = 0L;  /* Current size of buf.window */
    DWORD      dwMax    = 0L;  /* Maximal size of buf.window */
    DWORD      dwSetPos = 0L;  /* For entering new pos. of buf.window */
    DWORD      dwSetSiz = 0L;  /* For entering new size of buf.window */


    /* Scroll through list of datarecords */
    do {
	/* 0: Initialize shorthand var's */
	dwWin[0] = pstDBx->stCache.dwCbwin[0];
	dwWin[1] = pstDBx->stCache.dwCbwin[1];
	dwMax    = pstDBx->stCache.dwCused;
	dwSiz    = pstDBx->stCache.dwCbwin[1] - pstDBx->stCache.dwCbwin[0] + 1;


	/* 1: Prompt for input */
	fputs("\nEnter code (H:HELP) [F|P|U|C|D|N|L|R|M|S|K|X|H|Q] -> ", stdout);


	/* 2: Get ONE char code from user/stdin, and dispatch to eKeyDBRead() */
       iAnswer = getchar();    /* NB: often implemented as MACRO */
       iAnswer = toupper(iAnswer);
       switch (iAnswer) {

          case 'F':            /* First <dwSiz> slots */
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, K_FIRST, dwSiz), K_CONT)
             break;

          case 'P':            /* Previous <dwSiz> slots */
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, K_PREV, dwSiz), K_CONT)
             break;

          case 'U':            /* Up 1 slot */
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, dwWin[0] - 1, dwSiz), K_CONT)
             break;

          case 'C':            /* Current <dwSiz> slots ("echo") */
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, K_CURR, dwSiz), K_CONT)
             break;

          case 'D':            /* Down 1 slot */
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, dwWin[0] + 1, dwSiz), K_CONT)
             break;

          case 'N':            /* Next <dwSiz> slots */
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, K_NEXT, dwSiz), K_CONT)
             break;

          case 'L':            /* Last <dwSiz> slots */
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, K_LAST, -dwSiz), K_CONT)
             break;

          case 'R':            /* Resize */
             printf("\nENTER height of bufferwindow :\n");
             printf("\tCurrent[%ld], New -> ", dwSiz);
             scanf("%ld", &dwSetSiz);
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, K_CURR, dwSetSiz), K_CONT)
             break;

          case 'M':            /* Move */
             printf("\nENTER position of bufferwindow :\n");
             printf("\tCurrent[%ld], New -> ", dwWin[0]);
             scanf("%ld", &dwSetPos);
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, dwSetPos, dwSiz), K_CONT)
             break;

          case 'S':            /* Set-new */
             printf("\nENTER position & height of bufferwindow :\n");
             printf("\tCurrent[%ld %ld], New -> ", dwWin[0], dwSiz);
             scanf("%ld %ld", &dwSetPos, &dwSetSiz);
             KCHK_ERR(eKeyDBRead(pstDBx, pstBF, dwSetPos, dwSetSiz), K_CONT)
             break;

          case 'K':            /* Key-list */
             do {
                printf("\nENTER list of comma-separated key-values :\n->");
                scanf("%s", pstBF->pzBaddr);
                KCHK_ERR(eKeyDBRead(pstDBx, pstBF, K_LIST, 0), K_CONT)
             } while (Kstat == K_BADLIST);
             break;

          case 'X':            /* Expression for key-class */
             do {
                printf("\nENTER search expression :\n->");
                scanf("%s", pstBF->pzBaddr);
                KCHK_ERR(eKeyDBRead(pstDBx, pstBF, K_EXPR, 0), K_CONT)
             } while (Kstat == K_BADLIST);
             break;

          default:             /* Error */
             fprintf(stderr, "\nERROR in input[%c-x%x-d%d], - try again ...\a",
                             (char) iAnswer, iAnswer, iAnswer);
             iAnswer = 'E'; 
             /* fall through! */

          case 'H':            /* Help */
             fputs("\neKeyListScroll() function SCROLL CODES :\n", stdout);
             fprintf(stdout, "\t+================================================================+\n");
             fprintf(stdout, "\t|  F : MOVE   pos. of    bufferwindow first %3ld slots  in cache |\n", dwSiz);
             fprintf(stdout, "\t|  P : MOVE   pos. of    bufferwindow up    %3ld slots  in cache |\n", dwSiz);
             fprintf(stdout, "\t|  U : MOVE   pos. of    bufferwindow up      1  slot   in cache |\n");
             fprintf(stdout, "\t|  C : KEEP   pos. of    bufferwindow curr  %3ld slot   in cache |\n", dwSiz);
             fprintf(stdout, "\t|  D : MOVE   pos. of    bufferwindow down    1  slot   in cache |\n");
             fprintf(stdout, "\t|  D : MOVE   pos. of    bufferwindow down    1  slot   in cache |\n");
             fprintf(stdout, "\t|  N : MOVE   pos. of    bufferwindow down  %3ld slots  in cache |\n", dwSiz);
             fprintf(stdout, "\t|  L : MOVE   pos. of    bufferwindow last  %3ld slots  in cache |\n", dwSiz);
             fprintf(stdout, "\t+----------------------------------------------------------------+\n");
             fprintf(stdout, "\t|  M : ENTER  pos.   of  bufferwindow  :   new   slot   in cache |\n");
             fprintf(stdout, "\t|  R : ENTER  height of  bufferwindow  :   new   #slots in cache |\n");
             fprintf(stdout, "\t|  S : ENTER  pos/height bufferwindow  :   new   slot & #slots   |\n");
             fprintf(stdout, "\t+----------------------------------------------------------------+\n");
             fprintf(stdout, "\t|  K : ENTER  list       of key values :   new   array for cache |\n");
             fprintf(stdout, "\t|  X : ENTER  expr.      of key-class  :   new   array for cache |\n");
             fprintf(stdout, "\t+----------------------------------------------------------------+\n");
             fprintf(stdout, "\t|  H : HELP   options f. eKeyListScroll()                        |\n");
             fprintf(stdout, "\t|  Q : QUIT   function   eKeyListScroll()                        |\n");
             fprintf(stdout, "\t|  A : ABORT  program    key.c                                   |\n");
             fprintf(stdout, "\t+================================================================+\n");
             break;

           case 'Q':           /* Quit */
              break;

	    case 'A':           /* Abort, - Commit harakiri */
              (void) raise(SIGTERM);
              /*NOTREACHED*/
              break;

	} /* END switch (iAnswer=toupper(getchar())) */


	/* Eat rest of input-line in silence */
	while (getchar() != '\n')
	     /* skip input chars up to (& including) newline */ ;

	/* 3: Trace the buffer */
       if ( iAnswer != 'H' && iAnswer != 'K' && iAnswer != 'X' &&
            iAnswer != 'E' && iAnswer != 'Q' &&
            pstDBx->stCache.dwCused > (DWORD) 0 )
          eKeyBufDump(pstBF, FALSE);

    } while (iAnswer != 'Q');


    /* Function complete : return ok */
    STCK("eKeyListScroll");
    KRET_OK

} /* END function eKeyListScroll */
/*-1*/

#endif	/* MAIN */




/****************************************************************************/
/******************************** DB ****************************************/
/****************************************************************************/



/*+2 MODULE KEY.C ==========================================================*/
/*   NAME   03                    eKeyDBOpen                                */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eKeyDBOpen(pstDBx, pzDatFile, pzIdxFile,
           pstBF, pzDatBuf, iBufLen)
    DBASE    *pstDBx;         /* Pointer to DBASE struct. to initialize */
    char     *pzDatFile;      /* - Name of datafile for DBASE stDBx */
    char     *pzIdxFile;      /* - Name of keyfile for DBASE stDBx */
    BUFFER   *pstBF;          /* Pointer to BUFFER structure data records */
    char     *pzDatBuf;       /* - Location of databuffer for BUFFER stBF */
    int      iBufLen;         /* - Length of databuffer for BUFFER stBF */
{
/* DESCRIPTION
 *    Opens a "database" (an key'ed flat datafile) for subsequent record
 *    retrieval based on a list of key-values or on a search expression.
 *    The positions (offsets) of the records to retrieve from the datafile
 *    are determined in one of two ways :
 *     - KeyList....: offsets corresponding to key-values are looked up in a
 *                    previously generated keyfile (cf. module key.c).
 *     - SearchExpr.: offsets are found by a scan of the datafile sections
 *                    specified by the current cache array (cf. module search.c).
 *    In both cases the record positions are "cache'd" in an array of data-
 *    file offsets to facilitate fast subsequent scrolling through identified
 *    records; The scrolling operation comprises maintaining a range of active
 *    key'es (a "window") into the cache array, and filling a RAM buffer-
 *    area with data-records corresponding to the active buffer-window.
 *
 *    The eKeyDBOpen() function thus implies opening of a datafile with
 *    associated keyfile, plus initialization of a cache and optionally
 *    a databuffer for subsequent access to the datarecords :
 *
 *    1: Initialize CACHE : set structure variables to all zero (NULL/0)
 *    2: Initialize INDEX : set name of keyfile, and instantiate an "incore"
 *       keydescriptor structure (including opening of the keyfile).
 *       In DEBUG-mode : dump keystatus (size & load) to stdout.
 *    3: Initialize DATAFILE : open datafile as textfile, mode ReadOnly (RO)
 *    4: Initialize DATABUFFER : 3 possible cases ...
 *       4.1: pstBF != NULL & pzDatBuf == NULL : allocate a new dataarea
 *       4.2: pstBF != NULL & pzDatBuf != NULL : use caller-provided dataarea
 *       4.3: pstBF == NULL .................. : ignore buffer initialization
 *
 * RETURN
 *    Side effects ........: Cache structure variables reset to all NULL/0.
 *                           Index-structure instantiated, keyfile opened.
 *                           Datafile opened & (optionally) buffer allocated.
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 *
 * SEE ALSO
 *    The functions eKeyDBRead() and eKeyDBClose(), both in module key.c
 *-2*/

    char      aIdxFile[12];       /* Name of keyfile */
    char      *pIdx    = NULL;    /* - with associated ptr */
    eRetType  eRetCode = ERROR;   /* Return code for key.c functions */
    int       iRetCode = 1;       /* Int return code for setvbuf() */
#ifdef DEBUG
    DWORD     dwIdxSiz = 0L;      /* Debug: Index size  */
    DWORD     dwIdxUse = 0L;      /* Debug: Index usage */
    WORD      wIdxLoad = 0;       /* Debug: Index load  */
#endif			       /* DEBUG */


    /*- CACHE --------------------------------------------------------------*/
    /* 1: Init. CACHE array (not allocated) & buffer window (empty)         */

    /* Cache array */
    pstDBx->stCache.padwData = NULL;   /* Pointer to array of key-rec's */
    pstDBx->stCache.dwCsize  = 0L;     /* Size of array, initially zero */

    /* Buffer window : key range of stCache.padwData[] currently in buf. */
    pstDBx->stCache.dwCused    = 0L;   /* Total # of dataoffsets in cache */
    pstDBx->stCache.dwCbwin[0] = 0L;   /* Index of first dataoffset in buffer */
    pstDBx->stCache.dwCbwin[1] = 0L;   /* Index of last dataoffest in buffer  */


    /*- INDEX --------------------------------------------------------------*/
    /* 2: Init. INDEX filename & call key-open to instantiate keystruct */

    /* Set up name for keyfile */
    if (pzIdxFile == NULL) {
	/* Use default name for the keyfile : "pzDatFile".idx */
	(void) strncpy(aIdxFile, pzDatFile, 12);
	if ((pIdx = strchr(aIdxFile, '.')) == NULL) {
	    pIdx = aIdxFile + strlen(aIdxFile);
	}
	(void) strcpy(pIdx, ".idx");
    }
    else
	/* Use name passed as parameter for the keyfile */
	(void) strncpy(aIdxFile, pzIdxFile, 12);

    /* Call specific key access-function to open key, ie. :  */
    /* instantiate an "incore" key descriptor & open keyfile */
    eRetCode = (*peIdxOpen)(&(pstDBx->pIndex), aIdxFile, "rb");
    KRET_ERR(eRetCode == ERROR, K_IDXOPEN, 300)

#ifdef DEBUG
    /* Trace status of key to stdout */
    KCHK_ERR((*peIdxGetSize)(&(pstDBx->pIndex), &dwIdxSiz, &dwIdxUse), K_CONT)
    fprintf(stdout, "Index Keyrecords : Size=[%lu], Used=[%lu]\n", dwIdxSiz, dwIdxUse);
    KCHK_ERR((*peIdxGetLoad)(&(pstDBx->pIndex), &wIdxLoad), K_CONT)
    fprintf(stdout, "Index Loadfactor=%d\n", wIdxLoad);
#endif	/* DEBUG */


    /*- DATAFILE -----------------------------------------------------------*/
    /* 3: Init. DATAFILE : open & set up filesystem buffer                  */

    /* Open datafile as textfile mode RO, and give it a large I/O buffer */
    pstDBx->fdData = fopen(pzDatFile, "r");
    KRET_ERR(pstDBx->fdData == NULL, K_DATOPEN, 301)

    iRetCode = setvbuf(pstDBx->fdData, NULL, _IOFBF, 4*1024);
    KRET_ERR(iRetCode != 0, K_DATOPEN, 302) 


    /*- DATABUFFER ---------------------------------------------------------*/
    /* 4: Init. DATABUFFER : allocate & initialize buffer area, - optional! */

    /* 4.1-2: Fill in a BUFFER datastructure, if provided as argument! */
    if (pstBF != NULL) {

	KRET_ERR(iBufLen <= 0, K_BADARGS, 303)

	/* 4.1: Allocate a new dataarea for the BUFFER */
	if (pzDatBuf == NULL) {
	    /* Initialize to newly allocated dataarea */
	    pstBF->pzBaddr = (char *) calloc(iBufLen, sizeof(char));
	    KRET_ERR(pstBF->pzBaddr == NULL, K_BADALLOC, 304)
	}
	else {
	    /* 4.2: Use dataarea previously allocated by caller */
	    pstBF->pzBaddr = pzDatBuf;
	}

	/* Clear & Zero-terminate the dataarea */
	(void) memset(pstBF->pzBaddr, ' ', iBufLen);
	D(memset(pstBF->pzBaddr, '*', iBufLen));
	*(pstBF->pzBaddr + iBufLen - 1) = '\0';

	/* Set the buffer length */
	pstBF->wBsize = iBufLen;
    }

    /* 4.3: else (pstBF == NULL), - Ignore BUFFER initialization! */


    /* Function complete : return ok */
    STCK("eKeyDBOpen");
    KRET_OK

} /* END function eKeyDBOpen() */



/*+2 MODULE KEY.C ==========================================================*/
/*   NAME   04                    eKeyDBClose                               */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eKeyDBClose(pstDBx, pstBF)
    DBASE    *pstDBx;         /* Pointer to DBASE structure to close */
    BUFFER   *pstBF;          /* Pointer to BUFFER structure to close */
{
/* DESCRIPTION
 *    Closes a "database", that has previously been opened by eKeyDBOpen();
 *    The function "reverses" the actions of opening a "database" : it
 *     - closes the datafile and its accociated keyfile, and
 *     - free's the database cache and (optionally) the databuffer.
 *
 *    1: Shut down CACHE : free the cache array and reset cache struct. var's
 *    2: Shut down INDEX : free the "incore" keydescriptor structure
 *                         (including closing the keyfile).
 *    3: Shut down DATAFILE : close the datafile
 *    4: Shut down DATABUFFER : 2 possible cases ...
 *       4.1: pstBF != NULL : free buffer's dataarea, and reset buffersize
 *       4.2: pstBF == NULL : ignore release of buffer (stBF may be reused)
 *
 * RETURN
 *    Side effects ........: Cache array deallocated & size-var's reset to 0.
 *                           Indexfile closed & key-structure deallocated.
 *                           Datafile closed & (optionally) buffer deallocated.
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 *
 * SEE ALSO
 *   The function eKeyDBOpen() in module key.c.
 *-2*/

    eRetType  eRetCode = ERROR;   /* Return code for key.c functions */

    /* 0: Validate input parameter (prevent closing a not opened DB) */
    KRET_ERR(pstDBx->fdData == NULL, K_BADARGS, 400)


    /* 1: CACHE Shut down */
    eRetCode = eKeyCacheFree(&(pstDBx->stCache));
    KRET_ERR(eRetCode == ERROR, Kstat, 401)


    /* 2: INDEX Shut down : call specific key access-function. */
    eRetCode = (*peIdxClose)(&(pstDBx->pIndex));
    KRET_ERR(eRetCode == ERROR, K_IDXCLOSE, 402)


    /* 3: DATAFILE Shut down */
    eRetCode = (fclose(pstDBx->fdData) != 0 ? ERROR : OK);
    KRET_ERR(eRetCode == ERROR, K_DATCLOSE, 403)


    /* 4: DATABUFFER Shut down, - Optional (NB: buffer may be shared!) */
    if (pstBF != NULL) {
	/* 4.1: Free the buffer dataarea, and zero the buffersize */
	free(pstBF->pzBaddr);
	pstBF->pzBaddr = NULL;

	pstBF->wBsize = 0;
	pstBF = NULL;
    }
    /* 4.2: else (pstBF == NULL), - Ignore buffer release */


    /* Function complete : return ok */
    STCK("eKeyDBClose");
    KRET_OK

} /* END function eKeyDBClose() */ 



/*+2 MODULE KEY.C ==========================================================*/
/*   NAME   05                    eKeyDBRead                                */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eKeyDBRead(pstDBx, pstBF, lSetPos, lSetSiz)
    DBASE    *pstDBx;	       /* Pointer to DBASE struct. for reading data  */
    BUFFER   *pstBF;	       /* Pointer to BUFFER struct. for writing data */
    long      lSetPos;	       /* Startkey into stDBx->stCache.padwData[]  */
    long      lSetSiz;	       /* Number of padwData[] entries to retrieve   */
{
/* DESCRIPTION
 *    The "procedural strength" of this function has deliberately been chosen
 *    to be "communicational" (as opposed to "functional", - cf. Glenford
 *    J. Meyers' structured design terminology) : the function performs several
 *    related actions on the same data in the same code section (procedure).
 *    The aim of this design has been to provide the user with ONE GENERAL
 *    INTERFACE to all access-operations on a "database" (an key'ed flat
 *    datafile) :
 *     - the main function is to FIND and READ one or more data-records from
 *       a "database" (identified by pstDBx) to a buffer (identified by pstBF).
 *       The datarecord(s) are identified by a list of key-values or by a
 *       search expression, both passed as a zero-terminated string in the
 *       buffer-area (pstBF->pzBaddr).
 *     - the cache datastructure has been introduced to allow fast and easy
 *       PRESENTATION of already identified datarecords, using direct retrie-
 *       val by datafile offsets (ie. short cutting the time consuming FIND
 *       operations of key-lookup and text-search). The presentation options
 *       offered are : movement <lSetPos> and resizing <lSetSiz> of an key
 *       range in the cache array (a "buffer-window") specifying the records
 *       to be read from the datafile to the buffer-area.
 *
 *   The procedure of accessing a "database" thus requires :
 *    - one call of eKeyDBRead() with a key-list or a key-expression in the
 *      buffer-area <pstBF> to FIND a sequence of datafile offsets, and build
 *      a CACHE datastructure from these,
 *    - one or more subsequent calls of eKeyDBRead() with <lSetPos> and
 *      <lSetSiz> defined to RESET the cache bufferwindow and REFILL the data-
 *      buffer accordingly.
 *   The actions of redefining a cache array and resetting a cache buffer-
 *   window (thereby refilling the bufferarea) are intimately connected
 *   (logically, temporal, procedural and communicational strength), and
 *   I have therefore chosen to provide them as a single function for acces-
 *   sing a database :
 *
 *    Depending on the value of <lSetPos> the function may proceed along
 *    3 different paths :
 *    1: K_EXPR   the buffer pstBF contains a zero-terminated string defining
 *                a search expression; the function calls on eKeyCacheSearch()
 *                to search datarecords defined by a cache of datafile-offsets
 *                for a content "matching" the search expression.
 *    2: K_LIST   the buffer pstBF contains a zero-terminated string holding
 *                a comma-separated list of key values; the function calls on
 *                eKeyCacheFill() to set up a cache of datafile-offsets by
 *                looking up the keys in the keyfile and reading the offsets.
 *    3:<lSetPos> a specifcation of the position in a previously defined cache
 *      3.1:       - EITHER  any of a series of #defined symbolic constants
 *      3.2:       - OR      a numeric key value
 *      3.3:      Taken together <lSetPos> and <lSetSiz> defines a range of
 *                slots in the cache-array (a "bufferwindow") defining the
 *      3.4:      datarecords to be read from the datafile to the buffer, -
 *                an operation performed by calling function eKeyBufFill().
 *     After a call with K_EXPR or K_LIST to set up a cache-array, one may
 *     thus use the cache for subsequent direct retrieval of datarecords from
 *     the datafile to the buffer (thereby facilitating fast and intelligent
 *     buffer-handling such as vertical scrolling). The actual display of the
 *     buffer contents is left to the calling application, using its preferred
 *     user interface.
 *
 * RETURN
 *    Side effects ........: Depending on the value of parameter <lSetPos>,
 *                           - K_EXPR, K_LIST : cache allocated & initialized
 *                           - symbl./numeric : buffer filled cf. buf.window
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 *
 * SEE ALSO
 *    The functions eKeyDBOpen() and eKeyDBClose(), both in module key.c
 *-2*/

    DWORD     dwMax = pstDBx->stCache.dwCused; /* #slots used in cache */
    long      lNew[2];                         /* New bufferwindow in cache */
    long      lTmp = 0L;                       /* For "swap" of window TOP/BOT */
    eRetType  eRetCode = ERROR;


    /* INITIALIZE CACHE ARRAY or RESET CACHE WINDOW POS., cf. <lSetPos> */
    switch (lSetPos) {

	    /* 1-2: INITIALIZE CACHE ARRAY --------------------------------- */
	case K_EXPR:
           /* 1: Initialize a cache from a SEARCH-EXPRession in buffer */
           eRetCode = eKeyCacheSearch(pstDBx, pstBF);
           KRET_ERR(eRetCode == ERROR, Kstat, 500)
           KRET_OK
           /*NOTREACHED*/
           break;

	case K_LIST:
           /* 2: Initialize a cache from a KEY-LIST in buffer */
           eRetCode = eKeyCacheFill(pstDBx, pstBF->pzBaddr);
           KRET_ERR(eRetCode == ERROR, Kstat, 501)
           KRET_OK
           /*NOTREACHED*/
           break;

           /* 3.1: RESET CACHE WINDOW POS., using symbolic constant ------- */
	case K_FIRST:           /* First <lSetSiz> slots */
           lNew[0] = 1;
           break;

	case K_PREV:            /* Previous <lSetSiz> slots */
           lNew[0] = pstDBx->stCache.dwCbwin[0] - lSetSiz;
           break;

	case K_CURR:            /* Current <lSetSiz> slots ("echo") */
           lNew[0] = pstDBx->stCache.dwCbwin[0];
           break;

	case K_NEXT:            /* Next <lSetSiz> slots */
           lNew[0] = pstDBx->stCache.dwCbwin[1] + 1;
           break;

	case K_LAST:            /* Last <lSetSiz> slots */
           lNew[0] = (long) dwMax;
           break;

	    /* 3.2: RESET CACHE WINDOW POS., using numeric key ----------- */
	default:
           lNew[0] = lSetPos;
           break;

    } /* END switch (lSetPos) */


    /* 3.3: RESET CACHE WINDOW SIZE (clamp to actual cache range) */
    lNew[0] = (long) ( lNew[0] < 1L ? 1L :
                      (lNew[0] > (long) dwMax ? (long) dwMax : lNew[0]));

    /* Set new window range, and clamp to actual cache range */
    lSetSiz += ( lSetSiz > 0L ? -1L :
                (lSetSiz < 0L ?  1L : 0L));
    lNew[1] = lNew[0] + lSetSiz;
    lNew[1] = (long) ( lNew[1] < 1L ? 1L :
                      (lNew[1] > (long) dwMax ? (long) dwMax : lNew[1]));

    /* Swap window Top/Bottom-key, if nessecary */
    if (lNew[1] < lNew[0]) {
	lTmp    = lNew[0];
	lNew[0] = lNew[1];
	lNew[1] = lTmp;
    }


    /* 3.4: FILL BUFFER according to the new cache window */

    /* Check for valid cache : the cache must contain at least 1 element */
    assert(dwMax >= 0L);
    KRET_ERR(dwMax == 0L, K_CACEMPTY, 502)

    /* Read rec's from datafile to buffer cf. to new cache window: lNew[] */
    eRetCode = eKeyBufFill(pstDBx, pstBF, (DWORD(*)[])(&(lNew[0])));
    KRET_ERR(eRetCode == ERROR, Kstat, 503)


    /* Function complete : return ok */
    STCK("eKeyDBRead");
    KRET_OK

} /* END function eKeyDBRead() */




/****************************************************************************/
/******************************** CACHE *************************************/
/****************************************************************************/



/*+2 MODULE KEY.C ==========================================================*/
/*   NAME   06                    eKeyCacheFill                             */
/*== SYNOPSIS ==============================================================*/
PRIVATE   eRetType
eKeyCacheFill(pstDBx, pzKeyList)
    DBASE    *pstDBx;         /* Ptr to DBASE structure w. cache to create */
    char     *pzKeyList;      /* List of keys for data offsets to "cache"  */
{
/* DESCRIPTION
 *    Setup a cache array (pstDBx->stCache) from a list of comma-separated
 *    key specifications (pzKeyList), using lookup operations on the key-
 *    file (pstDBx->pIndex) to directly map keyvalues to record-offsets.
 *
 *    The keyfile has been previously generated from the datafile (by
 *    module key.c) as a collection of key-records identifying the
 *    mapping [keystring:fileoffset] for all records in the datafile.
 *
 *    1: INITIALIZE
 *       Create a cache-array to hold offsets into the datafile for the data-
 *       records specified by the keyvalues in <pzKeyList>
 *
 *    2: LOOKUP & CACHE ...
 *       Fill the cache-array with file-offsets (by lookup in the keyfile):
 *       LOOP through the <pzKeyList> string :
 *          2.1: Call on eKeyBufScan() to get next keyvalue from <pzKeyList>
 *          2.2: Lookup key-record in keyfile, and retrieve datafileoffset
 *          2.3: If key found, insert the datarecord offset in the cache-array
 *               else: ignore key; Expand cache array dynamically as required.
 *       WHILE (more keys in <pzKeyList>, or parse-error)
 *
 *    3: TERMINATE
 *       Set number of slots currently used in cache (= #keys from <pzKeyList>
 *       found in the key)
 *
 * RETURN
 *    Side effects ........: Old cache (if any) deallocated, new cache
 *                           allocated and fill'd in with a datarecord offset
 *                           for each keyvalue in <pzKeyList> found in key.
 *                           The cache struct. slotcount <dwCused> is updated.
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 *-2*/

    char     *pzKeyStr   = NULL;   /* Ptr into pzKeyList, set by eKeyBufScan */
    DWORD     dwKeyCount = 0L;     /* Current slot: pstDBx->stCache.padwData */
    DWORD     dwDatFile  = 0L;     /* Flatfile offset into datafile */
    eRetType  eRetCode   = ERROR;  /* Function return code */


    /* 1: INITIALIZE: Create cache for datafile offsets... */
    /* First free old cache, if any */
    if (pstDBx->stCache.padwData != NULL) {
	eRetCode = eKeyCacheFree(&(pstDBx->stCache));
	KRET_ERR(eRetCode == ERROR, Kstat, 600)
    }
    /* Then allocate new cache, initial size */
    eRetCode = eKeyCacheAlloc(&(pstDBx->stCache), 0.0);
    KRET_ERR(eRetCode == ERROR, Kstat, 601)


    /* 2: LOOP: LOOKUP & CACHE ... */
    /* Lookup datafile-offset: key[key], and insert offset in cache */
    while (eKeyBufScan(pstDBx, &pzKeyList, &pzKeyStr) == OK) {

	/* 2.1: Retrieve next keyvalue from <pzKeyList> : eKeyBufScan() */
	D(fprintf(stdout, "Cache#[%lu] Key#[%s]\n", dwKeyCount+1, pzKeyStr));

	/* 2.2: Look up datafile offset <dwDatFile> in keyfile[pzKeyStr] */
	(void) (*peIdxRead)(&(pstDBx->pIndex), pzKeyStr, &dwDatFile);
	KRET_ERR(Astat != A_NOTFOUND && Astat != A_OK, K_IDXREAD, 602)

	/* 2.3: If key defined in key, - insert dataoffset in cache */
	if (Astat == A_OK) {

	    /* Resize cache if overflow : expand to double size! */
	    if (dwKeyCount >= pstDBx->stCache.dwCsize) {
	        eRetCode = eKeyCacheAlloc(&(pstDBx->stCache), 2.0);
	        KRET_ERR(eRetCode == ERROR, Kstat, 603)
	    }

	    /* Insert datafileoffset in cache [1...dwCsize-1], slot 0 unused */
	    (*(pstDBx->stCache.padwData))[++dwKeyCount] = dwDatFile;

	    /* Trace key keyrecord and flatfile datarecord (optional) */
	    D(eKeyRecDump(pzKeyStr, dwDatFile));
	    D(eKeyDatDump(pstDBx->fdData, dwDatFile));
	}
	/* else (Astat == A_NOTFOUND) : ignore key not found in key */

    } /* END LOOP <while eKeyBufScan()==OK>, ie. while more in key-list. */

    /* Now eKeyBufScan() != OK; - if not <pzKeyStr> exhausted, return ERROR */
    KRET_ERR((Kstat != K_EOL), Kstat, 604)


    /* 3: Set total number of dataoffsets currently used in cache */
    pstDBx->stCache.dwCused = dwKeyCount;

    /* Trace cache contents (but don't print the data records) */
    D(eKeyCacheDump(pstDBx, FALSE));


    /* Function complete : return ok */
    STCK("eKeyCacheFill");
    KRET_OK

} /* END function eKeyCacheFill() */



/*+2 MODULE KEY.C ==========================================================*/
/*   NAME   07                    eKeyCacheSearch                           */
/*== SYNOPSIS ==============================================================*/
#define    MAXPAT  128     /* Max. allowed length of pattern string */

PRIVATE   eRetType
eKeyCacheSearch(pstDBx, pstBF)
    DBASE    *pstDBx;      /* Pointer to DBASE structure w. cache to search */
    BUFFER   *pstBF;       /* Pointer to BUFFER struct. with search pattern */
{
/* DESCRIPTION
 *    Uses an existing cache-array (pstDBx->stCache) of datafile record-offsets
 *    to retrieve a sequence of data-records one at a time, and scan each record
 *    for a search expression/pattern passed in the buffer <pstBF>.
 *    Each time the pattern is NOT found, the cache-entry is tagged, and after
 *    the cache has been fully searched, it is "compressed" by removing all
 *    tagged entries (ie. a "lazy delete").
 *    The function returns an updated cache containing only the offsets of
 *    those records, with a content "matching" the search expression.
 *
 *    1: INITIALIZE search machine
 *
 *    2: LOOP: RETRIEVE & SEARCH ...
 *       For all offsets in current cache : 
 *       2.1: Get offset of next datarecord from cache array
 *       2.2: Call eKeyBufRead() to read datarecord from file to buffer
 *       2.3: Search datarecord for pattern, - return TRUE if match
 *       2.4: If not match, - tag current cache slot ("lazy delete")
 *
 *    3: COMPRESS cache ...
 *       3.1: Scan through cache, removing each tagged entry
 *       3.2: Update cache count of used slots
 *
 * RETURN
 *    Side effects ........: Old cache compressed to hold only offsets for 
 *                           records with a content matching search pattern.
 *                           The cache struct. slotcount <dwCused> is updated.
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 *-2*/

    char      pzPat[MAXPAT+1];     /* String for retrieving pattern from buf. */
    DWORD     dwCindx  = 0L;       /* Index for looping through cache array */
    DWORD     dwCused  = 0L;       /* #slots currently used in cache array */
    DWORD     dwCoffs  = 0L;       /* Datafile Offset retrieved from cache */
    DWORD     dwCfree  = 0L;       /* Index of first free cache slot */ 	
    FLAG      fMatch   = FALSE;    /* Flag : match of pzKeyExpr<->datarecord */
    eRetType  eRetCode = ERROR;    /* Function return code */


    /* 1: INITIALIZE search machine */
    (void) strncpy(pzPat, pstBF->pzBaddr, MAXPAT);/* Save pattern from buffer */
    pzPat[MAXPAT] = '\0';                         /* Assure zero termination */
    (*pvBldSearch)((BYTE*)pzPat, strlen(pzPat));  /* Build search tables */


    /* 2: LOOP: RETRIEVE & SEARCH record, for all offsets in current cache */
    for( dwCindx = 1L, dwCused = pstDBx->stCache.dwCused;
         dwCindx <= dwCused; dwCindx++) {
      
       /* 2.1: Get offset of next datarecord from cache array */
       dwCoffs =  (*(pstDBx->stCache.padwData))[dwCindx];

       /* 2.2: Call eKeyBufRead() to read datarecord from file to buffer */
       eRetCode = eKeyBufRead(pstDBx->fdData, pstBF, &dwCoffs);
       KRET_ERR(eRetCode == ERROR, K_DATREAD, 700)

       /* 2.3: Search datarecord for pattern, - return TRUE if match */
       fMatch = (FLAG) (*piRunSearch)((BYTE*)(pstBF->pzBaddr), strlen(pstBF->pzBaddr));
       D(fprintf(stdout, "Search: %s\n\n", (fMatch ? "MATCH!" : "NO-MATCH"));)

	/* 2.4: If not match, - tag current cache slot */
	if (!fMatch)
	    (*(pstDBx->stCache.padwData))[dwCindx] = (DWORD) ULONG_MAX;

    }  /* END LOOP <for all offsets in cache> */


    /* 3: COMPRESS cache (ie. eliminate all tag'd entries) */
    D(eKeyCacheDump(pstDBx, FALSE);)

    /* 3.1: Scan through cache, removing each tagged entry */
    for( dwCindx = 1L, dwCfree = 0L;
         dwCindx <= dwCused; dwCindx++) {

       /* Retrieve datafile offset from cache (ULONG_MAX:delete tag) */
       dwCoffs = (*(pstDBx->stCache.padwData))[dwCindx];

       /* Hook in <dwCfree> on first tagged entry (to be deleted) */
       if (dwCoffs == (DWORD) ULONG_MAX && dwCfree == (DWORD) 0L)
           dwCfree = dwCindx;

       /* Move occupied entry up to first free slot, if any (step down free) */
       if (dwCoffs != (DWORD) ULONG_MAX && dwCfree > (DWORD) 0L)
	    (*(pstDBx->stCache.padwData))[dwCfree++] = dwCoffs;
    }

    /* 3.2: Update cache count of used slots */
    pstDBx->stCache.dwCused = (dwCfree > (DWORD) 0L ? dwCfree-1 : dwCused);
    D(eKeyCacheDump(pstDBx, FALSE);)

    /* Function complete : return ok */
    STCK("eKeyCacheSearch");
    KRET_OK

} /* END function eKeyCacheSearch() */



/*+3 MODULE KEY.C-----------------------------------------------------------*/
/*   NAME   08                 eKeyCacheAlloc                               */
/*-- SYNOPSIS --------------------------------------------------------------*/
#define INITSIZE   100L        /* Initial size of new cache array: MIN 2! */
#define PANIC      1.2         /* Fallback resize factor (bad malloc) */


PUBLIC    eRetType
eKeyCacheAlloc(pstCache, rResize)
    CACHE    *pstCache;        /* Pointer to cache datastructure */
    double    rResize;         /* Multiplication factor for resizing cache */
{
/* DESCRIPTION
 *    Allocate cache-array for struct. <pstCache> to hold datarecord-offsets.
 *
 *    1: If no array is defined for <pstCache>
 *            MALLOC it from scratch with a startsize of <INITSIZE>
 *    2: Else
 *       2.1: REALLOC it to new size determined by the factor <rResize>; 
 *       2.2: make it robust, using a fallback value of <rResize>.
 *    3: RESET cache structure variables: cache array-pointer & -size
 *
 * RETURN
 *    Side effects ........: Cache array (re)alloc'ed to specified size.
 *                           Cache struct. fields updated accordingly.
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 * SEE ALSO
 *    The function eKeyCacheFree() in module key.c.
 *-3*/

    DWORD(*p)[] = pstCache->padwData;  /* Pointer to old cache, if any */
    DWORD(*q)[] = NULL;                /* Pointer to new cache, if realloc */ 
    register DWORD dwNewSize = 0L;     /* Size of new cache (# slots) */


    /* 1: If no cache defined, ALLOCATE NEW array of size INITSIZE */
    if (p == NULL) {

	/* No cache, - allocate new of initial size INITSIZE */
	dwNewSize = INITSIZE;
	assert(dwNewSize > 2);   /* Ensure quadratic expansion on realloc */

	p = (DWORD((*)[])) malloc( (size_t) (sizeof(DWORD) * dwNewSize));
	KRET_ERR(p == NULL, K_BADALLOC, 800)
    }

    else {

    /* 2: Else (cache defined), REALLOCATE EXISTING array to oldsize*rResize */
    L_Again:

       /* 2.1: Active cache, - reallocate it using factor <rResize> */
       dwNewSize = (DWORD) (((double) pstCache->dwCsize) * rResize);
       KRET_ERR(dwNewSize <= (DWORD) 0L, K_BADARGS, 801)

       D(printf("eKeyCacheAlloc: dwCsize[%lu], rResize[%.2f], dwNewSize[%lu]\n",
                 pstCache->dwCsize, rResize, dwNewSize);)

       q = (DWORD((*)[])) realloc((void *) p, (size_t) (sizeof(DWORD) * dwNewSize));


       /* 2.2.a: Bad alloc, - retry with <rResize> reduced to factor PANIC */
       if (q == NULL && rResize > (double) PANIC) {
           rResize = (double) PANIC;
           goto L_Again;   /* Try once more (NOT considered harmfull...) */
       }

       /* 2.2.b: Realbad alloc, - bail out, but reset cache for next list */
       if (q == NULL) {
           (void) eKeyCacheFree(pstCache);
           KRET_ERR(q == (DWORD((*)[])) NULL, K_BADALLOC, 802) /* Bail out! */
       }

       /* 2.3: Realloc OK, point to resized cache */
       p = q;
    }


    /* 3: Reset cache structure variables: cache array-pointer & -size */
    pstCache->padwData = p;                /* New or resized cache */
    pstCache->dwCsize = dwNewSize - 1L;    /* Index range [1 ... dwNewSize] */
    D(printf("eKeyCacheAlloc: dwCsize[%lu], rResize[%.2f], dwNewSize[%lu]\n",
              pstCache->dwCsize, rResize, dwNewSize);)


    /* Function complete : return ok */
    STCK("eKeyCacheAlloc");
    KRET_OK

} /* END function eKeyCacheAlloc() */



/*+3 MODULE KEY.C ----------------------------------------------------------*/
/*   NAME   09                 eKeyCacheFree                                */
/*-- SYNOPSIS --------------------------------------------------------------*/
PUBLIC    eRetType
eKeyCacheFree(pstCache)
    CACHE    *pstCache;       /* Pointer to cache datastructure */
{
/*
 * DESCRIPTION
 *    Deallocate cache-array for struct <pstCache>.
 *
 *    1: FREE the cache-array of struct. <pstCache>.
 *    2: RESET the cache structure fields to NULL/0.
 *
 * RETURN
 *    Side effects ........: Cache array deallocated (ie. RAM free'd).
 *                           Cache struct. fields reset accordingly (NULL/0).
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 * SEE ALSO
 *    The function eKeyCacheAlloc() in module key.c
 *-3*/

    /* 1: FREE cache array */
    if (pstCache->padwData != NULL) {
	free((char *) pstCache->padwData);
	pstCache->padwData = NULL;
    }


    /* 2: RESET cache size to zero */
    pstCache->dwCsize = 0L;
    pstCache->dwCused = 0L;


    /* Function complete : return ok */
    STCK("eKeyCacheFree");
    KRET_OK

} /* END function eKeyCacheFree() */ 




/****************************************************************************/
/******************************** BUFFER ************************************/
/****************************************************************************/



/*+3 MODULE KEY.C-----------------------------------------------------------*/
/*   NAME   10                 eKeyBufScan                                  */
/*-- SYNOPSIS --------------------------------------------------------------*/
/*+0*/
/* Definition of KeySpec separators in KeyList */
#define KEYSEP     ','        /* Keylist keyspecification separator */
#define KEYEOL     '\0'       /* End-Of-List */

/* Definition of metachars for KeyClass and KeyRange specification */
#define KEYEXP     '-'        /* Keyspec. expansion indicator: range/class */
#define K_A        '*'        /* Metachar alfanumeric: digit | letter */
#define K_D        '#'        /* Metachar digit .....: [0-9] */
#define K_L        '@'        /* Metachar letter.....: [a-�A-�] */

/* Definition of character collating sequence for expansion of KeySpec's */
static const char acAlnum[] =
   "0123456789abcdefghijklmnopqrstuvwxyz���ABCDEFGHIJKLMNOPQRSTUVWXYZ���";

#define CHRPOS(c)  strchr(acAlnum, (int) c)
#define KEYDGT(i)  ( (acAlnum <= pB[i]) && (pE[i] <= acAlnum+9) )

/* Declaration of static arrays for expansion of KeySpec's */
static char *pB[KEYMAX];       /* Ptrs into acAlnum, for start KeyValue */
static char *pW[KEYMAX];       /* Ptrs into acAlnum, for work KeyValue */
static char *pE[KEYMAX];       /* Ptrs into acAlnum, for end KeyValue */
static int iCB[KEYMAX];        /* #Decimal "clicks", for start KeyValue */
static int iCW[KEYMAX];        /* #Decimal "clicks", for work KeyValue */

#define EXPDEC		       /* Uncomment to enable decimal interpretation */
/*-0*/

PRIVATE   eRetType
eKeyBufScan(pstDBx, ppzKeyList, ppzKeyStr)
    DBASE    *pstDBx;         /* Pointer to DBASE structure (w. key size) */
    char     **ppzKeyList;    /* Addr. of ptr. for walking the key-list */
    char     **ppzKeyStr;     /* Addr. of ptr. for returning retrieved key */
                              /* NB: *ppzKeyStr MUST be NULL on first call */
{
/* DESCRIPTION
 *    Scans a string <*ppzKeyList> containing one (or more, comma-separated)
 *    key specifications, and sets the stringpointer <*ppzKeyStr> on the
 *    substring of the next key-value to retrieve; The substring is zero-
 *    terminated before return from eKeyBufScan().
 *
 *    The precise SYNTAX of <*ppzKeyList> is as follows, where :
 *       {} means 0-more repetitions, | means alternative expressions, and
 *       a blank character indicates contatenation (NB: no whitespace allowed)
 *       *ppzKeyList ::=       {KeySpec,} KeySpec KEYEOL
 *       KeySpec     ::=   |   KeyValue                    // one key value
 *                         |   KeyValue KEYEXP KeyValue    // a key range
 *                             KeyClass KEYEXP             // a key class
 *       KeyValue    ::=       a string of exactly KeyLen
 *                             of arbitrary ASCII chars or numeric chars
 *                             depending on access method (VA, SS, ...)
 *       KeyClass    ::=       a KeyValye with possible embedded metachars :
 *            K_A    ::=       alphanumeric :  digit | letter
 *            K_D    ::=       digit        :  [0-9]
 *            K_L    ::=       letter       :  [a-�A-�]
 *
 *    Example of a valid keylist : "20240,20259,20713-20772,649#3-,01267"
 *       20240,20259 : two single KeyValues
 *       20713-20772 : a range of KeyValues : 20713,20714,20715, ... ,20772
 *       649#3-      : a class of KeyValues : 64903,64913,64923, ... ,64993
 *       01267       : one more single KeyValue
 *
 * RETURN
 *    Side effects ........: *ppzKeyStr pointed at start of next key-substring
 *                           in *ppzKeyList, and key-substring zero-terminated
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 *                           (End-Of-Line is treated as error with code K_EOL)
 * BUGS
 *    The scanner algorithm for the keylist-string is rather "unforgiving" :
 *    each KeyValue MUST be specified with precise length (no attempt of error
 *    recovery is made such as "patching" a too short key or "chopping" a too
 *    long key to the required length), and an incorrect key will result in
 *    immediate "abortion" of the KeyList scanning (w. error code K_BADLIST).
 *    No escape mechanism is provided for the KeyClass metachars.
 *
 *    Some key access methods (such as scatter-storage : SS/"hashing") is
 *    not well suited for sequential access. I have never the less provided a
 *    syntax for specifying a Range or Class of KeyValues in the KeyList
 *    parameter, - but BE WARNED : in a specification expanding to many (more
 *    than 100) KeyValues, the key lookup-operations required to setup the
 *    cache-array may consume a substantial amount of time (for a SS-key:
 *    6-12sec/100 keys, depending on key "hit rate" and machine type).
 *+0
 * IMPLEMENTATION
 *    The expansion of a KeyClass or KeyRange is done by the following means
 *    (which is by no means trivial ...) :
 *     - a boolean variable (fKeyExpand) is toggled ON, when a KeyClass or
 *       KeyRange is first met in the sequential scanning of the KeyList;
 *       the first KeyValue in the expansion is returned to the caller.
 *     - a "trap" is set up at the start of the function to "catch" the
 *       flow of control at subsequent calls, whenever fKeyExpand is ON;
 *       Inside the "trap" I generate the next KeyValue in the defined
 *       class/range, test if that value completes the expansion (in which
 *       case I toggle fKeyExpand OFF), and return the KeyValue to the caller.
 *
 *    In order to remember the state of expansion for all character-positions
 *    in the key BETWEEN function calls, I define 3 STATIC arrays to hold the
 *    begin-character pB[i], end-character pE[i] and current "work"-character
 *    pW[i] for the expansion of each position i in the KeyClass/KeyRange
 *    specification.
 *
 *    Expansion of digit-sequence(s) in a KeyRange can be handled in two ways:
 *     - EXPDEC not #defined : ordinary character expansion, with no signifi-
 *       cance attached to the positions of the digits.
 *       Example 15-29 -> [15,16,17,18,19,25,26,27,28,29]
 *     - EXPDEC is  #defined : decimal character expansion, ie. each digit
 *       expanded according to its position in the digit-sequence. This is
 *       probably what you want, so I have made this option default.
 *       Example 15-29 -> [15,16,17,18,19,20,21,22,23,24,25,26,27,28,29]
 *
 *    For the purpose of decimal character expansion, I define a new STATIC
 *    array iCB[i] to hold the number of "clicks" (ie. [0-9] runs) to "turn"
 *    each digit position i for a full expansion. This in effect is a small
 *    analog "difference engine" with symbolic gear-wheels for the decimal
 *    addition operation (Charles Babbage, personal communication...).
 *    Finally, for handling multiple digital runs embedded in an alphabetic
 *    KeyRange, I introduce the STATIC "work" array iCW[i], which is reset
 *    to iCB[i] each time we hit an alphabetic position in the key-expansion.
 *
 *    This design is an ultimate consequence of my wish to treat a KeyValue
 *    as an arbitrary string composed of digits and/or alphabetic characters,
 *    so I accept the complexity of key-expansion in the name of generality
 *    (- and besides : it was fun to "crack" the problem !).
 *-0
 *-3*/

    /* Setup length of actual KeyList and KeyValue */
    WORD        wListLen  = strlen(*ppzKeyList); /* Length of keylist string */
    WORD        wKeyLen   = 0;         /* Length of one key in <pzKeyList> */
    char        *pcKeySep = NULL;      /* Ptr's to delimit of KeyValue */
    char        *pc       = NULL;      /* Ptr's to delimit of KeyValue */
    register    int i=0, j=0;          /* Scratch int : count variables */

    /* Variables for KeyValue expansion */
    static FLAG fKeyExpand = FALSE;    /* Boolean: expansion? */
    FLAG        fDigits    = FALSE;    /* Boolean: digit-range? */
    char        *pEnd      = NULL;     /* Ptr to end char of expansion */
    char        cb=0, ce=0, cw=0;      /* Scratch char: begin, end, work */



    /*----------------------------------------------------------------------*/
    /* 0: Initialize variables                                              */
    /* Define fixed keylength for parse of the key-list: <ppzKeyStr>        */
    /* BUG: we should allow var-length keystring, relying on key separators */
#ifdef SS
    /* SS-key: Keylength included as field in key-struct */
    wKeyLen = pstDBx->pIndex->indexsize.wKsize;
#endif /*SS*/
#ifdef VA
    /* VA-key: Assume fixed default keylength (might include MAX in VA-struct) */
    wKeyLen = KEYLEN;
#endif /*VA*/
    assert(wKeyLen > 0);


    /*----------------------------------------------------------------------*/
    /* 1: Trap flow-of-control when inside KeyClass- or KeyRange-expansion; */
    /* - The first key in an expansion is generated in KEYEXP-switch below. */
    /* - The following keys are genereted by this key-expansion codeblock.  */
    /* - When expansion complete, fKeyExpand<-OFF and cntl "falls through". */

    /* Reset trap, when function called with new keylist! */
    if (*ppzKeyStr == NULL)
       fKeyExpand = FALSE;

    /* If trap active (lookout: here the bits hit the fan...) */
    if (fKeyExpand) {

       /* Step through key pointer-arrays: left<-right */
       for (i = wKeyLen - 1; i >= 0; i--) {

           /* At pos i: step work-ptr pW[i] from pB[i] to pEnd in acAlnum[] */
           pEnd = (iCW[i] > 0 ? CHRPOS('9') : pE[i]);	/* click defined? */
           if (pW[i] < pEnd) {

              /* Increment work-ptr at pos i & xfer char acAlnum->KeyValue */
              /* If char xfer'd was '9', count down #clicks for position i */
              *(*ppzKeyStr + i) = *(++(pW[i]));
              if (pW[i] == CHRPOS('9') && iCW[i] > 0)
                 iCW[i]--;

              /* Reset KeyValue & click-array in all pos. j to the right of i */
              for (j = i + 1, fDigits = KEYDGT(i); (WORD) j < wKeyLen; j++) {
                 fDigits = (fDigits && KEYDGT(j));
                 pW[j] = (iCW[j] >= 0 && fDigits ? CHRPOS('0') : pB[j]);
                 *(*ppzKeyStr + j) = *pW[j];
                 iCW[j] = (!fDigits ? iCB[j] : iCW[j]);
              }

              /* Key now generated, - break enclosing for-loop! */
              D(printf("Expanded KeyValue : [%s]\n", *ppzKeyStr));
              break;
           }
           /* else (pW[i] >= pE[i]): range at pos i already done! */
           /* Proceed with KeyValue[i-1], until KeyValue[0] */
       }

       /* If new KeyValue generated (ie: i < 0), return it to caller */ 
       fKeyExpand = (i < 0 ? FALSE : TRUE);
       if (fKeyExpand)
           KRET_OK

       /* Else continue processing KeyList : "Fall Through" to 2: */
       /* NB: ppzKeyStr already advanced in KEYEXP-switch below ! */

    } /* END [1]: Key Expansion active */



    /*----------------------------------------------------------------------*/
    /* 2: Key expansion NOT active, retrieve next key directly from KeyList */

    /* Check for KeyList exhausted */
    KRET_ERR((**ppzKeyList == KEYEOL), K_EOL, 1000)

    /* Reset decimal "click" arrays */
    for (i = 0; (WORD) i < wKeyLen; i++)
	iCB[i] = iCW[i] = -1;

    /* Point past end of next key, and check key length  */
    pcKeySep = *ppzKeyList + wKeyLen;
    KRET_ERR((pcKeySep > (*ppzKeyList + wListLen)), K_BADLIST, 1001)


    /* Setup pointer to retrieve the next key... */
    switch (*pcKeySep) {

	case KEYEXP:            /* A KEY EXPANSION */
	    /* --------------------------------------------------------------*/
	    /* Activate *KEYCLASS* expansion, ie. <KeyClass KEYEXP>          */

	    /* Check valid format of KeyClass specification */
           if ((*(pcKeySep + 1) == KEYSEP || *(pcKeySep + 1) == KEYEOL)) {

              /* Setup arrays of char-pointers for generating key class */
              for (i = 0; (WORD) i < wKeyLen; i++) {	/* Scan key left->right */

                 /* Set Begin-char and End-char for KeyValue in position i */
                 cw = *(*ppzKeyList + i);	/* Retrieve next key-char */
                 cb = (char) (cw == K_D ? '0' : (cw == K_L ? 'a' : (cw == K_A ? '0' : cw)));
                 ce = (char) (cw == K_D ? '9' : (cw == K_L ? '�' : (cw == K_A ? '�' : cw)));

                 /* Set Begin-, Work- and End-ptr. for KeyValue in pos. i */
                 pB[i] = pW[i] = CHRPOS(cb);
                 pE[i] = CHRPOS(ce);

                 /* Reset KeyValue in KeyList to beginning value */
                 if (pW[i] != NULL)
                    *(*ppzKeyList + i) = *pW[i];

              } /* END Setup c-arrays */

              /* Set pointer past KeyClass */
              pc = pcKeySep + 1;

           } /* END activate *KEYCLASS* expansion */

           else {
           /* --------------------------------------------------------------*/
           /* Activate *KEYRANGE* expansion, ie. <KeyValue KEYEXP KeyValue> */
                        
              /* Set ptr. past KeyRange & Check valid format of KeyRange spec. */
              pc = pcKeySep + wKeyLen + 1;
              KRET_ERR(*pc != KEYSEP && *pc != KEYEOL, K_BADLIST, 1002)
              KRET_ERR(pc > (*ppzKeyList + wListLen), K_BADLIST, 1003)

              /* Set Begin-, Work- and End- charptr. arrays for KeyValue */
              for (i = 0; (WORD) i < wKeyLen; i++) {
                 pB[i] = pW[i] = CHRPOS(*(*ppzKeyList + i));
                 pE[i] = CHRPOS(*(pcKeySep + 1 + i));
              }

#ifdef EXPDEC
              /* If decimal interpretation of digit sequence is enabled : */
              /* Set decimal "Click-count" for all digital (sub-)range(s) */
              for (i = 0; (WORD) i < wKeyLen; i++) {

                 if (KEYDGT(i) && pB[i] != pE[i]) {

                    /* Invalid KeyRange (begin > end) : set end = begin */
                    if (pB[i] > pE[i])
                       while (((WORD)++ i < wKeyLen) && KEYDGT(i))
                          pE[i] = pB[i];
                    else
                       /* Valid KeyRange : setup click-count arrays */
                       /* Step i through one digital (sub)range */
                       for (iCW[i]=iCB[i]=0, j=i+1; (WORD) i < wKeyLen && KEYDGT(i); i++, j++) {

                          /* Set click-counter for next pos (j) in the digital (sub)range */
                          if ( (WORD)j < wKeyLen && KEYDGT(j) ) {
                             iCW[j] = iCB[j] = iCB[i] * 10 + (pE[i]-pB[i]);           
                             D(printf("   Pos %d:[%c]->Pos %d:[%c] : Click[%d]\n",
                                i, *pB[i], j, *pE[j], iCW[j]));
                          } /* END j: calculate click for pos.j(=i+1), based at i */

                       } /* END i: calculate clicks for one (sub)range (stepping i) */

                 } /* END if KEYDGT(i) */
                 /* else: not digits or equal digits, - ignore */

              } /* END for i in [0,wKeyLen-1] */
#endif	/* EXPDEC */

           } /* END Activate *KEYRANGE* expansion */


	    /* ------------------------------------------------------------- */
	    /* KeySpec expansion (*KEYCLASS* and *KEYRANGE*)                 */

	    /* Setup for return of first KeyValue in key-expansion */
	    *pcKeySep  = '\0';          /* Zero-term. KeyValue string */
	    *ppzKeyStr = *ppzKeyList;   /* Return 1. KeyValue in KeyExp */

	    /* Setup for processing rest of KeyList */
	    *ppzKeyList = pc;           /* Point past KeyExp */
	    *ppzKeyList += (**ppzKeyList == KEYSEP ? 1 : 0);

	    D(printf("Expanded KeyValue : [%s]\n", *ppzKeyStr));
	    fKeyExpand = TRUE;          /* Raise KeyExpansion flag */
	    break;


	case KEYSEP:            /* ONE VALID KEY (comma separated) */
	    /* Setup for return of current KeyValue */
	    *pcKeySep  = '\0';          /* Zero-term. KeyValue string */
	    *ppzKeyStr = *ppzKeyList;   /* Return this key string segment */

	    /* Setup for processing rest of KeyList */
	    *ppzKeyList = (pcKeySep+1); /* Point past KeyValue */
	    break;


	case KEYEOL:            /* LAST KEY (zero-terminated) */
	    /* Setup for return of last KeyValue, - already zero-terminated */
	    *ppzKeyStr = *ppzKeyList;   /* Return this key string segment */

	    /* Setup for processing rest of KeyList */
	    *ppzKeyList = pcKeySep;     /* Point past KeyValue, - to K_EOL */
	    break;


	default:                /* BAD KEY-LIST FORMAT */
	    D(fprintf(stderr, "BAD KEYLIST FORMAT ->[%s]\n", *ppzKeyList));
	    KRET_ERR(YES, K_BADLIST, 1004)      /*CONSTVAL*/
	    break;

    } /* END switch (*pcKeySep): setup ptr. <*ppzKeyStr> to retrieve key */


    /* Function complete : return ok */
    STCK("eKeyBufScan");
    KRET_OK

} /* END function eKeyBufScan () */



/*+2 MODULE KEY.C ==========================================================*/
/*   NAME   11                 eKeyBufRead                                  */
/*== SYNOPSIS ==============================================================*/
PRIVATE eRetType
eKeyBufRead(fdData, pstBF, pdwOff)
    FILE     *fdData;         /* File pointer for datafile */
    BUFFER   *pstBF;          /* Ptr. to BUFFER structure for read */
    DWORD    *pdwOff;         /* Offset of record to read */
{
/* DESCRIPTION
 *   Read ONE datarecord from the datafile identified by file pointer <fdData>
 *   to the buffer specified by <pstBF>; Record to be read depends on <pswOff>:
 *    - *pdwOff != NEXT : Retrieve specified datarecord (file offset: <pdwOff>)
 *    - *pdwOff  = NEXT : Retrieve next datarecord (from current datafile pos)
 *
 *   1: RESET...
 *      1.1: RESET DATAFILE pos., according to <pdwOff> (catch EOF on datafile)
 *      1.2: RESET BUFFER pos. & size (to empty buffer)
 *   2: LOOP:
 *         read one line from datafile to buffer
 *         update current buffer position & size
 *      until (KEYMARK of next rec or EOF)
 *
 * RETURN
 *    Side effects ........: One datarecord specified by: <fdData>,<pdwOff>
 *                           read to buffer <pstBF>.
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 *-2*/
    register char *pcBinp   = NULL;    /* BUF: current buffer input pointer */
    register WORD wBsiz     = 0L;      /* BUF: current size of buffer (#byte left) */
    WORD          wBLlen    = 0L;      /* BUF: length of current buffer Line */
    int           iRetCode  = 1;       /* fseek() ret.code: 0=OK, !0=ERR */
    char         *pcRetCode = NULL;    /* fgets() ret.code: !NULL=OK, NULL=ERR */
    BYTE          bPeek     = (BYTE)0; /* char obtained by "peek" on stdin */


   /* 1.1: RESET DATAFILE position, according to <pdwOff> */
   if (*pdwOff != NEXT) {           /* Read from specified file offset */
      iRetCode = fseek(fdData, *pdwOff, SEEK_SET);
      KRET_ERR(iRetCode != 0, K_DATSEEK, 1100)
   } else                          /* Read from current file position */
      *pdwOff = ftell(fdData);    

   /* Catch EOF on datafile */
   KRET_ERR(feof(fdData), K_DATEOF, 1101)


   /* 1.2: RESET BUFFER pointer & size (to empty buffer) */
   pcBinp = pstBF->pzBaddr;     /* Init buffer input-ptr to buf.start */
   wBsiz  = pstBF->wBsize;      /* Init rest size to full buffer size */


    /* 2: LOOP: READ record lines (until KEYMARK of new rec or EOF) */
    do {
       /* Read one line from datafile to buffer */
       pcRetCode = fgets(pcBinp, wBsiz, fdData);
       KRET_ERR(pcRetCode == NULL && !fdData, K_DATREAD, 1102)

       /* Update current buffer pos. & size (advance pos., decrement size) */
       wBLlen = strlen(pcBinp);  /* Length of rec line, excl. term. '\0' */
       pcBinp += wBLlen;         /* Advance to terminal '\0' from fgets */
       wBsiz  -= wBLlen;         /* Count down buffer size */
       D(printf("TRACE: wBsiz[%d]\n", (int) wBsiz));
       KRET_ERR(wBsiz <=1, K_BUFOVFL, 1103)

       /* LookAhead: Peek next char */
       KRET_ERR(ungetc( (bPeek = (BYTE) getc(fdData)), fdData) == EOF, K_DATREAD, 1104)

    } while (bPeek != (BYTE) KEYMARK && !feof(fdData));
    /* Until next record || EOF */


   /* Debug trace */
   D(fputs("\nECHO DATA RECORD :\n", stdout));
   D(fputs(pstBF->pzBaddr, stdout));

   /* Function complete : return ok */
   STCK("eKeyBufRead");
   KRET_OK

} /* END function eKeyBufRead() */



/*+2 MODULE KEY.C ==========================================================*/
/*   NAME   12                    eKeyBufFill                               */
/*== SYNOPSIS ==============================================================*/
PRIVATE   eRetType
eKeyBufFill(pstDBx, pstBF, dwCbwinNew)
    DBASE    *pstDBx;         /* Handle of database-str. w. datafile */
    BUFFER   *pstBF;          /* Handle of buffer-str. w. bufferarea */
    DWORD    (*dwCbwinNew)[]; /* New window into cache array */
{
/* DESCRIPTION
 *    Fill the databuffer (address:pstBF->pzBaddr, length:pstBF->wBsize byte)
 *    with SEVERAL records read from the datafile (handle:pstDBx->fdData).
 *    The records to retrieve are specified by the pstDBx->stCache datastruct.
 *     - stCache.padwData[] holding an array of cach'ed datafile offsets
 *       (previously set up by : eKeyCacheFill() or eKeyCacheSearch())
 *     - stCache.dwCbwin[2] defining the range of cache-key'es currently
 *       held by the buffer : the old "buffer-window".
 *     - parameter <cCbwinNew[2]> defining the range of cache-key'es to be
 *       fill'ed into the buffer by eKeyBufFill() : the new "buffer-window".
 *
 *    1:   Set up shorthand pointers for easy access to cache buffer-windows.
 *
 *    2:   Retrieve all datarecords defined by the new cache buffer-window :
 *         LOOP FOR ALL slots in the buffer-window <dwCbwinNew[2]> ...
 *    2.1:    Set the datafile-pointer to the next cach'ed datafile-offset
 *    2.2:    Read the datarec. to the databuffer, in bursts of BLKSIZ chars :
 *            LOOP FOR EACH datarecord in the datafile <pstSBx->fdData>
 *    2.2.1:     Setup next blocksize (BLKSIZ, or less if buffer nearly full)
 *    2.2.2:     Read the block & check for completed record (KEYMARK present)
 *    2.2.3:     Move the buffer-readpointer to position after the block read
 *            WHILE (datarecord not complete AND more space in buffer)
 *         WHILE more slots in the new cache buffer-window
 *
 *    3:   If incomplete last datarecord, back up to previous KEYMARK in buffer.
 *    4:   Zero-terminate the datarecords, and clear to the buffer end.
 *    5:   Update the "old" cache buf.window to the datarec's now in the buffer.
 *
 * RETURN
 *    Side effects ........: Buffer pstBF->pzBaddr fill'd in with datarecords
 *                           (up to max pstBF->wBsize chars), according to
 *                           the specified new bufferwindow <dwCbwinNew[2]>.
 *                           The cache struct. bufferwindow dwCbwin[2] updated.
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 *
 * BUGS
 *    The bufferfilling algorithm is rather simple minded as regards scrolling:
 *    we always perform a complete refill of the buffer without utilizing the
 *    possibility of moving already retrieved datarecords in RAM instead of
 *    re-reading them from the datafile on DISK.
 *    To implement a more intelligent bufferfilling algorithm, you must main-
 *    tain a list of offsets into the databuffer for each datarecord currently
 *    in the cache (cf. the "old" buffer-window).
 *-2*/

    D(DWORD  dwOld[2];)                /* CACHE: buf.window, OLD (DEBUG) */
    DWORD    dwNew[2];                 /* CACHE: buf.window, NEW */
    register DWORD dwNext = 0L;        /* CACHE: next cache slot for read-to-buf. */
    char    *pBF0    = pstBF->pzBaddr; /* BUFFER: start-of-datarecord ptr. */
    char    *pBF1    = pstBF->pzBaddr; /* BUFFER: start-of-datablock ptr. */
    char    *pBF2    = NULL;           /* BUFFER: end-of-datablock ptr. */
    WORD     wBFRest = 0L;             /* BUFFER: #byte left in databuffer */
    WORD     wBFBlock= 0L;             /* BUFFER: current blocksize to read */
    FILE    *fdDataFile=pstDBx->fdData;/* DATA: datafile handle */
    FLAG     fDone   = FALSE;          /* Boolean: TRUE when datarecord read */
    int      iSkip;                    /* [0|1] : include or skip KEYMARK */
    eRetType   eRetCode = ERROR;       /* Return code for key.c functions */
    eKeyStatus eScrCode = K_OK;        /* Return code for eKeyBufFill() */


    /* 1: Set up shorthand pointers for convenient access to cache buf.win */
    /* Get old buffer window (for DEBUG trace) */
    D(dwOld[0] = pstDBx->stCache.dwCbwin[0];)
    D(dwOld[1] = pstDBx->stCache.dwCbwin[1];)

    /* Get new buffer window, and assert range "in bounds" */
    dwNew[0] = (*dwCbwinNew)[0];
    dwNew[1] = (*dwCbwinNew)[1];
    assert(dwNew[0] >= 1L);
    assert(dwNew[1] >= dwNew[0]);
    assert(dwNew[1] <= pstDBx->stCache.dwCused);

    D(fprintf(stdout, "BufFil:\tTOP old[%lu]->new[%lu]\n\tBOT old[%lu]->new[%lu]\n",
                       dwOld[0], dwNew[0], dwOld[1], dwNew[1]));

    /* 2: LOOP through ALL slots in the new cache buffer-window ... */
    for ( dwNext = dwNew[0], fDone = FALSE, wBFRest = (WORD) 2;
          dwNext <= dwNew[1] && wBFRest > (WORD) 1;
          dwNext++, fDone = FALSE ) {

	/* 2.1: Set filepointer on datarecord, using fileoffset in the cache */
	eRetCode = (fseek(fdDataFile, (*(pstDBx->stCache.padwData))[dwNext], SEEK_SET)
		    == 0 ? OK : ERROR);
	KRET_ERR(eRetCode == ERROR, K_DATSEEK, 1200)

	/* 2.2: LOOP Read ONE datarec. to buffer, in bursts of BLKSIZ chars */
	pBF0  = pBF1;                   /* remember start of datarecord */
	iSkip = 1;                      /* skip 1. KEYMARK in datarecord */
	do {

           /* 2.2.1: Setup next BLKSIZ block to read */
           wBFRest = pstBF->wBsize - (pBF1 - pstBF->pzBaddr);
           wBFBlock = (wBFRest > BLKSIZ ? BLKSIZ : wBFRest - 1);
           D(fprintf(stdout, "wBFrest[%d] - wBFBlock[%d]\n", wBFRest, wBFBlock));

           /* 2.2.2: Read the block & check if that completes the record read */
           if (wBFBlock > (WORD) 0) {

              /* Read next block of the datarecord from file */
              clearerr(fdDataFile);
              wBFBlock = (WORD) fread(pBF1, sizeof(char), wBFBlock, fdDataFile);

              /* Check for read-error and EOF */
              KRET_ERR(ferror(fdDataFile), K_DATREAD, 1201)
              if (feof(fdDataFile))
                 fDone = TRUE;

              /* Check if this block contains a KEYMARK (ie. read DONE!) */
              /* NOTE : memchr() interprets its args to "unsigned char" */
              if ((pBF2 = memchr(pBF1 + iSkip, (unsigned char) KEYMARK, wBFBlock)) != (char *) NULL) {
                wBFBlock = pBF2 - pBF1;	/* usable part of the block */
                fDone = TRUE;	            /* this datarecord finished */
	       }
	    }

	    /* 2.2.3: Move buffer readpointer to position after block read */
	    pBF1 += wBFBlock;
	    iSkip = 0;

	} while (!fDone && (wBFRest > 1));	/* reserve 1 byte for '\0' */
       /* END LOOP [2.2]: Block-Read of ONE datarecord */

    } /* END LOOP [2]: for #slots in cache buffer window (ie. #datarecs.) */
    /*   NB: Note the following conditions:                    */
    /* - inside the for-loop   : dwNext in dwNew[0]...dwNew[1] */
    /* - at exit (read succes) : dwNext =  dwNew[1]+1          */
    /* - at exit (buf. overfl) : dwNext =  cacheslot of incomplete last read+1 */


    /* 3: If incomplete last datarecord, back up to previous cacheslot/record */
    if (wBFRest == 1 && !fDone) {
       dwNext--;                   /* reset to slot AFTER last OK-read */
       pBF1 = pBF0;                /* reset to end of prev. record */
       eScrCode = K_BUFOVFL;       /* raise errorflag ("overflow!") */
    }


    /* 4: Zero-terminate the datarecords, and clear to buffer end ... */
    /* Terminate datarecords in the buffer */
    *pBF1++ = '\0';

    /* Clear from end of datarecords to buffer end */
    wBFRest = pstBF->wBsize - (pBF1 - pstBF->pzBaddr) - 1;
    if (wBFRest > 1) {
	(void) memset(pBF1, ' ', wBFRest);
	D((void) memset(pBF1, '*', wBFRest));
    }

    /* Terminate whole buffer (defensive programming) */
    *(pstBF->pzBaddr + pstBF->wBsize - 1) = '\0';


    /* 5: Finally update the cache bufferwindow */
    pstDBx->stCache.dwCbwin[0] = dwNew[0];
    pstDBx->stCache.dwCbwin[1] = dwNext - 1;


    /* Return : OK if buffer filled, else ERROR */
    STCK("eKeyBufFill");
    KRET_ERR(eScrCode != K_OK, eScrCode, 1202)
    KRET_OK

} /* END function eKeyBufFill() */




/****************************************************************************/
/******************************** UTILITY ***********************************/
/****************************************************************************/

#ifdef DEBUG   /* Matching #endif at end-of-file; -DEBUG-DEBUG-DEBUG-DEBUG- */


/*+3 MODULE KEY.C ----------------------------------------------------------*/
/*   NAME   13                    eKeyDBDump                                */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   eRetType
eKeyDBDump(pstDBx, pstBF)
    DBASE    *pstDBx;         /* Pointer to DBASE structure to dump */
    BUFFER   *pstBF;          /* Pointer to BUFFER structure to dump */
{
/* DESCRIPTION
 *    Dump all incore descriptors of a "database" (DBASE + associated BUFFER),
 *    Obs.: the dump routines are only active during DEBUG (compile switch).
 *
 *    Print header and ...
 *    1: Dump the key structure, handle <pstDBx->pIndex>
 *    2: Dump the cache-array, handle <pstDBx->stCache>
 *    3: Dump the buffer area, handle <pstBF->pzBaddr>
 *
 * RETURN
 *    Side effects ........: Control structs. of a "database" dumped on stdout.
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 * SEE ALSO
 *    The functions eKeyCacheDump(), eKeyIndexDump(), eKeyBufDump(), in key.c
 *-3*/
    eRetType eRetCode = ERROR;         /* Return code for key.c functions */

    /* 0: Print header */
    fputs("\n########## DUMP OF DBASE ##########\n", stdout);

    /* 1: Dump key control information */
    eRetCode = eKeyIndexDump(pstDBx);
    KRET_ERR(eRetCode == ERROR, Kstat, 1300)

    /* 2: Dump cache array contents (but NOT the datarecords) */
    eRetCode = eKeyCacheDump(pstDBx, FALSE);
    KRET_ERR(eRetCode == ERROR, Kstat, 1301)

    /* 3: Dump buffer contents (ONLY the null terminated data-area) */
    eRetCode = eKeyBufDump(pstBF, FALSE);
    KRET_ERR(eRetCode == ERROR, Kstat, 1302)


    /* Function complete : return ok */
    STCK("eKeyBufFill");
    KRET_OK

} /* END function eKeyDBDump() */



/*+3 MODULE KEY.C ----------------------------------------------------------*/
/*   NAME   14                    eKeyIndexDump                             */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   eRetType
eKeyIndexDump(pstDBx)
    DBASE    *pstDBx;         /* Pointer to DBASE structure w. key */
{
/* DESCRIPTION
 *    Dump the info. contained in a DBASE key-descr. <pstDBx->pIndex>.
 *
 *    Print header and ...
 *    1: Dump state.....: availability [open|closed], access-mode [RdWr|RdOnly],
 *    2: Dump id & size : name of keyfile, size of key and key
 *
 * RETURN
 *    Side effects ........: Control structs. of a "DBASE" dumped on stdout.
 *    Function return value: OK (operation succeeded)
 *
 * BUG
 *    Function pt. only defined for key of type SS.
 *-3*/

    /* 0: Print header */
    fputs("\n========== DUMP OF INDEX ==========\n", stdout);

#ifdef SS
    /* 1: Dump key open-state and open-mode */
    fprintf(stdout, "INDEX:\topenstatus[%s]\n",
	    pstDBx->pIndex->indexstatus == ICLOSED ? "ICLOSED" : "IOPEN");

    if (pstDBx->pIndex->indexstatus == IOPEN)
	fprintf(stdout, "\topenmode[%s]\n",
		pstDBx->pIndex->indexmode == RW ? "RW" : "RO");


    /* 2: Dump key name and size-info */
    fprintf(stdout, "\tfilename[%s]", pstDBx->pIndex->filename);
    fprintf(stdout, "\tsize: key[%d] key[%lu] used[%lu]\n",
            pstDBx->pIndex->indexsize.wKsize,      /* #chars in keystring */
            pstDBx->pIndex->indexsize.dwIsize,     /* #rec's total */
            pstDBx->pIndex->indexsize.dwIused );   /* #rec's used */
#endif /* SS */


    /* Function complete : return ok */
    STCK("eKeyIndexDump");
    KRET_OK

} /* END function eKeyIndexDump() */



/*+3 MODULE KEY.C ----------------------------------------------------------*/
/*   NAME   15                    eKeyCacheDump                             */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   eRetType
eKeyCacheDump(pstDBx, fAll)
    DBASE    *pstDBx;         /* Pointer to DBASE structure, with cache */
    int      fAll;            /* Dump contents of datarec's: [TRUE|FALSE] */
{
/* DESCRIPTION
 *    Dump the info. contained in a DBASE cache-descriptor <pstDBx->stCache>.
 *
 *    1: Initialize status variables for dump (cache length).
 *    2: Dump cache to stdout: print header & dump cache contents ...
 *       2.1: Catch & report empty buffer; otherwise 
 *       2.2: Cache contains data; - LOOP through cache pos. [1-dwCused] and:
 *            2.2.1: print the datafile record-offset stored in each slot,
 *            2.2.2: if fAll==TRUE, print content of datafile record as well.
 * RETURN  
 *    Side effects ........: Contents of <pstDBx->stCache> dumped on stdout.
 *    Function return value: OK if operation succeeded, ERROR otherwise; -
 *                           If ERROR, "Kstat" holds the precise error code.
 *-3*/
    DWORD          dwMax = 0L;         /* #keys currently in cache */
    DWORD          dwCount = 0L;       /* Loop count variable [1...dwMax] */
    eRetType       eRetCode = ERROR;   /* Return code for key.c functions */


    /* 1: Initialize status variable for dump (current cache length). */
    dwMax = pstDBx->stCache.dwCused;
    assert(dwMax >= 0L);


    /* 2: Dump cache to stdout: header & contents */
    fputs("\n========== DUMP OF CACHE ==========\n", stdout);
    if (dwMax == 0L)
       /* 2.1: Cache empty, - report to stdout */
	fputs("CACHE:\tis currently empty !\n", stdout);

    else
       /* 2.2: Cache contains data; - LOOP through cache array ... */
	for (dwCount = 1; dwCount <= dwMax; dwCount++) {

	    /* 2.2.1: Dump contents of cache slot */
	    fprintf(stdout, "CACHE:\tIndex[%4lu] : Offset[%lu]\n",
		    dwCount, (*(pstDBx->stCache.padwData))[dwCount] );

	    /* 2.2.2: If fAll==TRUE, dump contents of datarec's as well */
	    if (fAll) {
              eRetCode = eKeyDatDump(pstDBx->fdData,
                         (*(pstDBx->stCache.padwData))[dwCount]);
              KRET_ERR(eRetCode == ERROR, Kstat, 1500)
	    }
	    /* else (!fAll): ignore contents of datarecord */

	} /* END [2.2]: LOOP through cache */


    /* Function complete : return ok */
    STCK("eKeyCacheDump");
    KRET_OK

} /* END function eKeyCacheDump() */



/*+3 MODULE KEY.C ----------------------------------------------------------*/
/*   NAME   16                    eKeyRecDump                               */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   eRetType
eKeyRecDump(key, dwFileOffset)
    char     *key;             /* Key string */
    DWORD    dwFileOffset;     /* Datafile offset */
{
/* DESCRIPTION
 *    Dump one key keyvalue and its associated datafile offset, 
 *    both passed as parameters.
 *-3*/

    printf("Key[%s]-(lookup)->FlatfileOffset[%lu]\n", key, dwFileOffset);

    /* Function complete : return ok */
    STCK("eKeyRecDump");
    KRET_OK
}



/*+3 MODULE KEY.C ----------------------------------------------------------*/
/*   NAME   17                    eKeyDatDump                               */
/*-- SYNOPSIS --------------------------------------------------------------*/
#define      LINELEN 1024          /* Max linelength in datafile */
PRIVATE char aDatBuf[LINELEN + 1]; /* Linebuffer for dump of datarec. */

PRIVATE   eRetType
eKeyDatDump(fdDataFile, dwFileOffset)
    FILE *fdDataFile;              /* Datafile handle */
    DWORD dwFileOffset;            /* Datafile offset */
{
/* DESCRIPTION
 *    Dump the datafile record identified by file-handle and -offset,
 *    both passed as parameters.
 *    1: Set filepointer on datarecord, using <dwFileOffset>
 *    2: Read datarecord line-by-line, and print on stdout.
 *-3*/
    int       iSkip;               /* Skip KEYMARK (1. byte) in datarecord */
    eRetType  eRetCode = ERROR;    /* Return code for key.c functions */


    /* 1: Set filepointer on datarecord, using <dwFileOffset> */
    eRetCode = fseek(fdDataFile, dwFileOffset, SEEK_SET);
    KRET_ERR(eRetCode != 0, K_DATSEEK, 1700)


    /* 2: Read datarecord line-by-line, and print on stdout */
    for ( iSkip = 1;                /* iSkip: ignore 1.pos, 1.line! */
          (fgets(aDatBuf, LINELEN, fdDataFile) != (char *) NULL) &&
          (*(aDatBuf+iSkip) != KEYMARK);
          iSkip = 0)                /* iSkip: test 1.pos, next lines */
       fputs(aDatBuf, stdout);


    /* Function complete : return ok */
    STCK("eKeyDatDump");
    KRET_OK

} /* END function eKeyDatDump() */

#endif /* -DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG-DEBUG- */



/*+1 MODULE KEY.C ==========================================================*/
/*   NAME   18                    eKeyBufDump                               */
/*== SYNOPSIS ==============================================================*/
#define    VDULIN  75              /* Length of one line dump'ed to VDU */
#define    RAW     FALSE           /* Dump mode : raw or formatted */

PUBLIC    eRetType
eKeyBufDump(pstBF, fAll)
    BUFFER   *pstBF;               /* Pointer to BUFFER structure to dump */
    FLAG     fAll;                 /* Dump whole buffer, - not just datacontent */
{
/* DESCRIPTION
 *    Dump the contents of the bufferarea <pstBF->pzBaddr> to stdout.
 *    (Obs. PUBLIC function made accessible to user programs).
 *
 *    1: Initialize status variables for dump (buffer pointer & length).
 *    2: Dump buffer to stdout: print header & dump buffer contents [...]
 *       according to parameter fAll and compile-time switch RAW :
 *       2.1: Catch & report empty buffer; otherwise 
 *       2.2: fAll [FALSE]: dump only the (zero terminated!) string of data.
 *                 [TRUE ]: dump the whole bufferarea (cf. pstBF->wBsize),
 *                          RAW [TRUE ]: dump buffer as one long string
 *                              [FALSE]: insert linebreaks for each VDULIN chr.
 * RETURN
 *    Side effects ........: Buffer contents (if any) dumped on stdout.
 *    Function return value: OK (operation succeeded).
 *-1*/
    WORD      wLeft = 0;       /* #chars left in buffer for dumping to VDU */
    char      *pB1 = NULL;     /* Scratch buffer pointer */
    char      *pB2 = NULL;     /* Scratch buffer pointer */


    /* 1: Initialize dump status variables */
    pB1 = pstBF->pzBaddr;      /* Initialize start pointer for dumping */ 
    wLeft = pstBF->wBsize;     /* Initialize startsize for dumping */
    assert(wLeft >= 0);


    /* 2: Dump buffer to stdout: header & contents */
    D(fputs("\n========== DUMP OF BUFFER =========\n", stdout));
    D(fputc('[', stdout));

    if (wLeft == 0)
       /* 2.1: Buffer empty, - report to stdout */
       fputs("BUFFER:\tis currently empty !\n", stdout);

    else

       /* 2.2: Buffer contains data ... */
       /* Dump only datarecords (zero terminated!),- cf. flag param. */
       if (!fAll)
           fputs(pstBF->pzBaddr, stdout);
       else
       /* Dump whole buffer, - cf. comp.time switch: RAW [TRUE|FALSE] */
           if (RAW)             /*CONSTVAL*/
               /* Fast dump of whole buffer area, rely on terminal autowrap */
               fwrite(pstBF->pzBaddr, sizeof(char), wLeft, stdout);
           else {
               /* Dump whole buffer area, insert own linebreaks (more general) */
               for (pB2=pB1+wLeft; pB1+VDULIN < pB2; pB1 += VDULIN) {
                   fwrite(pB1, sizeof(char), VDULIN, stdout);
                   fputc('\n', stdout);
               }
               fwrite(pB1, sizeof(char), pB2-pB1, stdout);
           }

    D(fputs("]\n", stdout));


    /* Function complete : return ok */
    STCK("eKeyBufDump");
    KRET_OK

} /* END function eKeyBufDump() */



/* END module key.c                                                         */
/*==========================================================================*/