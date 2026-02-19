/*+1========================================================================*/
/* MODULE                          CRC.C                                    */
/*==========================================================================*/
/* FUNCTION    Compute CCITT-CRC-16 checksum "on-the-fly".
 *                  The module contains a function : wCCITTcrc() for computing
 *             the checksum of a block of data.  The function may be compiled
 *             to a standard subroutine, which may then be called from another
 *             module (#include headerfile crc.h).  Alternatively the module
 *             crc.c can compile to a "standalone" program (compiler switch
 *             -DMAIN) for calculating checksums of arbitrary files.
 *                  wCCITTcrc() computes a "Cyclic Redundancy Check", using
 *             the CCITT polynomial as "generator".  The error detection rate
 *             is guaranteed to be 99.9984%, worst case.  The checksum utility
 *             may be used to verify the integrity of any block of data in
 *             memory or on disk (file validation).
 *
 * SYSTEM      STANDARD C.
 *             Tested on UNIX V.3 and PC/MS DOS 3.3
 *
 * PROGRAMMER  Allan Dystrup
 *
 * COPYRIGHT   (c) Allan Dystrup, Sept.1991
 *
 * VERSION     $Header: d:/cwork/chb/ux/RCS/crc.c 1.1
 *             91/09/19 23:18:30 Allan_Dystrup Exp $
 *             ---------------------------------------------------------------
 *             $Log:	crc.c $
 * 			Revision 1.1  91/09/23  11:31:20  Allan_Dystrup
 * 			Initial revision
 *
 * REFERENCES  Theory : Schwaderer, W.D. [1988]
 *                "C Programmer's guide to NetBIOS"
 *                Howard W. Sams & Company.
 *             Implementation: Several sources, notably credit to :
 *                Joe Campbell, Bob Felice, Nigel Cort,
 *                William Hunt & Andrew Chalk, PhD.

 * USAGE       1. as a standard subroutine :
 *                   #include <crc.h>
 *                   char *pzData = "Data for validation"; // data buffer
 *                   WORD wLength = strlen(pzData);        // length of buffer
 *                   WORD crc = PRESET;                    // initial crc preset
 *                   crc = wCCITTcrc(*pzData, wLength, crc);
 *             2. as a standalone program :
 *                   crc <filename> [optional tracelevel]

 * BUGS        1. Works only in a STANDARD C environment
 *                (Will not compile under K&R C without some massaging)
 *             2. On-the-fly crc calculation is relativevely slow, -
 *                might be speeded up by a table look-up approach.
 *                (This is primarily a demo-program).
 *
 *-1========================================================================*/



/*--------------------------------------------------------------------------*/
/*                             Include files                                */
/*--------------------------------------------------------------------------*/

/* Include standard C headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <assert.h>
#include <stdarg.h>

/* Include project header */
#define CRC_ALLOC
#include "crc.h"


/*--------------------------------------------------------------------------*/
/*                     Switch for program trace                             */
/*--------------------------------------------------------------------------*/
int       trace;

#define    LEVEL0     0		       /* Ignore all trace */
#define    LEVEL1     1		       /* Highlevel trace */
#define    LEVEL2     2		       /* Mediumlev. trace */
#define    LEVEL3     4		       /* Lowlevel trace */

#ifdef NTRACE
#define    T(level,stmt)	       /* Don't expand trace statements */
#else
#define    T(level,stmt)       { if (trace & level) stmt; }
#endif


/*--------------------------------------------------------------------------*/
/*                     Global Constants & (proto)types                      */
/*--------------------------------------------------------------------------*/
#define BUFSZ  (32*1024)
#define POLY   0x8408
#define PRESET 0xFFFF
#define CRCOK  0x470F

typedef enum {
    fFalse,
    fTrue
}         eBOOL;

PRIVATE WORD
       wPOSTSETcrc(WORD wCrc);



