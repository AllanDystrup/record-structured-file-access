/*+1========================================================================*/
/*   MODULE                       INDEX.C                                   */
/*==========================================================================*/
/* FUNCTION    Creates an indexfile from a sequential/flat datafile;
 *             Compiles to an object-module with PUBLIC accessible functions
 *             or a stand alone utility program taking the following options:
 *              - file names of input/data-file and output/index-file
 *              - a character for identifying the key in a datarecord
 *              - the size of the keystring (#byte)
 *              - the initial size of the indexfile (expected max #records)
 *
 * SYSTEM      Standard Ansi C.
 *             Tested on PC/MS DOS V3.3 (MSC 6.00A) and UNIX SYS V.3 (GNU gcc)
 *
 * SEE ALSO    Modules : ACCESS.H, VA.H/C, HASH.H/C
 *
 * PROGRAMMER  Allan Dystrup
 *
 * COPYRIGHT   (c) Allan Dystrup, Kommunedata I/S 1991
 *
 * VERSION     $Header: d:/cwork/index/RCS/index.c 0.1 92/07/10 13:57:48
 *             Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *             ---------------------------------------------------------------
 *             $Log:	index.c $
 *             Revision 0.1  92/07/10  13:57:48  Allan_Dystrup
 *             PREREL (ALFA1)
 *
 * USAGE       Module index.c provides a "high level" interface for building
 *             an indexfile from a record structured datafile, using one of
 *             several predefined index-access methods. The actual access-
 *             strategy is choosen by a compiletime switch :
 *                -DVA : build a VirtualArray index (keyvalue = entry/slot in VA)
 *                -DSS : build a ScatterStorage index (keyvalue hash'ed to entry)
 *
 *             The API offered by module index.c consist of 2 functions for
 *             generating resp. checking an indexfile :
 *                eIdxMake   // Create indexfile and insert (key,offset) values
 *                eIdxTest   // Retrieve datarec's via index lookup key->offset
 *
 *             The module index.c may be compiled without main()-function
 *             for direct access to the API from a user program. Alternatively
 *             compiling the module with switch -DMAIN will build an executable
 *             program with a command line interface (function vIdxDefine) for
 *             generating indexfiles from datafiles.
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
 *             The sections embedded in <#ifdef SS ... #endif> apply only
 *             to the ScatterStorage ("HASH") access method, which requires
 *             index expansion by a total rehash of all keys. This might be
 *             made transparent by checking for expansion in (*peKeyInsert).
 *
 *
 *=========================== MODULE STRUCTURE ===============================
 *
 *
 *                                Data Model
 * The index functions are implemented simply as a sequence of calls of the
 * appropriate access functions (using the generic access API). This design
 * does not require any new datastructures (besides those offered by the
 * access module, - cf. va.h and hash.h).
 *
 *
 *                           Function Decomposition
 * The functions of module index.c may be grouped into two (three) levels :
 * (1) Main function (activated by compile switch -DMAIN), turning the
 *     module into a utility program with a simple command-line interface
 *     for building indexfiles from datafiles.
 * (2) High level functions providing the basic index generation API for
 *     use by main() - and possibly direct use by linking into user programs
 *     (when the module is compiled to .OBJ, ie. without -DMAIN).
 * [3] The index-generation functions calls the generic access API to perform
 *     the actual indexfile I/O (ie. creating, updating, reading etc the index).
 *     The generic access API is mapped to an actual access method, by inclusion
 *     of a headerfile [va.h | hash.h], as indicated by a compiletime switch :
 *     [-DVA | -DSS], - cf. the project makefile.
 *        
 * In this way the "high level" index generation code is made independent of
 * the "low level" index access strategy, gaining flexibility in choice of
 * access method, but retaining total transparancy for user programs.
 *
 * The calling hierachy is illustrated in the following diagram, where the
 * signatures :
 *     -> and <-   indicate input resp. output parameters to functions
 *    < > and (_)  indicate choice resp. repetition in function call
 *
 * ..........................................................................
 *  LEVEL 1
 *  Main                          main()
 *  Utility Pgm.                  | |  | 
 *              __________________| |  |        
 *              |                   |  |
 *              |                   | < > -> -t
 *          vIdxDefine              |  |__________________
 *              |                   |                    |
 *            getopt                |                    |
 * .................................|....................|...................
 * LEVEL 2                          |                    |
 * Index Generation                 |                    |
 * API                          eIdxMake---+      +---eIdxTest
 *                              |   |      |      |      |   |
 *                              |  (_)     |      |      |  (_)
 *                              |   |    eIdxStatPrint   |   |
 *                              |   |         |          |   |
 * .............................|...|.........|..........|...|...............
 * [LEVEL 3]                    |   |         |          |   |
 * Index Access                 |   |         |          |   |
 * API (generic)     (*peIdxCreate) |         | (*peIdxOpen) |
 *                       (*peKeyInsert)       |     (*peKeyFind)
 *                              |   |         |          |   |
 *                              |   |  (*peIdxGetSize)   |   |
 *                              |   |  (*peIdxGetLoad)   |   |
 *                              |   |         |          |   |
 *                              +---+---------+----------+---+
 *                                            |
 *                                           < >
 * Map to actual                        -DVA  |  -DSS
 * Access Method                    +---------+---------+
 * (compile switch)                 |                   |
 *                               [VA.H]             [SS.H]
 *
 *-1========================================================================*/
 


