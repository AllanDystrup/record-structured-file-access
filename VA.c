/*+1========================================================================*/
/* MODULE                         VA.C                                      */
/*==========================================================================*/
/* FUNCTION    Toolbox for building virtual arrays (arrays as disk-files with
 *             automatic caching in RAM). This module contains a collection
 *             of basic routines for : creating, accessing and maintaining
 *             index'es for very fast access to simple datafiles.
 *             A datafile must be record-structured (arbitrary record size)
 *             with each record identified by a unique NUMERIC key-value.
 *
 * SYSTEM      Standard Ansi C.
 *             Tested on PC/MS DOS V.3.3 (MSC 6.0A) & UNIX SYS V.3 (GNU GCC).
 *
 * SEE ALSO    Modules:
 *             GENERAL.H, ACCESS.H, VA.H for macros & errorhandling
 *             INDEX.C/H ............... for building index'es from datafiles.
 *             KEY.C/H ................. for accessing datarec's via index'es.
 *
 * PROGRAMMER  Allan Dystrup
 *
 * COPYRIGHT   (c) Allan Dystrup, Kommunedata I/S February 1992.
 *
 * VERSION     $Header: d:/cwork/index/RCS/va.c 0.1 92/07/06 09:42:13
 *             Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *             --------------------------------------------------------------
 *             $Log:	va.c $
 *             Revision 0.1  92/07/06  09:42:13  Allan_Dystrup
 *             PREREL (ALFA1)
 *
 * REFERENCES  Mark Tichenor[1988] : "Virtual Arrays in C"
 *             Dr. Dobb's Journal, May 1988.
 *
 * USAGE       Module va.c features the following public routines for
 *             building and working with indexfiles; - See headerfile va.h
 *             and the function documentation for a detailed description of
 *             the user accesible datastructures and interface functions.
 *                 eVAIdxCreate()        // Create a new indexfile
 *                 eVAIdxOpen()          // Open an existing indexfile
 *                 eVAIdxClose()         // Close an open indexfile
 *
 *                 eVAeyInsert()         // Insert new (key,offset) in index
 *                 eVAKeyDelete()        // Delete (key,offset) from index
 *                 eVAKeyFind()          // Look up (key,offset) in index
 *
 *                 eVAIdxGetLoad()       // Get loadfactor for open indexfile
 *                 eVAIdxGetSize()       // Get size for open indexfile
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
 *             The errorchecking of file I/O is rather "slack" for max speed.
 *
 * =========================== VIRTUAL ARRAYS ================================
 *
 * A Virtual Array (VA) is stored on disk, but accessed as though it was in
 * memory. This functionality is obtained by providing a set of access-
 * functions with built-in file management : When a value is assigned to an
 * array-element and when an array-element is referenced in an expression,
 * data is automatically written-to/read-from the corresponding VA file-record.
 * Thus the user never performs any explicit reads/writes on a VA.
 *
 * To minimize disk I/O the access-functions pass their record read/write-
 * operations through a cache buffer.
 *
 * The extremely simple access strategy of VA's can be used in building index-
 * files based on direct lookup : datafile-offset = VA[keyvalue]. This method
 * however is only feasible for datafiles with numeric key-values. Note that
 * the indexfile will be relatively large, when the keyvalue-range is big and
 * the datafile-records small; - For sparse keyvalues a scatter-storage (HASH)
 * technique should be preferred, unless very fast access is important.
 *
 *
 * =========================== MODULE STRUCTURE ==============================
 *
 *                                 Data Model
 * To define a Virtual Array (VA) on disk and in core we need some datastruct's
 * for size- & statusinfo (one HEADER-record) and for array element values
 * (multiple KEY-records). These data must be maintained both external and in
 * RAM. The appropriate data-structs. are defined in the include-file VA.H.
 *
 * (1) Header-record layout
 *                           RAM (incore)
 *                           struct stVACore (*VACB)
 *                           +---------------------+ FILE:
 *                           | enum     indexmode  | Open mode [RO | RW]
 *                           | FILE     *pfArFile  | Handle for VA file
 *             +---------------SIZEINFO stSize     | VA file size info.
 *             |             |                     | CACHE:
 *             |             | WORD wBfSize        | #rec's in VA buf. (BFSIZE)
 *             |             | WORD wBfElSize      | #bytes in VA buf-elem.
 *             |             | char *pcBf-------+  | Ptr. to VA cache-buf.
 *             |             | char *pcArElInit |  | Ptr. to (last) blank rec.
 *             |             +------ | -------- | -+
 *             |                     |          |
 *             |                     |          |   VA Cache buffer (malloc'ed)
 *             |                     |          |   Rec#  Rec.Contents
 *             |                     |          |   +-----+--------------------+
 *             |                     |        0 +-->|DWORD: <occupied slot>    |
 *             |                     |              +-----+--------------------+
 *             |                     |        1     |EMPTY: <empty slot>       |
 *             |                     |              +-----+--------------------+
 *             |                     |        :     :     :                    :
 *             |                     |              +-----+--------------------+
 *             |                     |     BFSIZE   |DWORD| <occupied slot>    |
 *             |                     |     ---------+-----+--------------------+
 *             |                     +---->BFSIZE+1 | --- | initialization rec |
 *             |                           ---------+-----+--------------------+
 *     DISK (external) HEADER
 *     struct stVAsize (SIZEINFO)
 *     +-----------------+
 *     | DWORD dwArSize  | #key-rec's total in VA file
 *     | DWORD dwArUsed  | #key-rec's used in VA file
 *     | WORD  wArElSize | #bytes in each key-rec
 *     | char  cFill	 | Fill char for empty rec
 *     +-----------------+
 *
 *
 * (2) Key-record layout (default)
 *
 *     DISK (external)
 *     struct stVArec (REC)
 *     +-----------------+
 *     | DWORD  offset	 | map VA[keyvalue]->FileOffset
 *     +-----------------+ (value:EMPTY if not defined)
 *
 *
 *                         Function decomposition
 * The functions to generate and access these data structures may be grouped
 * into 2 separate levels :
 * [1] HIGH LEVEL (INDEX INTERFACE)
 *     Application functions built "on top of" the basic VA-functionality.
 *     In this module we build an index interface (Hdr- & Key-rec. operations),
 *     - you may easily extend or substitute these for new purposes.
 * [2] LOW LEVEL (VIRTUAL ARRAY INTERFACE)
 *     Basic Virtual Array (VA) functions, - these constitute a stable set of
 *     utilities to generate, maintain and access VA-files on disk.
 *
 * The main calling hierachy is outlined in the following diagram :
 *
 *                    (1) Index operations
 *
 *      (1.1)                          (1.2)
 *      Index Header operations        Index Key operations
 *
 *      eVAIdxCreate                    eVAKeyInsert----+
 *       | eVAIdxOpen                   eVAKeyDelete--->+
 *       |  | eVAIdxClose               eVAKeyFind----->+-----------+
 *       |  |          |                                |           |
 *       |  |          |                           [RW] |      [RO] |
 *       |  |          |                                |           |
 *       |  |          |                                |           |
 *   ... |  | ........ | .............................. | ......... | ...
 *       |  |          |                                |           |
 *       |  |          |       (2) Virtual Array        |           |
 *       |  |          |                                |           |
 *       |  |          |       (2.1)                    |           |
 *       |  |          |       VA high-level I/O        |           |
 *       |  |          |       MACRO'S                  |           |
 *       |  |          |                          dwAOFF(pVA,dw)  dwROFF(pVA,dw)
 *       |  |          |                                |           |
 *       |  |          |                          pAREC(pVA,dw)   pRREC(pVA,dw)
 *       |  |          |       (2.2)                    |           |
 *       |  |          |       VA low-level I/O         |           |
 *       |  |          |       FUNC'S                   |           |
 *       +--|->iVAinit |                          pvVAaccess      pvVAread
 *       +->+->pVAopen |
 *          iVAclose<--+
 *
 *-1========================================================================*/



