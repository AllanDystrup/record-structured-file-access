/*+1========================================================================*/
/*   MODULE                       GETOPT.C                                  */
/*==========================================================================*/
/*   FUNCTION      Function getopt() in module GETOPT.C gets the next option
 *                 letter from the command line; The function is an enhanced
 *                 version of the UNIX C Library function, GETOPT(3).
 *
 *   SYSTEM        Standard (ANSI/ISO) C.
 *                 Tested on PC/MS DOS V.5 & UNIX SVR3, SVR4
 *
 *   SEE ALSO      Modules : GENERAL.H, GETOPT.H
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, august 1991
 *
 *   VERSION       $Header: d:/cwork/index/RCS/getopt.c 0.1 92/08/18 13:45:36
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 -----------------------------------------------------------
 *                 $Log:	getopt.c $
 *                 Revision 0.1  92/08/18  13:45:36  Allan_Dystrup
 *                 PREREL (ALFA1)
 *
 *   REFERENCES
 *
 *   USAGE         option = getopt (argc, argv, optstring);
 *                 <argc>      # arguments in <argv>
 *                 <argv>      Argument value array, i.e., an array of pointers
 *                             to the "words" extracted from the command line.
 *                 <optstring> The set of recognized options.  Each character in
 *                             the string is a legal option; any other character
 *                             encountered as an option in the command line is
 *                             an illegal option and an error message is displayed.
 *                             If a character is followed by a colon in <optstring>,
 *                             the option expects an argument.
 *
 *   RETURN        <option>    Returns the next option letter from the cmd line :
 *                             '?'     is returned in the case of an illegal
 *                                     option letter or a missing option argument.
 *                             NONOPT  is returned if a non-option argument is
 *                                     is encountered or the command line scan is
 *                                     completed (also see <optarg> below for
 *                                     both cases).
 *                 <optarg>    Returns the text of an option's argument or of
 *                             a non-option argument.  NULL is returned if an
 *                             option has no argument or if the command line
 *                             scan is complete. For illegal options or missing
 *                             option arguments, OPTARG returns a pointer to
 *                             the trailing portion of the defective ARGV.
 *
 *   CONTROL       <opterr>    Controls whether or not GETOPT prints out an
 *                             error message upon detecting an illegal option
 *                             or a missing option argument.  A non-zero value
 *                             enables error messages; zero disables them.
 *                 <optind>    Is the index in ARGV of the command line argument
 *                             that GETOPT will examine next.  GETOPT recognizes
 *                             changes to this variable.  Arguments can be skipped
 *                             by incrementing OPTIND outside of GETOPT and the
 *                             command line scan can be restarted by resetting
 *                             OPTIND to either 0 or 1.
 *
 *-1========================================================================*/


/* =========================================================================*/
/*                            Includes                                      */
/* =========================================================================*/

/* Standard c (ANSI/ISO) headerfiles */
#include  <stdio.h>            /* STDC I/O definitions. */
#include  <string.h>           /* STDC String functions. */

/* Project headerfile */
#define _GETOPT_ALLOC
#include  "getopt.h"           /* GETOPT(3) definitions. */



/* =========================================================================*/
/*                            Global var's                                  */
/* =========================================================================*/

/* Private variables. */
PRIVATE int iEndOptind   = 0;  /* Position of "--" in group */
PRIVATE int iLastOptind  = 0;  /* Last index in argv[] */
PRIVATE int iGroupOffset = 1;  /* Offset in argv[current] */