/*==========================================================================*/
/*                                Includes                                  */
/*==========================================================================*/

/* Standard C headerfiles */
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

/* #define S/H-DEBUG: runtime check of stack- and heap on DOS  */
/* Relies on PC/MS DOS V.3.3 system files which are deprecated */
//#include "../check/stck/stck.h"
   # define STCK(x)
   # define REAL_MAIN	main
//#include "../check/hpck/hpck.h"


/* Module headerfile */
#define _INDEX_ALLOC
#include "index.h"
#include "UTIL/getopt/getopt.h"



/*==========================================================================*/
/*                           Function prototypes                            */
/*==========================================================================*/
PRIVATE   void
          vGetkey   P((int Ksz, char *Kbf));

PRIVATE   WORD
          wKeyPrint P((char *pzKeyBuf, DWORD dwDatOff));

PRIVATE   eRetType
          eIdxStatPrint P((ITYPE *hIdx));



/*==========================================================================*/
/*                      Manifest constants & global var's                   */
/*==========================================================================*/
#define  MAXNAME    12                  /* Max length of filename (in DOS)  */
#define  MAXLINE    1025                /* Max length of line in datafile.  */
#define  DUMMY      (eRetCode=eRetCode) /* Dummy-expr for error-check macro */
PRIVATE  char aIdxFile[MAXNAME+1]= " "; /* Name of indexfile,- can't preset */
PRIVATE  int  iKeySize = 0;             /* Keysize (# bytes),- can't preset */
PRIVATE  FLAG fVerbose = FALSE;         /* Verbose trace of index-generat.? */




#ifdef MAIN
/****************************************************************************/
/******************************** MAIN **************************************/
/****************************************************************************/

PRIVATE void
        vSigCatch P((int iSigNum));


/* Default values for index generation, - overridden by cmd line options! */
PRIVATE char aDatFile[MAXNAME+1] = " "; /* Name of datafile, - can't preset */
PRIVATE BYTE bKMark   = '\020';         /* Unique mark indicating start of key */
                                        /* (Default: 16 = 0x10 = 020 = '') */
PRIVATE int  iIdxSize = 100;            /* Initial reasonable index sixe */
PRIVATE FLAG fTest    = FALSE;          /* Test index after generation ? */


/* Allocate an "incore" ITYPE descriptor structure w. handle */
PRIVATE ITYPE   pI    = NULL;           /* Ptr to "incore" index descr. */
PRIVATE ITYPE  *hI    = &pI;            /* Handle (ptr.addr) of incore descr */


/* Define "signon message" for index generation program */
PRIVATE const char SIGNON[] =
   "\nKMD Index Generator, Version 0.1.0\n"
   "MOD[index.c] VER[0.1.0 Pre] DAT[92/07/10] DEV[ad dec]\n"
   "Copyright (c) KommuneData I/S 1992\n\n";