/*==========================================================================*/
/*                             Includes                                     */
/*==========================================================================*/
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <limits.h>
#include  <signal.h>

#define    _VA_ALLOC
#include   "va.h"
#define    EMPTY   ((DWORD)ULONG_MAX)

/* #define S/H-DEBUG: runtime check of stack- and heap on DOS  */
/* Relies on PC/MS DOS V.3.3 system files which are deprecated */
//#include "../check/stck/stck.h"
   # define STCK(x)
   # define REAL_MAIN	main
//#include "../check/hpck/hpck.h"


/*==========================================================================*/
/*                         Function Prototypes                              */
/*==========================================================================*/
PRIVATE void
         *pvVAaccess P((VACB pVA, DWORD dwIndex));

PRIVATE void
         *pvVAread P((VACB pVA, DWORD dwIndex));



#ifdef MAIN
/****************************************************************************/
/******************************** MAIN **************************************/
/****************************************************************************/

PRIVATE void
        vSigCatch P((int iSigNum));


/* Define "signon message" for testdriver, module va.c */
PRIVATE char SIGNON[] =
        "\nKMD VirtualArray (VA) Functions (Testdriver), Version 0.1.0\n"
        "MOD[va.c] VER[0.1.0 Pre] DAT[92/07/10] DEV[ad dec]\n"
        "Copyright (c) KommuneData I/S 1992\n\n";


#define   KEYMAX  50          /* Max. size of key (#byte) */
#define   STEP    2L          /* Step-size for generating new VA file rec's */
VACB      pVA = NULL;         /* Handle for "incore" descr.struct */



/*+1 MODULE VA.C ===========================================================*/
/*   NAME   01                     main                                     */
/*== SYNOPSIS ==============================================================*/
int REAL_MAIN()
{
/* DESCRIPTION
 *   Testdriver for module va.c; - exercises the functions in the module
 *   and validates the functionality through trace-statements (when compiled
 *   with flag "DEBUG").
 *
 *   1: Print signon message & setup to catch "break" signals.
 *
 *   2: MAKE and fill a new VirtualArray (VA) ... :
 *      2.1: Try creating & opening new VA file, mode RW (init header), -
 *           actions 2.x are skipped, if the VA-file already exists
 *      2.2: Create dwArSize VA elements (in chunks of STEP)
 *      2.3: Delete elements #10-#19 in VA
 *      2.4: Try Re-initializing duplicate & deleted elements
 *      2.5: Close VA gracefully (flush cache to disk & release resources)
 *
 *   3: TEST the Virtual Array (VA) by looking up entries in the VA file ... :
 *      3.1: Open existing VA file, mode RO
 *      3.2: Print VA statistics
 *      3.3: Access all elements of the VA
 *      3.4: Close VA gracefully (flush cache to disk & release resources)
 *
 * RETURN
 *   main() is a testdriver and not intented to interface with any calling
 *   program. The return value from main() is thus insignificant in this
 *   context.
 *   You should however notice the checking of error-conditions on return
 *   from each call to a PUBLIC function defined in va.c; - This practice
 *   should also be followed in your application to "catch" and diagnose any
 *   malfunction in the system or in the services provided by this module.
 *   (You will probably want to write your own error-handling, though).
 *
 * EXAMPLE
 *   The contents of function main() demonstrates the basic aspects of
 *   building and accessing a VirtualArray using the *LOW LEVEL* public data-
 *   structures and interface-functions in VA.C/H.
 *   The modules INDEX.C/H and KEY.C/H provides more *HIGL LEVEL* interfaces
 *   for generating and using indexfiles from application programs.
 *
 * SEE ALSO
 *   general.h, access.h & va.h for a detailed description of symbolic constants,
 *   macro's, data structures, return codes and error codes.
 *-1*/

    char      pcKey[KEYMAX];     /* Key-value in string reprentation */
    register  DWORD dwKey = 0L;  /* Key-value in numeric representation */
    DWORD     dwOff    = 0L;     /* Offset of datarecord in datafile */
    DWORD     dwArSize = 10000L; /* Max Size (#slots) for VirtualArray */
    DWORD     dwSize   = 0L;     /* Current total slots in VA */
    DWORD     dwUsed   = 0L;     /* Current used slots in VA */
    WORD      wLoad    = 0;      /* Current load of VA: (100*dwUsed/dwSize) */
    eRetType  eRetCode = ERROR;  /* Return code of VA function [ERROR | OK] */


    /*----------------- 1: Signon and setup signal catcher -----------------*/
    /* 1.1: Signon */
    fputs(SIGNON, stdout);


    /* 1.2: Setup to redirect interrupt signals to our own handler */
    signal(SIGINT, vSigCatch);
    signal(SIGTERM, vSigCatch);


    /*----------------- 2: Make & fill a new VA index ----------------------*/

    /* 2.1: Try creating & opening new VA file, mode RW (init header) */
    eRetCode = eVAIdxCreate(&pVA, "./VA/va.key", sizeof(REC), 0L);


    /* If create OK, initialize elements of VA file ... */
    if (eRetCode == OK) {

	/* 2.2: Create dwArSize VA elements in chunks of STEP */
    /*      OBS: for test only even recs (#define STEP 2L) */
	fprintf(stdout, "\nCreating %ld elements...\n", dwArSize);
	for (dwKey = 0L; dwKey < dwArSize; dwKey += STEP) {
	    sprintf(pcKey, "%lu", dwKey);
	    ACHK_ERR(eVAKeyInsert(&pVA, pcKey, dwKey), A_STOP)
	    fprintf(stdout, "\tGenerating :\tArray[%s] = %4lu            \r",
              pcKey, dwKey);
 	}

	/* 2.3: Delete elements #10-#19 in VA */
	fprintf(stdout, "\n\nDeleting 10 of %ld elements: [#10-#19]...\n", dwArSize);
	for (dwKey = 10L; dwKey <= 19L; dwKey++) {
	    sprintf(pcKey, "%lu", dwKey);
	   	ACHK_ERR(eVAKeyDelete(&pVA, pcKey), A_CONT)
		fprintf(stdout, "\tDeleting  :\tArray[%s]\n", pcKey);
	}

	/* 2.4: Try Re-initializing duplicate & deleted elements */
	fprintf(stdout, "\nRe-initializing 10 of %ld elements: [#5-#14] to #+100...\n", dwArSize);
	for (dwKey = 5L; dwKey <= 14L; dwKey++) {
	    sprintf(pcKey, "%lu", dwKey);
	    ACHK_ERR(eVAKeyInsert(&pVA, pcKey, dwKey + 100L), A_CONT)
 	    D(fprintf(stdout, "\tRegenerating :\tArray[%s]= %4lu\n", pcKey, dwKey+100L);)
	}

	/* 2.5: Close index gracefully */
	ACHK_ERR(eVAIdxClose(&pVA), A_STOP)

    }   /* END 2: VA-create; - Else (create ERROR) assume file exists */
    else {

	/*----------------- 3: Find entries in the VA file ---------------------*/

	/* 3.1: Open existing VA file, mode RO */
	ACHK_ERR(eVAIdxOpen(&pVA, "va.key", "rb"), A_STOP);

	/* 3.2: Print VA statistics */
	ACHK_ERR(eVAIdxGetSize(&pVA, &dwSize, &dwUsed), A_STOP)
	fprintf(stdout, "\nIndex Keyrecords:\tSize=[%lu],\tUsed=[%lu]\n", dwSize, dwUsed);
	ACHK_ERR(eVAIdxGetLoad(&pVA, &wLoad), A_STOP)
	fprintf(stdout, "\nIndex Loadfactor:\tLoad=[%d]\n", wLoad);

	/* 3.3: Access all elements of the VA */
	fprintf(stdout, "\nAccessing %ld elements...\n", dwArSize);
	for (dwKey = 0L; dwKey < dwArSize; dwKey++) {
           sprintf(pcKey, "%lu", dwKey);
           if (eVAKeyFind(&pVA, pcKey, &dwOff) == OK) {
	           D(fprintf(stdout, "\tACCESSING :\tArray[%4s] = %4lu\n", pcKey, dwOff);)
           } else {
	           D(fprintf(stdout, "\tNOTFOUND  :\tArray[%4s] = %lu\n", pcKey, dwOff);)
           }
	}

	/* 3.4: Close VA */
	ACHK_ERR(eVAIdxClose(&pVA), A_STOP)

    } /* END 3: VA-open */

} /* END function main() */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   01.01                 vSigCatch                                 */
/*-- SYNOPSIS --------------------------------------------------------------*/
#define    MAXLINE 81