/*--------------------------------------------------------------------------*/
/*                     Global def's for error handling                      */
/*--------------------------------------------------------------------------*/
typedef enum {
    EARGS,
    EOPEN,
    EMEM,
    EREAD
}         eERRNO;

PRIVATE const char *rgpchErrmsg[] = {
    "Missing filename - Usage: crc <filename>\n",   /* EARGS */
    "Can't open file[%s] - Check file exists\n",    /* EOPEN, <filename> */
    "Out of memory - Need [%d]KB free ram\n",	    /* EMEM,  <ramsize>  */
    "Error reading file[%s] - Check disk\n"	    /* EREAD, <filename> */
};

PRIVATE void vChkErr(eBOOL cond, eERRNO err,...);



#ifdef MAIN
/*+1 MODULE CRC.C ==========================================================*/
/*   NAME                          main                                     */
/*== SYNOPSIS ==============================================================*/
void
main(int argc,			       /* Argument count */
     char *argv[]		       /* Argument vector */
)
{
/* DESCRIPTION
 *    This main function may act as both a testdriver for module CRC.C and as
 *    a small application program for calculating file checksums.
 *    1    If MAIN is not defined, the module compiles function wCCITTcrc to
 *         a "standard subroutine", which may be accessed by including the
 *         the headerfile crc.h in your application module.
 *    2    If MAIN is defined (compiler flag -DMAIN), the module compiles to
 *         2.1   If NDEBUG is not defined : a testdriver for module CRC.C
 *         2.2   If NDEBUG is defined (compiler flag -DNDEBUG) : a standalone
 *               application for computing file checksums.
 *-1*/
    FILE     *hInFile;             /* Handle for input file */
    BYTE     *rgchFbuf;            /* Buffer for reading input file */
    DWORD     dwCount;             /* Counter of bytes in rgchFbuf */
    WORD      wCRC;                /* The crc value being computed */


#ifdef DEBUG			   /* If debug! */
    /*----------------------------------------------------------------------*/
    /*                 Test driver part of main()                           */
    /*----------------------------------------------------------------------*/
    BYTE      rgchTbuf[40];	       /* Allocate Test-buffer for int. tests */

    trace = LEVEL1 | LEVEL2;	       /* Raise trace level */

    fputs("Module CRC.C, internal test :\n", stdout);

    fputs("   1. Functionality test\n", stdout);
    rgchTbuf[0] = 'T';                /* One ASCII char */
    rgchTbuf[1] = (BYTE) 0xD9;	       /* crc for 'T' - 1. byte */
    rgchTbuf[2] = (BYTE) 0xE4;	       /* crc for 'T' - 2. byte */
    rgchTbuf[3] = (BYTE) NULL;

    wCRC = PRESET;
    fprintf(stdout, "TEST [T],\t\t\t\t expected:0xD9E4 - got:0x%X\n\n",
	    wPOSTSETcrc(wCCITTcrc(rgchTbuf, 1, wCRC)));

    wCRC = PRESET;
    fprintf(stdout, "TEST [T <CRC>],\t\t\t\t expected:0x%X - got:0x%X\n\n",
	    wPOSTSETcrc(wCCITTcrc(rgchTbuf, 3, wCRC)), CRCOK);

    wCRC = PRESET;
    strcpy((char *) rgchTbuf, "THE,QUICK,BROWN,FOX,0123456789");
    fprintf(stdout, "TEST [%s],\t expected:0x6E20 - got:0x%X\n\n",
	    rgchTbuf, wPOSTSETcrc(wCCITTcrc(rgchTbuf, strlen((char *) rgchTbuf), wCRC)));

    fputs("   2. Range test\n", stdout);
    wCRC = PRESET;
    rgchTbuf[0] = (BYTE) '\0';	       /* Empty string */
    fprintf(stdout, "TEST [],\t\t\t\t expected:0x0000 - got:0x%0.4X\n\n",
	    wPOSTSETcrc(wCCITTcrc(rgchTbuf, strlen((char *) rgchTbuf), wCRC)));

    rgchTbuf[0] = (BYTE) 0x00;	       /* Lower limit ASCII */
    rgchTbuf[1] = (BYTE) 0x7F;
    rgchTbuf[2] = (BYTE) 0xFF;	       /* Upper limit ASCII */
    rgchTbuf[3] = (BYTE) 0xB8;	       /* crc for [0x00 0x7F 0xFF], 1. byte */
    rgchTbuf[4] = (BYTE) 0xBA;	       /* crc for [0x00 0x7F 0xFF], 2. byte */
    rgchTbuf[5] = (BYTE) NULL;
    wCRC = PRESET;
    fprintf(stdout, "TEST [0x00 0x7F 0xFF <CRC>],\t\t expected:0x%X - got:0x%X\n\n\n",
	    wPOSTSETcrc(wCCITTcrc(rgchTbuf, 5, wCRC)), CRCOK);
#endif

    /*----------------------------------------------------------------------*/
    /*                 Application part of main()                           */
    /*----------------------------------------------------------------------*/
    /* Check valid call format : crc filename [trace-level] */
    vChkErr(argc < 2, EARGS);
    trace = (argc > 2 && isdigit(*argv[2]) ? atoi(argv[2]) : LEVEL0);

    /* Open input file, mode read-binary */
    hInFile = fopen(argv[1], "rb");
    vChkErr(hInFile == NULL, EOPEN, argv[1]);

    /* Set up a large buffer */
    rgchFbuf = (BYTE *) malloc(BUFSZ);
    vChkErr(rgchFbuf == NULL, EMEM, BUFSZ);

    /* "Preset" crc to binary all 1's (avoids problem of leading 0 in data) */
    wCRC = PRESET;


    /* Calculate crc of file */
    while (fTrue) {

	/* Read next chunk of the inputfile */
	dwCount = fread(rgchFbuf, 1, BUFSZ, hInFile);

	/* If End-Of-File, print crc & terminate */
	if (dwCount == 0) {
	    vChkErr(!feof(hInFile), EREAD, argv[1]);

	    wCRC = wPOSTSETcrc(wCRC);
	    printf("CCITT CRC (REVERSE) for %8s   is\t[%04X]\n", argv[1], wCRC);
	    exit(0);
	}
	/* Update crc with value calculated from contents of buffer */
	T(LEVEL1, printf("Main calling wCCITTcrc with dwCount=%lu, crc=0x%X\n", dwCount, wCRC));
	wCRC = wCCITTcrc(rgchFbuf, dwCount, wCRC);
    }

} /* END function main */
#endif