/*+1 MODULE INDEX.C ========================================================*/
/*   NAME   01                    MAIN                                      */
/*== SYNOPSIS ==============================================================*/
int
REAL_MAIN(argc, argv, envp)
    int argc;                  /* Argument count */
    char *argv[];              /* Argument vector */
    char *envp[];              /* Environment pointer */
{
/* DESCRIPTION
 *    Testdriver and main function for module index.c; - exercises the
 *    functions in the module and validates the functionality through 
 *    trace-statements (when compiled with flag "-DDEBUG").
 *
 *    1: Print signon message & setup to catch "break" signals.
 *    2: Get options (from the command line) defining the index to create.
 *    3: Call eIdxMake() to generate an index file from the datafile.
 *   [4: Optionally call eIdxTest() to test the index file.]
 *
 * RETURN
 *    1 (OK)..., when function completed succesfully.
 *    0 (ERROR), otherwise.
 *-1*/

    /* 1: Signon and setup signal catcher */
    fputs(SIGNON, stdout);

    (void) signal(SIGINT, vSigCatch);
    (void) signal(SIGTERM, vSigCatch);


    /* 2: Get options (from the command line) defining the index to create; - */
    /* cf Module <GETOPT.H/.C> for a description of the function & parameters */
    vIdxDefine(argc, argv, "k:K:h:H:m:M:d:D:i:I:tTvV");


    /* 3: Make an index file from the datafile */
    ICHK_ERR((void) eIdxMake(hI, aDatFile, (WORD) bKMark, iKeySize, aIdxFile, iIdxSize), I_CONT)


    /* 4: Optionally test the index file */
    if (fTest)
       ICHK_ERR((void) eIdxTest(hI, aDatFile, (WORD) bKMark), I_CONT)


    return (OK);

}   /* END function REAL_MAIN() */



/*+3 MODULE INDEX.C --------------------------------------------------------*/
/*   NAME   01.01                 vIdxDefine                                */
/*-- SYNOPSIS --------------------------------------------------------------*/
PUBLIC void
vIdxDefine(argc, argv, optstring)
    int argc;                  /* Argument count */
    char *argv[];              /* Argument vector */
    char *optstring;           /* String defining valid options */
{
/* DESCRIPTION
 *    Support routine for main() driver :
 *    Parse commandline, and translate arguments to internal variables.
 *
 *    0: Check arguments: require at least 2 (keysize & inputfile)
 *    1: LOOP:   // Parse command line
 *          1.1: Retreive next option from cmd.line to <option>,<optarg>
 *          1.2: Transfer/Convert <option>,<optarg> to program variables
 *       ENDLOOP
 *    2: If invalid cmdline, print "usage" & exit with error, else return. 
 *-3*/

    register int option = 0;           /* Next option letter from cmd. line */
    FLAG         fError = FALSE;       /* Flag for error in program options */


    /* 0: Validate args; - Require at least 2 (keysize & inputfile) */
    fError = (argc < 3 ? TRUE : FALSE);


    /* 1: LOOP parse commandline for all options ... */
    while ( ((option = getopt(argc, argv, optstring)) != NONOPT)
            || (optarg != NULL) )   {


       /* 1.1: Retreive next option from cmdline to <option>,<optarg> */
       D(printf("Option = '%c'    Argument = \"%s\"\n", option,
                (optarg == NULL ? "<empty>" : optarg)));


       /* 1.2: Transfer/Convert <option>,<optarg> to pgm. variables */
       switch (option) {

          case 'k':   case 'K':           /* Size (#byte) of keystring */
             iKeySize = atoi(optarg);
             break;

          case 'h': case 'H':             /* Startsize (#rec) of index */
             iIdxSize = atoi(optarg);
             break;

          case 'm': case 'M':             /* Unique char marking keystart */  
             bKMark = (BYTE) *optarg; 
             break;

          case 'd': case 'D':             /* Name of data-file */
             strncpy(aDatFile, optarg, MAXNAME);
             break;

          case 'i': case 'I':             /* Name of index-file */
             strncpy(aIdxFile, optarg, MAXNAME);
             break;

          case 't': case 'T':             /* Test index after generation ? */
             fTest = TRUE;
             break;

          case 'v': case 'V':             /* Trace index generation on VDU? */
             fVerbose = TRUE;
             break;

          default:                        /* Should not occur! */
             /*FALLTHROUGH*/
          case '?':                       /* Illegal option or missing arg */
             /*FALLTHROUGH*/
          case NONOPT:                    /* Illegal arg or end-of-scan */
             fError = TRUE;
             break;
        }

    } /* END [1]: LOOP parse commandline ... */


    /* 2: If invalid cmdline, print "usage" & terminate */
    if (fError) {
       fprintf(stderr, "\nAnvendelse: indexX -d fi [-m c] -k # [-i fo] [-h #] [-v] [-t]\a\n");
       fprintf(stderr, "hvor  X     er memory model : [S|M|L], typisk L\n");
       fprintf(stderr, "     -d fi  angiver input datafil fil : 'fi'\n");
       fprintf(stderr, "    [-m c]  angiver m�rketegn : 'c' (i pos 1) for n�gle\n");
       fprintf(stderr, "            default '' (ie. 16=0x10=020)\n");
       fprintf(stderr, "     -k #   angiver l�ngde (# byte) af n�gle\n");
       fprintf(stderr, "    [-i fo] angiver output indexfil : 'fo'\n");
       fprintf(stderr, "            default input datafil med extension .idx\n");
       fprintf(stderr, "    [-h #]  angiver index startst�rrelse (# records)\n");
       fprintf(stderr, "            default 100, - udvides dynamisk\n");
       fprintf(stderr, "    [-v]    angiver om index-generering skal f�lges ('verbose')\n");
       fprintf(stderr, "            default IKKE aktiveret\n");
       fprintf(stderr, "    [-t]    angiver om index skal testes efter generering\n");
       fprintf(stderr, "            default IKKE aktiveret\n\n");
       fprintf(stderr, "Eksempel:   indexL -k5 -h6144 -d myfile.dat -v -m!\n\n");
       exit(EXIT_FAILURE);
    }

}   /* END function vIdxDefine() */