PRIVATE void
vSigCatch(iSigNum)
    int       iSigNum;	       /* Signal number to catch */
{
/* DESCRIPTION
 *    Support function for main() test driver.
 *    Signal handler set up to catch the "break" signals : SIGINT (asynch.
 *    interactive attention) & SIGTERM (asynch. interactive termination).
 *    1: Prompts user for break confirmation
 *    2: Depending on user confirmation : [Y]->terminate or [N]->continue... :
 *       2.1: Terminates program "gracefully" (using global VA file descr.)
 *       2.2: Continues : eat rest of line, reset signal, and continue.
 * RETURN
 *    If break confirmed: program terminated with exit code 'EXIT_FAILURE'
 *    else: signal 'iSigNum' reset and program execution resumed.
 * BUGS
 *    Asynch. signals don't guarantee access to volatile data at sequence pts;
 *    Since we restrict our access to READ operations, this shouldn't pose any
 *    problem, - though not strictly ANSI (cf. type sig_atomic_t).
 *-4*/

	/* --------------------------------------------------------------------------
    char      pzLine[MAXLINE]; // Line buffer
    DWORD     dwSize = 0L;     // Var. param1 for eVAGetSize
    DWORD     dwUsed = 0L;     // Var. param2 for eVAGetSize

    assert(iSigNum > 0);

    // 1: Prompt user for break confirmation
    fprintf(stdout, "\nINTERRUPT:  Signal [%d] received\n", iSigNum);
    fprintf(stdout, "\tCurrent state of VirtualArray: integrity[%s]\n",
	    (pVA->indexmode == RW ? "UNKNOWN" : "OK"));
    (void) eVAIdxGetSize(&pVA, &dwSize, &dwUsed);
    fprintf(stdout, "\tCurrent size  of VirtualArray: total[%lu], used[%lu]\n",
	    dwSize, dwUsed);
    fprintf(stdout, "\tAbort program? [Y|N]  => ");


    // 2: Depending on user answer : terminate [Y] or continue [N]
    if (toupper(getchar()) == 'Y') {

	// 2.1: terminate program "gracefully"
	eVAIdxClose(&pVA);
	exit(EXIT_FAILURE);
    }
    else  {
	// 2.2: Continue : eat rest of line, reset signal, and continue
	fgets(pzLine, MAXLINE, stdin);
	signal(iSigNum, vSigCatch);
    }
	-------------------------------------------------------------------------- */

	/* 2.1: Terminate program "gracefully" */
	eVAIdxClose(&pVA);
	signal(iSigNum, SIG_DFL);
	raise(iSigNum);
	abort();

} /* END function vSigCatch() */

#endif			       /* MAIN */




/**************************** [1]  HIGH LEVEL *******************************/
/**************************** INDEX INTERFACE *******************************/
/****************************************************************************/


