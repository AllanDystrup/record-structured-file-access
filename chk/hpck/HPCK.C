/*==========================================================================*/
/* MODULE                          HPCK.C                                   */
/*==========================================================================*/
/* FUNCTION    Contains a simple routine for diagnosing a corrupted heap :
 *             places a known pattern ("fence") around locations of dynamically
 *             allocated storage, and verifies these patterns at each subsequent
 *             call of allocation functions.
 *
 * SYSTEM      PC/MS DOS,  Tested on MS DOS V.3.3.
 *             Compilers   MSC 6.0
 *
 * SEE ALSO    Module hpck.h
 *
 * PROGRAMMER  Allan Dystrup
 *             Patterned after routines by Eric White and Roberd Ward.
 *
 * COPYRIGHT   (c) Allan Dystrup, Kommunedata I/S 1991
 *
 * VERSION     $Header: d:/cwork/soul1/RCS/hpck.c 0.1 92/09/29 16:51:22
 *             Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *             ---------------------------------------------------------------
 *             $Log:	hpck.c $
 *             Revision 0.1  92/09/29  16:51:22  Allan_Dystrup
 *             EXPERIMENTAL
 *
 * REFERENCES
 *
 * USAGE
 *
 * BUGS        The module needs further documentation, - when I get the time!
 *
 *-1========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "hpck.h"

#define MAIN

/*==========================================================================*/
/*                           buffer administration                          */
/*==========================================================================*/
#define MAXMALLOCS 200	       /* Max # sep. alloc. buffers not yet freed */
#define KP         0xAA       /* Def. of known pattern for buffer "fence" */
#define KPW        2	       /* Def. of known pattern width : length(KP) */

static int nummallocs = 0;    /* # of currently allocated buffers */

/* List of all memory locations allocated (and not yet freed) */
struct mtype {
    unsigned char *addr;
    int       size;
};
static struct mtype m[MAXMALLOCS];



/*==========================================================================*/
/*                           private prototypes                             */
/*==========================================================================*/
PRIVATE void trace	P((char *s));
PRIVATE void dumpbuf	P((int x));
PRIVATE void dumpbuf	P((int x));



#ifdef MAIN
/****************************************************************************/
/*************************** TEST DRIVER ************************************/
/****************************************************************************/

/*+2 MODULE HPCK.C =========================================================*/
/*   NAME                          main                                     */
/*== SYNOPSIS ==============================================================*/
int main()
{
    char     *string1, *string2;

    string1 = vmalloc(4);
    string2 = vmalloc(6);

    strcpy(string2, "12345");  /* OK */
    vverify(string2);
    strcpy(string1, "ABCDEFGHIJ");	/* OOPS : out of bounds! */
    vverify(string1);
    

    printf("string1 : [%s]\n", string1);
    printf("string2 : [%s]\n", string2);

    vfree(string1);
    vfree(string2);
}

#endif   /* ifdef MAIN */



/****************************************************************************/
/*************************** HEAP CHECK & DEBUG *****************************/
/****************************************************************************/


/*+2 MODULE HPCK.C =========================================================*/
/*   NAME                          vverify                                  */
/*== SYNOPSIS ==============================================================*/
void
vverify(id)
    char     *id;
{
/* DESCRIPTION
 *    Verify the entire malloc heap
 *-2*/

    char      s[80];
    int       x, c;

    /* Loop through list of malloc buffers */
    for (x = 0; x < nummallocs; x++)
	/* For each allocated buffer */
	if (m[x].addr != NULL)
	    /* Look at buffer's first (and last) KPW chars : */
	    for (c = 0; c < KPW; c++) {

		   /* Verify that "fence" around buffer is still intact */
		   if (*(m[x].addr + c) != KP ||
		      *(m[x].addr + KPW + m[x].size + c) != KP) {
		      sprintf(s, "ERROR: Malloc Area Corrupted [%s]\n", id);
		      trace(s);
		      fputs(s, stderr);
		      dumpbuf(x);
		      exit(1);
	   	   }
           }
}