/*+4 MODULE INDEX.C --------------------------------------------------------*/
/*   NAME   01.02                 vSigCatch                                 */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vSigCatch(iSigNum)
     int iSigNum;
{
/* DESCRIPTION
 *    Support function for main() driver :
 *    Signal handler set up to catch the "break" signals : SIGINT (asynch.
 *    interactive attention) & SIGTERM (asynch. interactive termination).
 *
 *    1: Prompt user for break confirmation; We have two possible situations:
 *       a: break during index generation (INTEGRITY="UNKNOWN"), - will leave
 *          the index in a not fully generated state, possibly unusable.
 *          Issue warning if this situation !
 *       b: break during index test (INTEGRITY="OK"), - no problem!
 *    2: Depending on user confirmation : [Y]->terminate or [N]->continue.
 *
 * RETURN
 *    If break confirmed: program terminated with exit code 'EXIT_FAILURE'
 *    else: signal 'iSigNum' reset and program execution resumed.
 *
 * BUGS
 *    Asynch. signals don't guarantee access to volatile data at sequence pts;
 *    Since we restrict our access to READ operations, this shouldn't pose any
 *    problem, - though not strictly ANSI (cf. type sig_atomic_t).
 *-4*/

    char      pzLine[MAXLINE];         /* Line buffer */


    /* 1: Prompt user for break confirmation; Issue warning if situation a! */
    fprintf(stderr, "\nAFBRYDELSE:\tSignal [%d] modtaget\n", iSigNum);
    fprintf(stderr, "\tTilstanden af indexfil er p.t. : INTEGRITY [%s]\n",
                    (pI->indexmode == RW ? "UNKNOWN" : "OK"));
    fprintf(stderr, (pI->indexmode == RW ? "\tIndex'et er i den nuv�rende tilstand ufuldst�ndigt!\n\t" : "\t"));
    fprintf(stderr, "Skal programmet stoppes? [J|N] => ");


    /* 2: Depending on user answer: Terminate[J] or continue [N] */
    (void) fgets(pzLine, MAXLINE, stdin);
    if (strchr(pzLine,'j') != NULL || strchr(pzLine, 'J') != NULL) {
       /* 2.1: Terminate program "gracefully" */
       (void) (*peIdxClose) (hI);
       exit(EXIT_FAILURE);
    }
    else
       /* 2.2: Continue : reset signal, and continue */
       signal(iSigNum, vSigCatch);


}   /* END function vSigCatch() */

#endif   /* ifdef MAIN */




/****************************************************************************/
/******************************** INDEX *************************************/
/****************************************************************************/


/*+2 MODULE INDEX.C ========================================================*/
/*   NAME   02                    eIdxMake                                  */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eIdxMake(hINDEX, pzDatFile, wKeyMark,
         wKeySize, pzIdxFile, dwIdxSize)