/*+2 MODULE VA.C ===========================================================*/
/*   NAME   02                     eVAIdxCreate                             */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eVAIdxCreate(ppVA, pzIdxFile, dummy1, dummy2)
    VACB     *ppVA;	       /* Handle for VA index descr.struct to create */
    char     *pzIdxFile;       /* Name of physical Virtual Array index (file) */
    WORD      dummy1;	       /* Dummy argument, - ignored */
    DWORD     dummy2;	       /* Dummy argument, - ignored */
{
/* DESCRIPTION
 *    Creates (and opens) a VirtualArray file in the current directory on
 *    disk with the name of "pzIdxFile" and mode Read/Write. A header with
 *    VA size-info (initially all zero) is written to the file, and an incore
 *    descriptor structor for VA (VACB) is set up with handle ppVA.
 *
 *    1: Call iVAinit() to create new VA-file, write header & close file.
 *    2: Call eVAIdxOpen() to (re)open the VA-file and set up an incore VACB.
 *
 * RETURN
 *    Side effects (netto).: VirtualArray file created & opened.
 *                           Incore VACB allocated & initialized.
 *    Call-by-reference....: Handle ppVA pointed to new VACB.
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 *-2*/

    /* 1: Call iVAinit() to create new VA-file, write header & close file */
    ARET_ERR(iVAinit(pzIdxFile, sizeof(REC), ' ') == 0, A_NOTCREATE, 200)


    /* 2: Call eVAIdxOpen() to (re)open VA-file and set up an incore VACB */
    ARET_ERR(eVAIdxOpen(ppVA, pzIdxFile, "r+b") == ERROR, A_FILEOPEN, 201)

    ARET_OK

} /* END function eVAIdxCreate() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   03                     eVAIdxOpen                               */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eVAIdxOpen(ppVA, pzIdxFile, pzAccess)
    VACB     *ppVA;	   /* Handle for VA index descr.struct to open */
    char     *pzIdxFile;   /* Name of physical Virtual Array index (file) */
    char     *pzAccess;	   /* Mode "rb" (ReadOnly) or "w+b" (ReadWrite) */
{
/* DESCRIPTION
 *    Opens an already existing VirtualArray file in the current directory
 *    on disk with the name of "pzIdxFile" and mode "pzAccess".
 *    Allocates an incore VA descr.struct. (VACB), initializes this with
 *    sizeinfo from the VA-file header and points handle ppVA to the VACB.
 *
 *    1: Call pVAopen() to set up a default incore VACB (save handle in ppVA).
 *    2: Correct template for empty VA-rec in VACB to : EMPTY.
 *       (the general/default empty VA-rec. template is : "all spaces")
 *
 * RETURN
 *    Side effects.........: VA-file opened & incore VACB instantiated.
 *    Call-by-reference....: Handle ppVA pointed to new VACB.
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 *-2*/

    /* 1: Call pVAopen() to set up an "incore" VACB (save handle in *ppVA) */
    *ppVA = pVAopen(pzIdxFile, pzAccess);
    ARET_ERR(*ppVA == NULL, A_FILEOPEN, 300)


    /* 2: Correct empty index-rec in VA header to : EMPTY */
    *((DWORD *) ((*ppVA)->pcArElInit)) = EMPTY;

    ARET_OK

} /* END function eVAIdxOpen() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   04                     eVAIdxClose                              */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eVAIdxClose(ppVA)
    VACB     *ppVA;	       /* Handle for VA index descr.struct to open */
{
/* DESCRIPTION
 *   Closes an open VA file, and releases the resources allocated for the
 *   attached "in core" VA descriptor structure (VACB) pointed to by *ppVA.
 *
 *   1: Calls the generic VA function iVAclose() to perform the shutdown.
 *
 * RETURN
 *    Side effects.........: VA cache buffer flushed (if VA opened mode RW).
 *                           VirtualArray file (*ppVA)->pfArFile closed.
 *    Call-by-reference....: *ppVA VA descr.struct. fully deallocated.
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 *-2*/

    /* 1: Call iVAclose() to release all resources allocated to VACB */
    ARET_ERR(iVAclose(*ppVA) == 0, A_FILECLOSE, 400)

    ARET_OK

} /* END function eVAIdxClose() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   05                     eVAKeyInsert                             */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eVAKeyInsert(ppVA, pcKey, dwDatOffset)
    VACB     *ppVA;	       /* Handle for VA index descr.struct */
    char     *pcKey;           /* Key-string (index/slot) for VirtualArray */  
    DWORD     dwDatOffset;     /* Value to insert in VA-slot: VA[Key] */
{
/* DESCRIPTION
 *   Inserts value "dwDatOffset" into the VirtualArray, slot VA[Key]; -
 *   String "pcKey" holds a unique Key-value identifying a datafile-record,
 *   Number dwDatOffset holds the file-offset of the rec. corresp. to pcKey.
 *   The function inserts the value "dwDatOffset" into VA-slot: VA[Key].
 *
 *   1: CONVERT Key from string (pcKey) to number (dwKey) format.
 *   2: ASSERT that slot VA[Key] -if it exists- is not already occupied;
 *      If Key > VA-max, VA is extended with free slots until & incl. Key.
 *      The unique value: EMPTY marks a free/new slot in the VirtualArray.
 *   3: INSERT value "dwDatOffset" into the VirtualArray, slot VA[Key]; -
 *      Duplicate keys are obviously not allowed (Key<->VAslot 1:1 relation).
 *   4: UPDATE VA header with new "used count" (incr. #used slots).
 *
 * RETURN
 *    Side effects.........: dwDatOffset inserted into VA-slot VA[Key]; -
 *                           VA-file extended with free slots if past EOF.
 *                           VACB & VA-file header size-info updated.
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 *-2*/

    char     *pcScan   = NULL;  /* Scan-ptr for ANSI lib.func. strtoul() */
    DWORD     dwKey    = 0L;    /* Numeric Key-value (converted from pcKey) */
    DWORD     dwOff    = 0L;    /* Current val. in VA[Key], EMPTY if free */
    REC      *pRec     = NULL;  /* Ptr to cache buffer entry holding VA-rec */
    int       iRetCode = 0;     /* Integer return code */


    /* 1: Convert Key from string to number format */
    dwKey = strtoul(pcKey, &pcScan, 10);
    ARET_ERR(pcScan == pcKey, A_OTHER, 500)


    /* 2: Assert that slot VA[Key] (if it exists) is currently free */
    if (dwKey < (DWORD) (*ppVA)->stSize.dwArSize) {
	(void) eVAKeyFind(ppVA, pcKey, &dwOff);
	ARET_ERR(dwOff != EMPTY, A_DUPLICATE, 501)
    }


    /* 3: Insert value "dwDatOffset" into VirtualArray, slot VA[dwKey] */
    /* If dwKey > dwArSize, first extend VA with free slots, cf pvVAaccess */
    pRec = pAREC(*ppVA, dwKey);
    ARET_ERR(pRec == NULL, A_WRITE, 502)
    pRec->dwOffset = dwDatOffset;


    /* 4: Update VA header with new "used count" (incr. #used slots) */
    iRetCode = fseek((*ppVA)->pfArFile, sizeof(DWORD), SEEK_SET);
    ARET_ERR(iRetCode != 0, A_SEEK, 503)
    (*ppVA)->stSize.dwArUsed++;
    iRetCode = fwrite(&(*ppVA)->stSize.dwArUsed, sizeof(DWORD), 1, (*ppVA)->pfArFile);
    ARET_ERR(iRetCode != 1, A_WRITE, 504)


    ARET_OK

} /* END function eVAKeyInsert() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   06                     eVAKeyDelete                             */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eVAKeyDelete(ppVA, pcKey)
    VACB     *ppVA;	       /* Handle for VA index descr.struct */
    char     *pcKey;           /* Key-string (index/slot) for VirtualArray */  
{
/* DESCRIPTION
 *   Inserts a "free-mark" into the VirtualArray, slot VA[Key]; -
 *   String "pcKey" holds a unique Key-value identifying a datafile-record,
 *   Const. EMPTY is a unique value used for marking a free VA-slot.
 *   The function inserts EMPTY into VA-slot: VA[Key], thus deleting it.
 *
 *   1: CONVERT Key from string (pcKey) to number (dwKey) format.
 *   2: ASSERT that slot VA[Key] exists and is currently occupied,
 *      (unique value: EMPTY marks a free slot in the VirtualArray).
 *   3: INSERT value EMPTY (ie. deleted/free) into VA, slot VA[Key].
 *   4: UPDATE VA header with new "used count" (decr. #used slots).
 *
 * RETURN
 *    Side effects.........: VA-slot VA[Key] marked deleted/free; -
 *                           VACB & VA-file header size-info updated.
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 *-2*/

    char     *pcScan   = NULL; /* Scan-ptr for ANSI lib.func. strtoul() */
    DWORD     dwKey    = 0L;   /* Numeric Key-value (converted from pcKey) */
    DWORD     dwOff    = 0L;   /* Current val. in VA[Key], EMPTY if free */
    int       iRetCode = 0;    /* Integer return code */


    /* 1: Convert Key from string to number format */
    dwKey = strtoul(pcKey, &pcScan, 10);
    ARET_ERR(pcScan == pcKey, A_OTHER, 600)


    /* 2: Assert that slot VA[Key] exists and is occupied */
    ARET_ERR(dwKey >= (*ppVA)->stSize.dwArSize, A_NOTFOUND, 601)
    (void) eVAKeyFind(ppVA, pcKey, &dwOff);
    ARET_ERR(dwOff == EMPTY, A_NOTFOUND, 602)


    /* 3: Insert value EMPTY (ie. free) into VirtualArray, slot VA[Key] */
    dwAOFF(*ppVA, dwKey) = EMPTY;


    /* 4: UPDATE VA header with new "used count" (decr. #used slots) */
    iRetCode = fseek((*ppVA)->pfArFile, sizeof(DWORD), SEEK_SET);
    ARET_ERR(iRetCode != 0, A_SEEK, 603)
    (*ppVA)->stSize.dwArUsed--;
    iRetCode = fwrite(&(*ppVA)->stSize.dwArUsed, sizeof(DWORD), 1, (*ppVA)->pfArFile);
    ARET_ERR(iRetCode != 1, A_WRITE, 604)


    ARET_OK

} /* END function eVAKeyDelete() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   07                     eVAKeyFind                               */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eVAKeyFind(ppVA, pcKey, pdwDatOffset)
    VACB     *ppVA;	       /* Handle for VA index descr.struct */
    char     *pcKey;           /* Key-string (index/slot) for VirtualArray */  
    DWORD    *pdwDatOffset;    /* Value to retrieve from VA-slot: VA[Key] */
{
/* DESCRIPTION
 *   Finds VA-slot VA[Key], and reads it's value into var.param *pdwDatOffset.
 *   String "pcKey" holds a unique Key-value identifying a datafile-record,
 *   Ptr. "pdwDatOffset" points to a variable for returning an offset value.
 *   The function read the value in VA[KEY], and assigns it to *pdwDatOffset.
 *
 *   1: CONVERT Key from string (pcKey) to number (dwKey) format.
 *   2: ASSERT that slot VA[Key] exists (ie. Key is inside current VA range).
 *   3: RETRIEVE value (ie. datafile-offset) from VA slot: VA[Key]; -
 *         If    VA-file opened mode RW , flush cache entry before read,
 *         Else (VA-file opened mode RO), just overwrite cache entry.
 *
 * RETURN
 *    Call-by-reference....: Offset of datarecord identified by Key
 *                           returned in var.param. *pdwDatOffset.
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 *                           (eg. attempt to find a free/deleted key-record)
 *-2*/

    char     *pcScan = NULL;   /* Scan-ptr for ANSI lib.func. strtoul() */
    DWORD     dwKey  = 0L;     /* Numeric Key-value (converted from pcKey) */


      /* 1: Convert Key from string to number format */
    dwKey = strtoul(pcKey, &pcScan, 10);
    ARET_ERR(pcScan == pcKey, A_OTHER, 700)


    /* 2: Assert that VirtualArray slot: VA[Key] currently exists */
    ARET_ERR(dwKey >= (*ppVA)->stSize.dwArSize, A_NOTFOUND, 701)


    /* 3: Retrieve datafile-offset from VA slot VA[Key] ... */
    if ((*ppVA)->indexmode == RW)	      /* Flush cache entry bef. read */
	*pdwDatOffset = dwAOFF(*ppVA, dwKey);
    else /* indexmode: RO */ 
	*pdwDatOffset = dwROFF(*ppVA, dwKey); /* Just overwrite cache entry */


    /* Return offset in *pdwDatOffset; If free/deleted, signal NOTFOUND */
    ARET_ERR(*pdwDatOffset == EMPTY, A_NOTFOUND, 702)
    ARET_OK

} /* END function eVAKeyFind() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   08                     eVAIdxGetSize                            */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eVAIdxGetSize(ppVA, pdwSize, pdwUsed)
    VACB     *ppVA;	       /* Handle for VA index descr.struct (VACB) */
    DWORD    *pdwSize;	       /* Ptr. to variable for returning size */
    DWORD    *pdwUsed;	       /* Ptr. to variable for returning used */
{
/* DESCRIPTION
 *    Retrieve current size (total #VA-slots) & usage (used #VA-slots)
 *    from "incore" VA descriptor struct. (VACB) to var. parameters.
 *
 *    1: RETRIEVE array-size & -usage from "incore" VACB with handle ppVA.
 *    2: ASSERT sensible values of size and used variables.
 *
 * RETURN
 *    Call-by-reference....: VA-size & used returned in *pdwSize & *pdwUsed.
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 *-2*/

    /* 1: Retrieve array-size & -used from incore descriptor structure */
    *pdwSize = (*ppVA)->stSize.dwArSize;
    *pdwUsed = (*ppVA)->stSize.dwArUsed;


    /* 2: Assert sensible values (hard internal error if not!) */
    assert(*pdwUsed <= *pdwSize);


    /* 3: RETURN size- and used-values in var.param.: *pdwSize & *pdwUsed */
    ARET_OK

} /* END function eVAIdxGetSize() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   09                     eVAIdxGetLoad                            */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eVAIdxGetLoad(ppVA, pwLoad)
    VACB     *ppVA;	       /* Handle for VA index descr.struct (VACB) */
    WORD     *pwLoad;	       /* Var. Param for ret. load = 100 * used/size */
{
/* DESCRIPTION
 *    Retrieve current size (total #VA-slots) & usage (used #VA-slots)
 *    from "incore" VA descriptor struct. (VACB), and use these values 
 *    to calculate the current VA load-factor = (100 * used)/total.
 *
 *    1: RETRIEVE array-size & -used from incore VACB (call eVAIdxGetSize()).
 *    2: ASSERT positive value of size (thus preventing divide-by-zero error).
 *    3: CALCULATE the VA load-factor from size & used.
 *
 * RETURN
 *    Call-by-reference....: VA-load factor returned in *pwLoad.
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 *-2*/

    DWORD     dwSize = 0L;    /* Current total size (#slots) of VA */
    DWORD     dwUsed = 0L;    /* Current used size (#slots) of VA */
    *pwLoad = 0;              /* Current load-factor of VA; - clear */


    /* 1: Call eVAIdxGetSize() to get array-size & -used from incore VACB */
    ARET_ERR(eVAIdxGetSize(ppVA, &dwSize, &dwUsed) != OK, Astat, 800)


    /* 2: Assert positive value of size (prevent divide-by-zero error) */
    if (dwSize == 0L) {
	ARET_OK
    }

    /* 3: Calculate VA load-factor from size & used */
    *pwLoad = (WORD) ((100L * dwUsed) / dwSize);


    ARET_OK

} /* END function eVAIdxGetLoad() */