/*+2 MODULE ================================================================*/
/*   NAME                      wCCITTcrc                                    */
/*== SYNOPSIS ==============================================================*/
PUBLIC    WORD
wCCITTcrc(BYTE * pszData,      /* Pointer to databuffer */
	  DWORD dwLength,       /* Length of databuffer */
	  WORD wCrc             /* Current value of crc, to be updated */
)
{
/* DESCRIPTION
 *    This routine generates the 16-bit remainder of a block of data,
 *    using the 16-bit CCITT polynomial generator.  The basic idea is to
 *    treat the entire message as a (rather long) binary number; the crc
 *    checksum is then obtained by taking the one's complement of the
 *    remainder after the modulo 2 division by a generator polynomial.
 *                           16   12   5
 *    The CCITT-CRC uses: ( X  + X  + X + 1 ) for the generator polynomial.
 *    This may also be expressed as :
 *         bit position     16   12        5     0
 *       - in binary :       1 0001 0000 0010 0001
 *       - in hex    :       1    1    0    2    1
 *    In computing the crc, a 17-bit dataregister is simulated by testing
 *    the MostSignificantBit before shifting the data.  This affords us the
 *    luxury of specifying the polynomial as a 16-bit value : 0x1021.
 *    The crc is generated in "LSB->MSB" order, so the bits of the polynomial
 *    are also stored in reverse order : POLY = 0x8408.
 *-2*/
    WORD      wData;		       /* One word of data */
    BYTE      chBit;		       /* Counter for each bit in char */
    DWORD     dwCount = 0;	       /* Counter for each buffer run */


    /* Check actual parameters */
    assert(pszData != NULL);

    /* Make it roubust : define crc to zero for empty buffer */
    if (dwLength == 0 && wCrc == PRESET)
	return (wCrc);

    /* For each BYTE (8 bit) in the wData block ... */
    do {

	/* For each bit in the wData BYTE ... */
	for (chBit = 0, wData = (WORD) 0xFF & *pszData++;
	     chBit < 8;
	     chBit++, wData >>= 1) {   /* move up next bit for XOR */

	    if ((wCrc & 0x0001) ^ (wData & 0x0001))
		/* If msb of (crc XOR data) is on, shift & subtract poly */
		wCrc = (wCrc >> 1) ^ POLY;
	    else
		/* Otherwise, transparent shift */
		wCrc >>= 1;
	}

	T(LEVEL2, ((dwCount++ % 10) == 0 ?
		   printf("\n[%05lu]:  %04X", dwCount - 1, wCrc) : printf("  %04X", wCrc)));

    } while (--dwLength);

    return (wCrc);

} /* END function wCCITTcrc */