ITYPE   *hINDEX;        /* Handle (ptr-to-address) of incore struct */
char    *pzDatFile;     /* Name of the datafile for the index */
WORD    wKeyMark;       /* Tag indicating start of key (nb: WORD) */
WORD    wKeySize;       /* Size of one key (# byte) */
char    *pzIdxFile;     /* Name of the indexfile to create */
DWORD   dwIdxSize;      /* #keyrecs. of the index to generate */
{
/* DESCRIPTION
 *    Make and fill a new index (using the incore descriptor handle hINDEX)
 *    from a specified datafile (named pzDatFile). The format of the datafile
 *    MUST comply with the following specification :
 *     - the file is a sequence of sequentially stored records
 *     - each record consists of an arbitrary number of lines
 *     - each line is at most MAXLINE characters long
 *     - the first line in each datarecord contains :
 *       1. position .....: a common tag "wKeyMark", indicating start of a key
 *       2-(wKeySize+1)...: a unique key value, wKeySize byte long;
 *                          If the key is shorter than wKeySize, it is
 *                          silently padded at the end with spaces.
 *                          If the key is longer than wKeySize, it is
 *                          silently "chopped off" to wKeySize length.
 *    The index is initially generated with room for "dwIdxSize" key-
 *    records, but it is dynamically resized (expanded one or more times)
 *    as required by the repeated (*peKeyInsert)()-operations.
 *
 *    1: Opens the input datafile <pzDatFile>, mode ReadOnly
 *    2: Sets up the name for the output indexfile <pzIdxFile>
 *       (using default name : "datafile".idx)
 *    3: Makes the output indexfile (mode ReadWrite), by :
 *       3.1: Creating (and opening) the file
 *       3.2: Printing initial statistics for the indexfile (size, use-factor)
 *       3.3: LOOP Reading the datafile one line at a time (MAXLINE chars);
 *                 For each line starting with a keyword :
 *              -  Getting the file offset (#byte from file start)
 *              -  Entering a new keyrecord (key,offset) into the index file
 *                 (Expanding and reporting the index filesize if nessecary)
 *            ENDLOOP
 *       3.4: Printing final statistics for the completed indexfile
 *       3.5: Closing the index file (thereby ensuring the file integrity)
 *    4: Closes the input datafile.
 *
 * RETURN
 *    OK..., when function completed succesfully.
 *    ERROR, otherwise.
 *-2*/

    BYTE     bKeyMark     = (BYTE)wKeyMark;/* Unique mark (undo C-promotion) */
    FILE     *fdDatFile   = NULL;          /* Filehandle for datafile */
    register DWORD  dwDatFilePtr = 0L;     /* Flatfile ptr into the datafile */
    char     aDatBuf[MAXLINE];             /* Line buffer for datafile */
    char     *pDat        = NULL;          /* Ptr. for datafile name */
    char     *pIdx        = NULL;          /* Ptr. for indexfile name */
    WORD     wIdxLoad     = 0;             /* Index load factor */
    eRetType eRetCode     = ERROR;         /* Func. return code [ERROR | OK] */


    /* 1: Open the input datafile (mode RO) */
    fdDatFile = fopen(pzDatFile, "r");
    IRET_ERR(fdDatFile == NULL, I_DATOPEN, 200)


    /* 2: Set up default name for the output indexfile = "pzDatFile".idx */
    if (*pzIdxFile == ' ') {
       strncpy(pzIdxFile, pzDatFile, MAXNAME);
       if ((pIdx = strchr(pzIdxFile, '.')) == NULL)
          pIdx = pzIdxFile + strlen(aIdxFile);
       strcpy(pIdx, ".idx");
    }


    /* 3: Create (and open) the output indexfile, mode RW */
    if (fVerbose)
       fprintf(stdout, "Genererer indexfil til start-st�rrelse, vent venligst ...\n");


    /* 3.1: Create & open file */
    if ((*peIdxCreate) (hINDEX, aIdxFile, wKeySize, dwIdxSize) == OK) {


       /* 3.2: Print initial statistics for the (empty) indexfile */
       eRetCode = eIdxStatPrint(hINDEX);
       D(ICHK_ERR(DUMMY, I_CONT))
       IRET_ERR(eRetCode == ERROR, I_INDEX, 201)


       /* 3.3: LOOP : Read datafile one line at a time ... */
       for (dwDatFilePtr = (DWORD) 0L;
            fgets(aDatBuf, MAXLINE - 1, fdDatFile) != (char *) NULL;
            dwDatFilePtr = (DWORD) ftell(fdDatFile)) {

          /* For each line starting with a "bKeyMark" (in 1. position) */
          if ((BYTE) *aDatBuf == bKeyMark) {

             /* Keystring now in start of linebuffer; - Chop off at wKeySize */
             aDatBuf[1 + wKeySize] = '\0';

             /* Skip to end of key-string */
             for (pDat=aDatBuf; (*pDat!='\r') && (*pDat!='\n') && (*pDat!='\0'); pDat++)
                /* skip to end-of-string */ ;

             /* Pad with trailing spaces */
             while (pDat <= (aDatBuf + wKeySize))
                *pDat++ = ' ';

             if (fVerbose)
                fprintf(stdout, "Entering: KEY[%s] <-> OFFSET[%lu]               \r",
                                 aDatBuf + 1, dwDatFilePtr);

             /* Enter new keyrecord (=key,offset) into the index file */
             eRetCode = (*peKeyInsert) (hINDEX, aDatBuf + 1, dwDatFilePtr);

             /* Check for error (issue warning for duplicate key) */
             D(ACHK_ERR(DUMMY, A_CONT))
             if (eRetCode == ERROR && Astat == A_DUPLICATE) {
                aDatBuf[1 + wKeySize] = '\0';
                printf("\nDUPKEY : [%s]\n", aDatBuf + 1);
             } else
                IRET_ERR(eRetCode == ERROR, I_INDEX, 202)

#ifdef SS
             /* Expand index file, if nessecary (ie. SS-index > 80% full) */
             (void) (*peIdxGetLoad) (hINDEX, &wIdxLoad);
             if (Astat == A_XPAND) {

                /* Double the index size */
                if (fVerbose)
                   fprintf(stdout, "Expanding indexfile to double size, please wait ...\n");
                eRetCode = (*peIdxResize) (hINDEX, 200);
                D(ACHK_ERR(DUMMY, A_CONT))
                IRET_ERR(eRetCode == ERROR, I_INDEX, 203)

                /* Print new statistics after the index expansion */
                eRetCode = eIdxStatPrint(hINDEX);
                D(ICHK_ERR(DUMMY, I_CONT))
                IRET_ERR(eRetCode == ERROR, I_INDEX, 204)

             } /* END if Astat==A_XPAND */
#endif /*SS*/

          } /* END For each line starting with a "bKeyMark" (in 1. position) */

       } /* END [3.3]: LOOP Read datafile one line at a time */


       /* 3.4: Print the final statistics for completed index */
       eRetCode = eIdxStatPrint(hINDEX);
       D(ICHK_ERR(DUMMY, I_CONT))
       IRET_ERR(eRetCode == ERROR, I_INDEX, 205)


       /* 3.5: Close the indexfile */
       eRetCode = (*peIdxClose)(hINDEX);
       D(ACHK_ERR(DUMMY, A_CONT))
       IRET_ERR(eRetCode == ERROR, I_INDEX, 206)

    } /* END [3]: Create and open output indexfile */
    else {
       /* (*peIdxCreate)() == ERROR */
       /* Report error during creation of index file */
       D(ACHK_ERR(DUMMY, A_CONT))
       IRET_ERR(TRUE, I_INDEX, 207)
    }


    /* 4: Close the datafile */
    fclose(fdDatFile);


    /* Return status to caller */
    STCK("eIdxMake");
    IRET_OK

}   /* END function eIdxMake() */