/*************************** [2]  LOW  LEVEL ********************************/
/*********************** VIRTUAL ARRAY INTERFACE ****************************/
/****************************************************************************/


/*+2 MODULE VA.C ===========================================================*/
/*   NAME   10                     iVAinit                                  */
/*== SYNOPSIS ==============================================================*/
PUBLIC int
iVAinit(pzIdxFile, wElSize, iFill)
char   *pzIdxFile;     /* Name of physical Virtual Array (file) */
WORD   wElSize;	       /* Size (#byte) of one record in VA file */
int    iFill;          /* Filler-char for initializing empty slot */
{
/* DESCRIPTION
 *    Create a VirtualArray-index file in the current directory on disk
 *    with the name of <pzIdxFile> and an record-size of <wElSize>:
 *
 *    1: Check that VA file does not already exist (bail out if it does!)
 *    2: Create new VA file :
 *       2.1: OPEN new VA file, mode <w+b> (create/truncate).
 *       2.2: WRITE a VA header (for empty VirtualArray) to the file.
 *       2.3: CLOSE the VA file.
 *
 * RETURN
 *    Side effects.........: VA-file created, header-rec written, file closed.
 *    Function return value: 1 if operation succeeded, 0 otherwise.
 *-2*/

    FILE   *hVAFile  = NULL;        /* Handle for VA file */
    DWORD   dwArSize = 0L;          /* Init. size of VA file (only header) */
    DWORD   dwArUsed = 0L;          /* Init. #used entries,-empty from start */
    char    cFill    = (char)iFill; /* Reset char param. promotion from int! */
    int     iRetCode = 0;           /* Integer return code */


    D(fprintf(stdout, "Creating VA-file[%s] :\n", pzIdxFile);)


    /* ---------------------- Create empty VA-file ------------------------ */

    /* 1: Check that VA file does not already exist */
    if ((hVAFile = fopen(pzIdxFile, "r")) != (FILE *) NULL)
       goto L_ErrRet;

    assert(wElSize >= 1);


    /* 2.1: Create new VA file (open mode RW) */
    if (!(hVAFile = fopen(pzIdxFile, "w+b")))
       goto L_ErrRet;


    /* 2.2: Write an "empty" VA-header to the new VA file */
    iRetCode  = fwrite(&dwArSize, sizeof(DWORD), 1, hVAFile);  /* Write VA size of zero */
    iRetCode += fwrite(&dwArUsed, sizeof(DWORD), 1, hVAFile);  /* Write VA used of zero */
    iRetCode += fwrite(&wElSize, sizeof(WORD), 1, hVAFile);    /* Write VA element size */
    iRetCode += fwrite(&cFill, sizeof(char), 1, hVAFile);      /* Write fill char */
    if (iRetCode != 4 || ferror(hVAFile))
       goto L_ErrRet;

    D(fprintf(stdout, "\tWrote VA-header: dwArSize[%ld], dwArUsed[%ld], wArElSize[%d], cFill[%c]\n",
	     dwArSize, dwArUsed, wElSize, cFill));


    /* -------------------------- Return ---------------------------------- */

    /* 2.3.1: Fall through to Return OK */
    (void) fclose(hVAFile);
    return (1);

L_ErrRet:
    /* 2.3.2: Goto Return ERROR (after cleanup), - not considered harmfull! */
   (void) fclose(hVAFile);
   return (0);


} /* END function iVAinit() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   11                     pVAopen                                  */
/*== SYNOPSIS ==============================================================*/
PUBLIC    VACB
pVAopen(pzIdxFile, pzAccess)
    char     *pzIdxFile;    /* Name of physical Virtual Array index (file) */
    char     *pzAccess;     /* "rb" (for ReadOnly) or "w+b" (for ReadWrite) */
{
/* DESCRIPTION
 *    Open an existing VirtualArray (VA) file & set up an incore descriptor:
 *
 *    1:   Instantiate an "incore" VA descriptor structure (VACB) :
 *         1.2: Allocate "incore" VACB, and point pVA to it.
 *         1.2: Open the (existing) VA file <pzIdxFile>, mode <pzAccess>.
 *         1.3: Initialize the "incore" VACB from the VA file header.
 *    2:   Instantiate a VA cache-buffer for the new VACB :
 *         2.1: Allocate a VA cache buffer, size BFSIZE.
 *         2.2: Set up last buf-rec as a blank (using fill-char from VA hdr).
 *         2.3: Initialize buf-rec index negative (=empty) for all elements.
 *
 * RETURN (2.4)
 *    Side effects.........: VA-file opened & VACB w/cache buffer instantiated.
 *    Function return value: Ptr to instantiated VA descr., - NULL if error.
 *-2*/

    VACB      pVA        = NULL;   /* Ptr to incore VA descr.struct. (VACB) */   
    register  char *pcBf = NULL;   /* Ptr to VA cache buffer */
    register  WORD  i    = 0;      /* Scratch count integer */
    int       iRetCode   = 0;      /* Integer func return code */


    D(fprintf(stdout, "Opening VA-file[%s], mode[%s] :\n",
	     pzIdxFile, (strcmp(pzAccess, "rb") == 0 ? "RO" : "RW"));)


    /* ---------------------- Instantiate VACB ---------------------------- */

    /* 1.1: Allocate an "incore" VA-file descr.struct. & point pVA to it */
    pVA = (VACB) malloc(sizeof(struct stVACore));
    if (pVA == NULL)
	return NULL;


    /* 1.2: Open the (existing) VA file <pzIdxFile>, mode <pzAccess> */
    pVA->pfArFile = fopen(pzIdxFile, pzAccess);
    if (pVA->pfArFile == NULL)
       goto L_ErrRet;
    pVA->indexmode = (strcmp(pzAccess, "rb") == 0 ? RO : RW);


    /* 1.3: Initialize an "incore" VA-index descriptor from the file header */
    iRetCode  = fread(&(pVA->stSize.dwArSize), sizeof(DWORD), 1, pVA->pfArFile);
    iRetCode += fread(&(pVA->stSize.dwArUsed), sizeof(DWORD), 1, pVA->pfArFile);
    iRetCode += fread(&(pVA->stSize.wArElSize), sizeof(WORD), 1, pVA->pfArFile);
    iRetCode += fread(&(pVA->stSize.cFill), sizeof(char), 1, pVA->pfArFile);
    if (iRetCode != 4 || ferror(pVA->pfArFile))
       goto L_ErrRet;
    pVA->wBfElSize = sizeof(DWORD) + pVA->stSize.wArElSize;
    D(fprintf(stdout, "\tRead  VA-header: dwArSize[%ld], dwArUsed[%ld], wArElSize[%d], cFill[%c]\n",
       pVA->stSize.dwArSize, pVA->stSize.dwArUsed,
       pVA->stSize.wArElSize, pVA->stSize.cFill));


    /* ---------------------- Instantiate BUFFER -------------------------- */
           
    /* 2.1: Allocate a VA cache buffer */
    pVA->pcBf = (char *) malloc(pVA->wBfElSize * (BFSIZE + 1));
    if (pVA->pcBf == NULL)
       goto L_ErrRet;
    pVA->wBfSize = BFSIZE;


    /* 2.2: Set up blank-rec template, using filler-char "cFill" from VACB */
    pcBf = pVA->pcBf + pVA->wBfElSize * pVA->wBfSize;
    pVA->pcArElInit = pcBf + sizeof(DWORD);
    for (i = 0; i < pVA->wBfElSize; i++)
	*pcBf++ = pVA->stSize.cFill;


    /* 2.3: Init rec.index EMPTY (=ULONG_MAX) for all buffer elements */
    pcBf = pVA->pcBf;
    for (i = 0; i < pVA->wBfSize; i++) {
	*((DWORD *) pcBf) = EMPTY;
	pcBf += pVA->wBfElSize;
    }


    /* -------------------------- Return ---------------------------------- */

    /* 2.4.1: Fall through to Return OK */
    return (pVA);

L_ErrRet:
    /* 2.4.2: Goto Return ERROR (after cleanup), - not considered harmfull! */
    (void) fclose(pVA->pfArFile);
    free(pVA);
    return (NULL);

} /* END function pVAopen() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   12                     iVAclose                                 */
/*== SYNOPSIS ==============================================================*/
PUBLIC int
iVAclose(pVA)
    VACB      pVA;       /* Ptr to "incore" VA descriptor structure (VACB) */
{
/* DESCRIPTION
 *    Close an existing VirtualArray (VA) file & release the incore descriptor:
 *
 *    1: If VA file opened mode Read/Write, flush VA cache buffer to disk.
 *       1.1: Point to first VA cache record.
 *       1.2: LOOP: flush all occupied VA cache records to disk.
 *    2: Release all resources allocated to the VACB descriptor structure.
 *
 * RETURN
 *    Side effects.........: VA cache flushed (if mode RW); VACB released.
 *    Function return value: 1 if operation succeeded, 0 otherwise.
 *-2*/

    DWORD     dwArIndex = 0L;     /* Index into VA-file (VA slot number) */
    long      lFileOff  = 0L;     /* Offset into VA-file (#byte from start) */
    char      *pcBf     = NULL;   /* Ptr to VA cache buffer */
    register  WORD  i   = 0;      /* Scratch count integer */


    D(fprintf(stdout, "\nClosing VA-file\n");)


    /* 1: If VA file opened Read/Write, flush VA cache buffer to disk */
    if (pVA->indexmode == RW) {

	D(fprintf(stdout, "\tFlushing VA cache to disk\n");)
	/* 1.1: Point to first VA cache record */
	pcBf = pVA->pcBf;

	/* 1.2: Flush all occupied VA cache records to disk */
	for (i = 0; i < pVA->wBfSize; i++) {

	    /* Retrieve VA index from cache */
	    dwArIndex = *((DWORD *) pcBf);

	    /* If slot occupied (index != ULONG_MAX), write cache-elem. to VA on disk */
	    if (dwArIndex != EMPTY) {
		   lFileOff = HEADER + dwArIndex * pVA->stSize.wArElSize;
		   (void) fseek(pVA->pfArFile, lFileOff, SEEK_SET);
		   (void) fwrite(pcBf + sizeof(DWORD), pVA->stSize.wArElSize, 1, pVA->pfArFile);
	    }

	    /* Point to next VA cache record */
	    pcBf += pVA->wBfElSize;
	}
    }


    /* 2: Release all resources allocated to the VACB */
    D(fprintf(stdout, "\tReleasing all VACB resources\n");)
    free(pVA->pcBf);
    (void) fclose(pVA->pfArFile);
    free(pVA);


    return (1);

} /* END function iVAclose() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   13                     pvVAaccess                               */
/*== SYNOPSIS ==============================================================*/
PRIVATE void     *
pvVAaccess(pVA, dwArIndex)
    VACB      pVA;	            /* Pointer to VA-Control-Block struct */
    DWORD     dwArIndex;	    /* Index in VA for record to access */
{
/* DESCRIPTION
 *    Access (Read / Write) a record in the virtual array (VA) :
 *
 *    1:   Calc. cache buffer address (circular wrap) of VA rec# : "dwArIndex".
 *    2:   Retrieve current rec# in calculated cache buffer entry (1. field).
 *    3:   Test VA rec# "dwArIndex" against current rec# in the cache buffer :
 *         3.1: If VA-rec present, return address of rec. contents (2. field),
 *         3.2: else If past EOF of VA, create new element(s): extend VA file,
 *         3.3: else If buffer slot occupied by other rec., flush to disk.
 *    4:   Now read VA record "dwArIndex" from file to cache buffer, and
 *         Return address of rec. contents (2. field of cache buffer entry).
 *
 * RETURN
 *    Side effects.........: VA-file silently extended, if dwArIndex past EOF.
 *    Call-by-reference....: VACB (handle pVA) updated with new sizeinfo.
 *    Function return value: Pointer to record content (in cache-buffer), -
 *                           NULL if error (disk full et. al.)
 *-2*/

    WORD      wBfIndex    = 0;    /* Cache buffer addr. of VA-rec : "dwArIndex" */
    DWORD     dwBfArIndex = 0L;   /* VA-rec curr. occupying cache for dwArIndex */
    DWORD     dwTmpIndex  = 0L;   /* Temp. var. holding a VA index */
    char      *pcBf       = NULL; /* Generic ptr. to cache buffer element */


    /* 1: Calculate cache buffer address (pcBf) of VA record (dwArIndex) */
    wBfIndex = (WORD) (dwArIndex % pVA->wBfSize);
    pcBf = pVA->pcBf + (wBfIndex * pVA->wBfElSize);


    /* 2: Retrieve VA record currently in calculated cache buffer (dwBfArIndex) */
    dwBfArIndex = *(DWORD *) pcBf;


    /* 3.1: If dwArIndex present in buffer, return address of record contents */
    if (dwBfArIndex == dwArIndex)
	return (void *) (pcBf + sizeof(DWORD));


    /* 3.2: If past EOF of VA, create up to "dwArIndex" new EMPTY VA records */
    if (dwArIndex >= pVA->stSize.dwArSize) {

	/* Position on file-end & write EMPTY rec's up to dwArIndex (incl.) */
	(void) fseek(pVA->pfArFile, 0, SEEK_END);
	for (dwTmpIndex = pVA->stSize.dwArSize; dwTmpIndex++ <= dwArIndex; /* empty */ )
	   if (fwrite(pVA->pcArElInit, pVA->stSize.wArElSize, 1, pVA->pfArFile) != 1
           || ferror(pVA->pfArFile) != 0 )         /* Check for disk full! */
             return (void *) (NULL);

	/* Update VA header with new file size */
	(void) fseek(pVA->pfArFile, 0, SEEK_SET);
	pVA->stSize.dwArSize = dwArIndex + 1L;
	(void) fwrite(&pVA->stSize.dwArSize, sizeof(DWORD), 1, pVA->pfArFile);
    }


    /* 3.3: If buffer slot occupied by other element, flush to disk */
    if (dwBfArIndex != EMPTY) {
	(void) fseek(pVA->pfArFile, HEADER + (dwBfArIndex * pVA->stSize.wArElSize), SEEK_SET);
	(void) fwrite(pcBf + sizeof(DWORD), pVA->stSize.wArElSize, 1, pVA->pfArFile);
    }


    /* 4: Now read element from file to cache buffer */
    (void) fseek(pVA->pfArFile, HEADER + (dwArIndex * pVA->stSize.wArElSize), SEEK_SET);
    (void) fread(pcBf + sizeof(DWORD), pVA->stSize.wArElSize, 1, pVA->pfArFile);

    /* Reset buf.addr. of current rec. & Return addr. of rec.contents */
    *((DWORD *) pcBf) = dwArIndex;
    return (void *) (pcBf + sizeof(DWORD));

} /* END function pvVAaccess() */



