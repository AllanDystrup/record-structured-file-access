/*+1========================================================================*/
/* MODULE                          STCK.C                                   */
/*==========================================================================*/
/* FUNCTION    Simple routine for diagnosing a corrupted stack :
 *             verify that it is possible to trace back to the stacktop
 *             and count the number of frames that is traversed.
 *
 * SYSTEM      PC/MS DOS, Tested on MS DOS V.3.3.
 *             Compilers MSC 6.0 (nb: MS proprietary _far keyword)
 *             The routine will only work with compilers, that use the "stan-
 *             dard" frame structure in their setup (TURBO C, MSC et.al.)
 *
 * SEE ALSO    Module STCK.H
 *
 * PROGRAMMER  Allan Dystrup
 *             Patterned after routines by D. Michmerhuizen and Roberd Ward.
 *
 * COPYRIGHT   (c) Allan Dystrup, Kommunedata I/S 1991
 *
 * VERSION     $Header: d:/cwork/soul1/RCS/stck.c 0.1 92/09/29 16:48:57
 *             Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *             ---------------------------------------------------------------
 *             $Log:	stck.c $
 *             Revision 0.1  92/09/29  16:48:57  Allan_Dystrup
 *             EXPERIMENTAL
 * 
 * REFERENCES
 *
 * USAGE       MAIN
 *             To setup to use the stack checking feature, you must rename
 *             your main() module to real_main(); - this file : "stck.c"
 *             contains a main(), that will be called in its place.
 *             After the needed stack data is recorded in main(), your
 *             real_main() is called , and you may then call the stack test
 *             routine as often as desired from within your code (generally
 *             it's a good idea to call it before return statements).
 *
 *             TEST ROUTINE
 *             int iCheckStack(char *id_string);
 *             Arguments : char *id_string     // a string to identify caller
 *             Returns.. : TRUE if stack corrupted, FALSE otherwise
 *             Example.. : if (iCheckStack("End Func Z")) exit(1);
 *
 * BUGS        The module needs further documentation, - when I get the time!
 *
 *-1========================================================================*/

#include "./MSDOS/dos.h"
#include <stdio.h>
#include <stdlib.h>

#include "./stck.h"

static unsigned int _far *framepointer;  /* NB: MS proprietary _far keyword */
static unsigned int _far *stacktop;      /* NB: MS proprietary _far keyword */
static unsigned int i;	                 /* Count stackframes when walking the stack */
struct SREGS sregs;	                 /* i80x86 segment registers, cf dos.h */

extern int
          real_main    P((int argc, char *argv[], char *envp[]));


#ifdef MAIN
/****************************************************************************/
/***************************** TEST DRIVER **********************************/
/****************************************************************************/

/* Test-function prototypes */
PRIVATE void vFunA(void);
PRIVATE void vFunB(void);
PRIVATE void vFunC(void);
PRIVATE void vFunD(void);


/*+3 MODULE STCK.C ---------------------------------------------------------*/
/*   NAME                      real_main                                    */
/*-- SYNOPSIS --------------------------------------------------------------*/
real_main(argc, argv, envp)
    int       argc;
    char     *argv[];
    char     *envp[];
{
/* DESCRIPTION
 *   The "real" main() is normally provided by the calling user,
 *   - here it is used as a test driver :
 *
 *   real_main -> vFunA -> vFunB -> vFunC
 *                               -> vFunD  expected CRACH!!!
 *-3*/

    /* The test driver */
    STCK("main");

    vFunA();
}


/*+3 MODULE STCK.C ---------------------------------------------------------*/
/*   NAME                      vFunA                                        */
/*-- SYNOPSIS --------------------------------------------------------------*/
void
vFunA(void)
{
/*-3*/
    int       abc;
    char     *ptr;

    ptr = 0;
    ptr += abc;		       /* stray pointer, - not catched by STCK */
    STCK("vFunA");

    vFunB();
}

/*+3 MODULE STCK.C ---------------------------------------------------------*/
/*   NAME                      vFunB                                        */
/*-- SYNOPSIS --------------------------------------------------------------*/
void
vFunB(void)
{
/*-3*/
    int       abc;
    char     *ptr;

    ptr = 0;
    ptr += abc;		       /* stray pointer, - not catched by STCK */
    STCK("vFunB");

    vFunC();
    vFunD();
}