/*+2 MODULE INDEX.C=========================================================*/
/*   NAME   03                    eIdxTest                                  */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eIdxTest( hINDEX, pzDatFile, wKeyMark)
    ITYPE   *hINDEX;        /* handle (ptr->addr) of incore indexstruct */
    char    *pzDatFile;     /* name of datafile for which to make an index */
    WORD    wKeyMark;       /* unique mark indicating start of key */
{
/* DESCRIPTION
 *    Test an index by repeated key lookup operations :
 *
 *    1: Opens the datafile <pzDatFile>, mode ReadOnly.
 *    2: Sets up default name for the indexfile = "pzDatFile".idx
 *    3: Tests the indexfile by :
 *       3.1: Opening the indexfile <aIdxFile[]>, mode ReadOnly.
 *       3.2: Printing initial statistics for the indexfile
 *       3.3: LOOP Reading keyvalue (from stdin/keybd or a random generator) :
 *              -  Lookup datafile offset in index file
 *              -  Read & print key- & datarecord
 *            ENDLOOP // while more input (ie. keystring not empty)
 *    4: Closes the index- & data-files
 *
 * RETURN
 *    OK..., when function completed succesfully.
 *    ERROR, otherwise.
 *-2*/

    BYTE     bKeyMark     = (BYTE)wKeyMark;/* Unique mark (undo C-promotion) */
    FILE     *fdDatFile   = NULL;          /* Filehandle for datafile */
    DWORD    dwDatFilePtr = 0L;            /* Flatfile ptr into the datafile */
    char     aDatBuf[MAXLINE];             /* Line buffer for datafile */
    char     *pIdx        = NULL;          /* Ptr. for indexfile name */
    eRetType eRetCode     = ERROR;         /* Func. return code [ERROR | OK] */


    /* 1: Open the datafile <pzDatFile>, mode RO */
    fdDatFile = fopen(pzDatFile, "r");
    IRET_ERR(!fdDatFile, I_DATOPEN, 300)


    /* 2: Set up default name for the indexfile <aIdxFile[]> = pzDatFile.idx */
    if (aIdxFile[0] == ' ') {
       strncpy(aIdxFile, pzDatFile, MAXNAME);
       if ((pIdx = strchr(aIdxFile, '.')) == NULL)
          pIdx = aIdxFile + strlen(aIdxFile);
       strcpy(pIdx, ".idx");
    }


    /* 3: Test lookup of datafile records, using the indexfile ... */

    /* 3.1: Open the indexfile (mode RO) */
    eRetCode = (*peIdxOpen) (hINDEX, aIdxFile, "rb");
    D(ACHK_ERR(DUMMY, A_CONT))
    IRET_ERR(eRetCode == ERROR, I_INDEX, 301)


    /* 3.2: Print initial statistics for the indexfile */
    eRetCode = eIdxStatPrint(hINDEX);
    D(ICHK_ERR(DUMMY, I_CONT))
    IRET_ERR(eRetCode == ERROR, I_INDEX, 302)
#ifdef  SS
    if (fVerbose)
       (void) eHashIdxProcess(hINDEX, wKeyPrint);
#endif /*SS*/


    /* 3.3: LOOP while more input : */
    /* Enter keyvalue, lookup in index file, print key- & datarec. */
    do {
       /* 3.3.1: Get key value (from stdin or a random generator) */
       vGetkey(iKeySize, aDatBuf);


       /* 3.3.2: Lookup keyrecord in index file, print key- & datarecord */
       eRetCode = (*peKeyFind) (hINDEX, aDatBuf, &dwDatFilePtr);
       D(ACHK_ERR(DUMMY, A_CONT))


       /* If keyrecord found ... */
       if (Astat == A_OK) {

          /* Print keyrecord from index file : (key, flatfileoffset) */
          (void) wKeyPrint(aDatBuf, dwDatFilePtr);

          /* Lookup datarecord in datafile using flatfileoffset from keyrec */
          IRET_ERR(fseek(fdDatFile, dwDatFilePtr, SEEK_SET) != 0, I_DATSEEK, I_STOP)

          /* Print contents of datarecord on stdout, starting w. key-line */
          if (fgets(aDatBuf, 1023, fdDatFile) != (char *) NULL)
             fputs(aDatBuf, stdout);

          while ((fgets(aDatBuf, 1023, fdDatFile) != (char *) NULL) &&
                 (BYTE) *aDatBuf != bKeyMark)
             fputs(aDatBuf, stdout);

       } /* END If keyrecord found */

    } while (*aDatBuf != '\0');
    /* END do enter keyvalue (while keyvalue not empty) */


    /* 4: Close index- & data-file */
    eRetCode = (*peIdxClose) (hINDEX);
    D(ACHK_ERR(DUMMY, A_CONT))
    IRET_ERR(eRetCode == ERROR, I_INDEX, 303)

    IRET_ERR(fclose(fdDatFile) != 0, I_DATCLOSE, I_STOP)


    /* Return status to caller */
    STCK("eIdxTest");
    IRET_OK

}   /* END function eIdxTest() */