/*+2 MODULE VA.C ===========================================================*/
/*   NAME   14                     pvVAread                                 */
/*== SYNOPSIS ==============================================================*/
PRIVATE void     *
pvVAread(pVA, dwArIndex)
    VACB      pVA;	            /* Pointer to VA-Control-Block struct */
    DWORD     dwArIndex;	    /* Index in VA for record to read */
{
/* DESCRIPTION
 *    Read a record in the virtual array (VA) :
 *
 *    0:   Check VA index out-of-range; If yes, return NULL-ptr (can't read).
 *    1:   Calc. cache buffer address (circular wrap) of VA rec# : "dwArIndex".
 *    2:   Retrieve current rec# in calculated cache buffer entry (1. field).
 *    3:   Test VA rec# "dwArIndex" against current rec# in the cache buffer :
 *         3.1: If VA-rec present, return address of rec. contents (2. field),
 *         3.2: If buffer slot occupied by other elem, simply read new elem &
 *              Return address of rec. contents (2. field of cache buf.entry).
 * RETURN
 *    Function return value: Pointer to record content (in cache-buffer), -
 *                           NULL if error (index out-of-range).
 *-2*/

    WORD     wBfIndex    = 0;    /* Cache buffer addr. of VA-rec : "dwArIndex" */
    DWORD    dwBfArIndex = 0L;   /* VA-rec curr. occupying cache for dwArIndex */
    char     *pcBf       = NULL; /* Generic ptr. to cache buffer element */


    /* 0: Check for VA index out-of-range; if yes, return NULL ptr. */
    if (dwArIndex >= pVA->stSize.dwArSize)
	return (void *) (NULL);


    /* 1: Calculate cache buffer address (pcBf) of VA record (dwArIndex) */
    wBfIndex = (WORD) (dwArIndex % (DWORD)pVA->wBfSize);
    pcBf = pVA->pcBf + wBfIndex * pVA->wBfElSize;


    /* 2: Retrieve VA record currently in calculated cache buffer (dwBfArIndex) */
    dwBfArIndex = *(DWORD *) pcBf;


    /* 3.1: If dwArIndex present in buffer, return address of record contents */
    if (dwBfArIndex == dwArIndex)
	return (void *) (pcBf + sizeof(DWORD));


    /* 3.2: Else (slot free or occupied by other), simply read new elem! */
    (void) fseek(pVA->pfArFile, HEADER + (dwArIndex * pVA->stSize.wArElSize), SEEK_SET);
    (void) fread(pcBf + sizeof(DWORD), pVA->stSize.wArElSize, 1, pVA->pfArFile);

    /* Reset buf.addr. of current rec. & Return addr. of rec.contents */
    *((DWORD *) pcBf) = dwArIndex;
    return (void *) (pcBf + sizeof(DWORD));

} /* END function pvVAread() */



/* END module va.c                                                          */
/*==========================================================================*/