/*+3 MODULE STCK.C ---------------------------------------------------------*/
/*   NAME                      vFunC                                        */
/*-- SYNOPSIS --------------------------------------------------------------*/
void
vFunC(void)
{
/*-3*/
    STCK("vFunC");
}

/*+3 MODULE STCK.C ---------------------------------------------------------*/
/*   NAME                      vFunD                                        */
/*-- SYNOPSIS --------------------------------------------------------------*/
void
vFunD(void)
{
/*-3*/
    unsigned _far *crash;
    int       i = 0;

    crash = (unsigned _far *) &crash;

    /* wipe out stack !!! */
    for (++crash; i <= 5; i++)
	*(++crash) = 0;

    STCK("vFunD");	       /* should generate an error! */
}

#endif	 /* ifdef MAIN */



/****************************************************************************/
/***************************** STACK CHECK & DEBUG **************************/
/****************************************************************************/


/*+2 MODULE STCK.C =========================================================*/
/*   NAME                      (dummy) main                                 */
/*== SYNOPSIS===============================================================*/
void
main(argc, argv, envp)
    int       argc;
    char     *argv[];
    char     *envp[];
{
/* DESCRIPTION
 *    Dummy main routine for recording stacktop at program startup; -
 *    calls real_main() provided by user program.
 *
 *    In most C compiler environments, the word just above the first local
 *    variable of a routine is the stack frame pointer; - it points back to
 *    the the frame pointer of the calling routine, and so on back to main().
 *
 *    The assumed "standard" setup for a compiled routine is thus :
 *       ; *** prolog ***
 *       PUSH    bp      ; save return address (this is the stack frame ptr.)
 *       MOV     bp,sp   ;
 *       SUB     sp,xxx  ; space for local data of routine
 *
 *       ...             ; compiled code for routine
 *
 *       ; *** epilog ***
 *       POP     bp      ; restore return address
 *       RET
 *-2*/

    unsigned int mainstack = 0;/* used for address */

    stacktop = &mainstack + 1; /* record address */

    real_main(argc, argv, envp);	/* and call the real main() */
}



/*+2 MODULE STCK.C =========================================================*/
/*   NAME                    iCheckStack                                    */
/*== SYNOPSIS ==============================================================*/
int
iCheckStack(calling_id)
    char     *calling_id;
{
/* DESCRIPTION
 *    A workhorse function to walk the stack, and make sure that we do not
 *    stray out of the stack during the walk : verify that it is indeed
 *    possible to trace back from the calling routine's stack frame to the
 *    top of the stack, and count the number of frames in the process.
 *
 * BUGS
 *    This routine will NOT catch all types of stack corruption, - it only
 *    looks at portion of the data on the stack (the stack frame links) and
 *    an errant pointer could conceivably damage part of the stack, while
 *    leaving the frame pointers intact.
 *-2*/

    unsigned _far *stackbottom;/* used for its address */

    i = 0;
    stackbottom = (unsigned _far *) &stackbottom;	/* get addr. of stack */
    framepointer = stackbottom + 2;	                /* find framepointer */
    segread(&sregs);

    /* Walk up the stack (ie. down the calling chain!)           */
    /* Exit when framepointer NOT in [stackbottom ... stacktop], */
    /* or at end of a complete stack walk                        */
    while (framepointer < stacktop &&
	   framepointer > stackbottom &&
	   i++ < MAX_STACK_DEPTH) {
	/* Trace back one return address to next frame on stack */
	framepointer = (unsigned _far *)
	    ((((long) (sregs.ss)) << 16) + (*framepointer));
    }

    /* We should now be exactly at the top of the stack, */
    /* If not, a stackframe pointer has been corrupted   */
    if (framepointer != stacktop) {
	printf("stack found corrupted in routine %s\n", calling_id);
	return (1);	       /* stack bad */
    }
    return (0);		   /* stack OK */

}  /* End function iCheckStack */


/* End module STCK.C                                                        */
/*==========================================================================*/