/****************************************************************************/
/******************************** UTILITY ***********************************/
/****************************************************************************/


/*+4 MODULE INDEX.C --------------------------------------------------------*/
/*   NAME   04                   vGetkey                                    */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vGetkey(Ksz, Kbf)
    int  Ksz;       /* Actual length of keystring */
    char *Kbf;      /* Ptr to buffer for entering keystring (at least 30 chr) */
{
/* DESCRIPTION
 *    Generate one keyvalue in buffer <Kbf[]>, with keylength <Ksz>;
 *    1.1: If compileswitch -DRANDOM set, generate a (pseudo)random keyvalue
 *    1.2: Else prompt for, and read a keyvalue from stdin.
 *
 * RETURN
 *    Sidefect: Generated keyvalue in buffer (var.param) <Kbf[]>, length <Ksz>.
 *-4*/

#ifdef RANDOM

    /* 1.1: Generate a (pseudo)random keystring, using std.C rand() function */
    int       k = Ksz;            /* Size of keystring (#byte) to generate */
    char     *p = Kbf;            /* Scan ptr. into keystring-buffer Kbf[] */

    while (k-- > 0)
       *p++ = (char) ((rand() % 10) + '0');

#else

    /* 1.2: Prompt user for keystring, and read it from stdin */
    fprintf(stdout, "\nEnter a key value %d chars -> ", Ksz);
    strncpy(Kbf, "                             ", Ksz+1); /* Clear keybuffer */
    (void) fgets(Kbf, Ksz+1, stdin);                      /* Read new keyval */
    while (fgetc(stdin) != '\n')
       /* eat rest of line */;

#endif


    /* "Clamp" (chop off) keystring in Kbf at length Ksz */   
    *(Kbf + Ksz) = '\0';

}   /* END function vGetkey() */