/*+2 MODULE GETOPT.C =======================================================*/
/*   NAME   01                 getopt()                                     */
/*== SYNOPSIS ==============================================================*/
PUBLIC int
getopt(argc, argv, pzOptStr)
    int     argc;              /* Argument count */
    char    **argv;            /* Argument vector */
    char    *pzOptStr;         /* String of valid options */
{
/* DESCRIPTION
 *
 *   1: Check if caller has restarted or advanced the scan by modifying <optind>
 *   2: Scan command line & retrieve next option and/or argument ... :
 *      2.1: Retrieve NON-OPTION ARGUMENT from <optarg>, ie. :
 *           no option marker "-", or past end-of-options indicator "--"
 *      2.2: Retrieve OPTION from group (marked "-") w. possible ARGUMENT.
 *           2.2.1: If END-OF-GROUP, advance to next group, ie. slot in argv[]
 *           2.2.2: Retrieve option to <option>
 *           2.2.3: If END-OF-OPTIONS, set indicator & advance to next group
 *           2.2.4: Check option, - if invalid print error message & return '?'
 *      2.3: Retrieve OPTION ARGUMENT to <optarg>, - if any.
 *           2.3.1: Flush-up argument : rest of current <argv>
 *           2.3.2: Separate argument : whole of next <argv>
 *   3: Return next option or (if none) next non-option arg.
 *-2*/

    char     *pzGroup = NULL;  /* Current argument string : argv[current] */
    char     *s       = NULL;  /* Pointer for locating option in pzOptStr */
    char     cOption  = ' ';   /* Option retrieved from pzGroup */


    /* 1: Reset pointers, if caller forced restart or advance of the scan */
    /* Scan restart */
    if (optind <= 0) {
       iEndOptind  = 0;
       iLastOptind = 0;
       optind = 1;
    }

    /* Scan advance */
    if (optind != iLastOptind)
       iGroupOffset = 1;


    /* 2: Scan command line & retrieve next option and/or argument */
    for (cOption = ' ', optarg = NULL;
         optind < argc;
         optind++, iGroupOffset = 1, cOption = ' ') {

       pzGroup = argv[optind];


       /*---------------------------------------------------------------------*/
       /* 2.1: Retrieve NON-OPTION ARGUMENT from <optarg>                     */
       /* ie.: no option marker "-", or past end-of-options indicator "--"    */

       if ( (pzGroup[0] != '-') ||
            ((iEndOptind > 0) && (optind > iEndOptind))) {

           if (optind == iLastOptind)
               continue;

           /* Return NONOPT (cOption = ' ') and argument. */
           optarg = pzGroup;
           break;
       }


       /*---------------------------------------------------------------------*/
       /* 2.2: Retrieve OPTION from pzGroup (marked "-") w. possible ARGUMENT */

       /* 2.2.1: If END-OF-GROUP, advance to next pzGroup, ie. slot in argv[] */
       if (iGroupOffset >= (int) strlen(pzGroup))
           continue;

       /* 2.2.2: Retrieve option to <cOption> */
       cOption = pzGroup[iGroupOffset++];

       /* 2.2.3: If END-OF-OPTIONS, set indicator & advance to next pzGroup */
       if (cOption == '-') {
           iEndOptind = optind;    /* Mark end-of-options position. */
           continue;
       }

       /* 2.2.4: Check option, - if invalid print error message & return '?' */
       s = strchr(pzOptStr, cOption);
       if (s == NULL) {
           if (opterr)
               (void) fprintf(stderr, "%s: illegal option -- %c\n",
                              argv[0], cOption);

           /* Return '?' and offending argument */
           cOption = '?';
           optarg  = &pzGroup[iGroupOffset - 1];
           break;

       }


       /*---------------------------------------------------------------------*/
       /* 2.3: Retrieve OPTION ARGUMENT to <optarg>, - if any                 */

       /* Option expecting an argument ? */
       if (*++s == ':') {

           /* 2.3.1: Flush-up argument : rest of current <argv> */
           if (iGroupOffset < (int) strlen(pzGroup)) {
               optarg = &pzGroup[iGroupOffset];
               iGroupOffset = strlen(pzGroup);
           }
           else {
           /* 2.3.2: Separate argument : whole of next <argv> */
               if ((++optind < argc) && (*argv[optind] != '-')) {
                   optarg = argv[optind];
               }
               else {
                   if (opterr)
                       (void) fprintf(stderr, "%s: option requires an argument -- %c\n",
                           argv[0], cOption);
                   cOption = '?';
                   optarg = &pzGroup[iGroupOffset - 1];
                   iGroupOffset = 1;
               }

           }
           break;

       } /* END[2.3] Retrieve OPTION ARGUMENT */
       break;

    } /* END[2]: cmd.line scan */


    /* 3: Return the option and ("optionally") its argument. */
    iLastOptind = optind;
    return ((cOption == ' ') ? NONOPT : (int) cOption);

} /* END function getopt() */



/* END of module GETOPT.C                                                   */
/*==========================================================================*/
