/*+1========================================================================*/
/* MODULE                          CRC.H                                    */
/*==========================================================================*/
/* FUNCTION    Headerfile for CRC.C
 *             To be included for access to function wCCITTcrc()
 *             for computing CCITT CRC-16 checksums of data in ram or on disk.
 * SEE ALSO    Documentation for file CRC.C
 * PROGRAMMER  Allan Dystrup
 * COPYRIGHT   (c) Allan Dystrup, Sept.1991
 * VERSION     $Header: d:/cwork/chb/ux/RCS/crc.h
 *             1.1 91/09/23 11:28:41 Allan_Dystrup Exp $
 *             $Log:	crc.h $
 *             Revision 1.1  91/09/23  11:28:41  Allan_Dystrup
 *             Initial revision
 *-1========================================================================*/

/* Make sure crc.h is included only once; - matching endif is at EOF! */
#ifndef CRC_H_
#define CRC_H_


/*==========================================================================*/
/* Handle declaration, allocation & initialization of global data in h-file */
/*==========================================================================*/
#ifdef CRC_ALLOC
#define    CRC_CLASS
#define    CRC_INIT(x) x
#else
#define    CRC_CLASS   extern
#define    CRC_INIT_(x)
#endif


#define PRIVATE     static
#define PUBLIC


/*==========================================================================*/
/*                     Function prototype                                   */
/*==========================================================================*/
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;


CRC_CLASS PUBLIC WORD
wCCITTcrc(BYTE * pszData,	       /* Pointer to databuffer */
	  DWORD dwLength,	       /* Length of databuffer */
	  WORD wCrc	               /* Current value of crc, to be updated */
);

#endif				       /* #ifdef CRC_H_ */
/* End of module crc.h                                                      */
/*==========================================================================*/