/*+4 MODULE INDEX.C --------------------------------------------------------*/
/*   NAME   05                    wKeyPrint                                 */
/*-- SYNOPSIS --------------------------------------------------------------*/
#define CONTINUE   1
#define MAXKEY     80
PRIVATE char pzKBuf[MAXKEY + 1];

PRIVATE   WORD
wKeyPrint(pzKeyBuf, dwDatOff)
    char     *pzKeyBuf;            /* Keystring, - not zero terminated */
    DWORD     dwDatOff;            /* Corresponding datafile offset */
{
/* DESCRIPTION
 *    Print one index key-record: (keyvalue,offset) on stdout; -
 *    1: Make sure key-string is zero-terminated
 *    2: Print an index-record, ie. corresponding (key-value,datafile offset)
 *
 * RETURN
 *    Return "Again" code, ie: [1] continue or [0] stop, cf (*peIdxProcess)();
 *    This function may be passed as parameter to function : *peIdxProcess()
 *    for looping through the index-file and printing the contents of all
 *    valid (ie. USED) indexrecords.
 *-4*/


    /* 1: Make sure key-string is zero-terminated */
    strncpy(pzKBuf, pzKeyBuf, iKeySize);
    *(pzKBuf + iKeySize) = '\0';


    /* 2: Print an index-record: corresponding (key-value,datafile offset) */
    fprintf(stdout, "Key[%s]-(lookup)->FlatfileOffset[%lu]\n", pzKBuf, dwDatOff);


    /* 3: Return "Again" code: [1] continue/[0] stop, - cf. (*peIdxProcess)() */
    return(CONTINUE);

}   /* END function wKeyPrint() */



/*+4 MODULE INDEX.C---------------------------------------------------------*/
/*   NAME   06                    eIdxStatPrint                             */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   eRetType
eIdxStatPrint(hIdx)
    ITYPE    *hIdx;        /* Handle for index descr.struct */
{
/* DESCRIPTION
 *    If FLAG fVerbose on :
 *    1: Print size- and
 *    2: load-statistics for index file.
 *
 * RETURN
 *    OK...: when function completed succesfully.
 *    ERROR: otherwise.
 *-4*/
    DWORD     dwSize   = 0L;       /* Indexfile size  (#keyrecords) */
    DWORD     dwUsed   = 0L;       /* Indexfile usage (#keyrecords in use) */
    WORD      wLoad    = 0;        /* Indexfile load factor: 100*dwUsed/dwSize */
    eRetType  eRetCode = ERROR;    /* Func. return code : [ERROR | OK] */


    if (fVerbose) {

       /* 1: Retrieve and print index size */
       eRetCode = (*peIdxGetSize)(hIdx, &dwSize, &dwUsed);
       D(ACHK_ERR(DUMMY, A_CONT))
       IRET_ERR(eRetCode == ERROR, I_INDEX, 400)
       fprintf(stdout, "Index keyrecords : Size=[%lu], Used=[%lu]\n", dwSize, dwUsed);


       /* 2: Retrieve and print index load */
       eRetCode = (*peIdxGetLoad)(hIdx, &wLoad);
       D(ACHK_ERR(DUMMY, A_CONT))
       IRET_ERR(eRetCode == ERROR, I_INDEX, 400)
       fprintf(stdout, "Index loadfactor : Load=[%d]\n", wLoad);
    }


    IRET_OK

}   /* END function eIdxStatPrint() */



/* END module index.c                                                       */
/*==========================================================================*/