/*+2 MODULE HPCK.C =========================================================*/
/*   NAME                          trace                                    */
/*== SYNOPSIS ==============================================================*/
void
trace(s)
    char     *s;
{
/* DESCRIPTION
 *    Write ("trace") one textstring to the logfile
 *-2*/

    static FILE *fdLog = NULL;

    /* If logfile not open, open it! */
    if (fdLog == NULL) {
	remove("HPCK.LOG");    /* delete any existing logfile */
	fdLog = fopen("HPCK.LOG", "w");	/* open new logfile (reset) */
	setbuf(fdLog, NULL);   /* no buffering, please */
    }

    /* Trace string to logfile */
    fputs(s, fdLog);
}


/*+2 MODULE HPCK.C =========================================================*/
/*   NAME                          dumpbuf                                  */
/*== SYNOPSIS ==============================================================*/
void
dumpbuf(x)
    int       x;
{
/* DESCRIPTION
 *    Dump one malloc-buffer to the logfile
 *-2*/

    unsigned char *c;
    char      s[80];


    /* Point to beginning of buffer, incl. fence */
    c = (unsigned char *) m[x].addr - 2;

    /* Loop through buffer (incl. 2 * fence), one char at a time */
    while (c <= (m[x].addr + KPW + m[x].size + KPW - 1)) {

	sprintf(s, "%04.4lx : %02x [%c] ", (long) c, *c, *c);	/* addr & char */

	if (c == m[x].addr)    			/* Beg. fence ? */
	    strcat(s, "<= leading known pattern");

	if (c == m[x].addr + KPW)		/* 1. Buffer char ? */
	    strcat(s, "<= address of malloc buffer");

	if (c == m[x].addr + m[x].size + KPW)	/* End fence ? */
	    strcat(s, "<= trailing known pattern");

	strcat(s, "\n");
	trace(s);
	c++;
    }
}



/*+2 MODULE HPCK.C =========================================================*/
/*   NAME                          vdump                                    */
/*== SYNOPSIS ==============================================================*/
void
vdump(id)
    char     *id;
{
/* DESCRIPTION
 *    Dump the entire malloc heap to the logfile
 *-2*/

    char      s[80];
    int       x;

    sprintf(s, "========== Dump of malloc heap [%s] ==========\n", id);
    trace(s);

    /* Loop through list of malloc buffers */
    for (x = 0; x < nummallocs; x++)
	/* For each allocated buffer */
	if (m[x].addr != NULL) {

	    sprintf(s, "Malloc buffer addr : %04.4lx\n", (long) m[x].addr);
	    trace(s);

	    sprintf(s, "Malloc buffer size : %04x\n", KPW + m[x].size + KPW);
	    trace(s);

	    dumpbuf(x);
	}
}



/****************************************************************************/
/*************************** DYNALLOC SUBSTITUTES ***************************/
/****************************************************************************/


/*+2 MODULE HPCK.C =========================================================*/
/*   NAME                          vmalloc                                  */
/*== SYNOPSIS ==============================================================*/
char     *
vmalloc(size)
    size_t    size;
{
/*-2*/
    char     *buffer, s[80];
    int       c, x;

    /* Verify the malloc heap */
    vverify("vmalloc");

    /* Allocate malloc buffer from C stdlib */
    if ((buffer = calloc(KPW + size + KPW, 1)) == NULL) {
	sprintf(s, "ERROR: calloc returned NULL\n");
	fprintf(stderr, s);    /* to VDU */
	trace(s);	       /* to log */
	exit(1);
    }

    sprintf(s, "%04.4lx:vmalloc  size = %ld\n", (long) buffer, (long) size);
    trace(s);


    /* Find a place for an entry in the buffer list m[] */
    for (x = 0; x < MAXMALLOCS && m[x].addr != NULL; x++);
    if (x == MAXMALLOCS) {
	sprintf(s, "ERROR: MAXMALLOCS too small\n");
	fprintf(stderr, s);    /* to VDU */
	trace(s);	       /* to log */
	exit(1);
    }

    /* Set entry in buffer list for new malloc buffer */
    m[x].addr = (unsigned char*) buffer;
    m[x].size = size;

    if (x == nummallocs)
	nummallocs++;

    /* Set up "fence" around the malloc buffer */
    for (c = 0; c < KPW; c++) {
	*(m[x].addr + c) = KP; /* Beginning of buffer */
	*(m[x].addr + KPW + m[x].size + c) = KP;	/* End of buffer */
    }

    /* Fill the malloc buffer with '#' */
    for (c = 0; c < m[x].size; c++)
	*(m[x].addr + KPW + c) = '#';

    dumpbuf(x);
    return (buffer + KPW);     /* Exclude fence! */
}