/*+3 MODULE ----------------------------------------------------------------*/
/*   NAME                      wPOSTSETcrc                                  */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   WORD
wPOSTSETcrc(WORD wCrc)        /* Crc value to be "post-conditioned" */
{
/* DESCRIPTION
 *    CRC postconditioning  :  Do 1's complement and swap bytes in wCrc.
 *    When a crc is itself included in the calculation, the valid crc is :
 *    Final wCrc:0xF0B8, After complement:0x0F47, After byte swap:0x470F
 *-3*/
    WORD      wSwap;		       /* Word for swapping crc HI/LO */

    T(LEVEL2, printf("\nFinal CRC : %04X", wCrc));

    wCrc = ~wCrc;
    T(LEVEL2, printf("\nComplement: %04X", wCrc));

    wSwap = wCrc;
    wCrc = (wCrc << 8) | (wSwap >> 8 & 0xFF);
    T(LEVEL2, printf("\nByte Swap : %04X\n", wCrc));

    return (wCrc);

} /* END function wPOSTSETcrc */



/*+3 MODULE ----------------------------------------------------------------*/
/*   NAME                          vChkErr                                  */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vChkErr(eBOOL cond,		   /* Error-condition to check (TRUE=error) */
	eERRNO err,		   /* Errornumber if error-condition TRUE */
	...			   /* Var. number of args for error-msg. */
)
{
/* DESCRIPTION
 *    Simple error handler.
 *    All errors are fatal (ie cause program termination).
 *    Error messages are written to stderr, and may be redirected to a file.
 *       Alternatively you might want to log error msg directly in an err-file.
 *    Error-messages are "hard coded" into array rgpchErrmsg[], -
 *       In a larger application they would be initialized from a file,
 *       and moved to its own source module together with function vChkErr().
 *-3*/
    va_list   ap;		       /* argument pointer */

    if (cond) {

	/* Print error header identifying module */
	fprintf(stderr, "\nMODULE: File[%s] - Line[%d]" \
		"\n\tVersion: Date[%s] - Time[%s]" \
		"\n\tError..: Number[%02d] - ", \
		__FILE__, __LINE__, __DATE__, __TIME__, err);

	/* Point ap to 1. var arg, Print error-message & Clean up */
	va_start(ap, err);
	(void) vfprintf(stderr, rgpchErrmsg[err], ap);
	fflush(stderr);
	va_end(ap);

	/* Exit module */
	abort();
    }

} /* END function vChkErr */



/* END module CRC.C                                                         */
/*==========================================================================*/