/*+2 MODULE HPCK.C =========================================================*/
/*   NAME                          vcalloc                                  */
/*== SYNOPSIS ==============================================================*/
void     *
vcalloc(n, size)
    size_t    n, size;
{
/*-2*/
    return ((void *) vmalloc(n * size));
}


/*+2 MODULE HPCK.C =========================================================*/
/*   NAME                          vfree                                    */
/*== SYNOPSIS ==============================================================*/
void
vfree(buffer)
    char     *buffer;
{
/*-2*/
    char     *b, s[80];
    int       x;

    /* Verify the malloc heap */
    vverify("vfree");

    /* Point to beginning of buffer, incl. fence */
    b = buffer - KPW;

    /* Locate buffer in the buffer list */
    for (x = 0; x < nummallocs && m[x].addr != (unsigned char *)b; x++);
    if (x == nummallocs) {
	sprintf(s, "ERROR: location to free is NOT in list\n");
	fprintf(stderr, s);    /* to VDU */
	trace(s);	       /* to log */
	exit(1);
    }

    sprintf(s, "%04.4lx:vfree\n", (long) b);
    trace(s);
    dumpbuf(x);

    /* Free malloc buffer via C stdlib */
    free(b);

    /* Reset entry in buffer list for free'd malloc buffer */
    m[x].addr = NULL;
    if (x == nummallocs - 1)
	nummallocs--;
}


/*+2 MODULE HPCK.C =========================================================*/
/*   NAME                          vrealloc                                 */
/*== SYNOPSIS ==============================================================*/
char     *
vrealloc(buffer, size)
    char     *buffer;
    size_t    size;
{
/*-2*/
    char     *b, *b2, s[80];
    int       c, x;


    /* Verify the malloc heap */
    vverify("vrealloc");

    /* Point to beginning of buffer, incl. fence */
    b = buffer - KPW;


    /* Locate buffer in the buffer list */
    for (x = 0; x < nummallocs && m[x].addr != (unsigned char *)b; x++);
    if (x == nummallocs) {
	sprintf(s, "ERROR: location to realloc is NOT in list\n");
	fprintf(stderr, s);    /* to VDU */
	trace(s);	       /* to log */
	exit(1);
    }

    sprintf(s, "%04.4lx:vrealloc  size = %ld\n", (long) b, (long) size);
    trace(s);

    /* Remove (overwrite) trailing fence of malloc buffer */
    for (c = 0; c < KPW; c++)
	*(m[x].addr + KPW + m[x].size + c) = 0;

    /* Re-allocate malloc buffer via C stdlib : */
    /* "resize" the malloc buffer, without changing the contents */
    b2 = realloc(b, size + KPW + KPW);


    /* Redefine entry in buffer list for reallocated malloc buffer */
    m[x].addr = (unsigned char *)b2;
    m[x].size = size;

    /* Set up trailing "fence" for the reallocated malloc buffer */
    /* NB: beginning fence unchanged by realloc */
    for (c = 0; c < KPW; c++)
	*(m[x].addr + KPW + m[x].size + c) = KP;

    return (b2 + KPW);
}


/* End of module HPCK.C                                                     */
/*==========================================================================*/
