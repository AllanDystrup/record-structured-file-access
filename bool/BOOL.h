/*+1========================================================================*/ 
/*   MODULE                      BOOL.H                                     */ 
/*==========================================================================*/ 
/*   FUNCTION      Headerfile for module BOOL.C (and user modules).  
 *  
 *	 SYSTEM        Standard (ANSI/ISO) C.
 *                 Tested on PC/MS DOS 5.0 (MSC 600A).  
 *  
 *    SEE ALSO      Modules : ERROR.C, BOOL.C
 *  
 *    PROGRAMMER    Allan Dystrup.
 *  
 *    COPYRIGHT     (c) Allan Dystrup, 1991, 2025
 *  
 *    VERSION      $Header: d:/cwk/kf/bool/RCS/bool.h 1.1 92/10/25 16:51:29
 *                 Allan_Dystrup Exp Locker: Allan_Dystrup $  
 *                 ----------------------------------------------------------  
 *                 $Log: bool.h $  
 *                 Revision 1.1 1992/10/25	16:51:29    Allan_Dystrup
 *                 Initial revision
 *		  --------------------
 *		  Revision 1.2 2025/12/07	10:00:00	Allan_Dystrup
 *		  Port to UBUNTU Linux on Windows10/WSL, Using CLion
 *		  Port to Windows 10 native, Using CLion for Windows
 *
 *-1=========================================================================*/  

/****************************************************************************/ 
/****************************** HEADER FILES ********************************/ 
/****************************************************************************/ 
#ifndef BOOL_H		/* Make sure this header is included only once      */
	#define BOOL_H	/* Matching #endif is at end-of-file                */
 
	/* Include header with general definitions */
	#include "../../general.h"


	/*--------------------------------------------------------------------------*/
	/* Defines to handle declaration, allocation and initialization of global   */
	/* data in a common header file.                                            */
	/* The _BOOL_INIT(x) macro won't work with aggregates because it interprets */
	/* a comma as indicating a new macro parameter (not part of the current); - */
	/* Aggregates are initialized inside a  #ifdef _BOOL_ALLOC ... #endif       */
	/*--------------------------------------------------------------------------*/
	#ifdef  BOOL_ALLOC                 /* BOOL.C: Allocate space for globals -- */
		#define BOOL_CLASS         /*    ignore (no extern class)           */
		#define BOOL_INIT(x)  x    /*    initialize                         */
	#else                              /* EXTERN: No allocation in user modules */
		#define BOOL_CLASS  extern /*  declare as extern                    */
		#define BOOL_INIT(x)	   /*    don't initialize           	    */
	#endif                             /* END BOOL_ALLOC ---------------------- */


	/****************************************************************************/
	/****************************** DATA STRUCTURES *****************************/
	/****************************************************************************/

	#define  SYMMAX (125+2)           /* Size of symbol table, max. 125 entries */
	#define  STRMAX (SYMMAX*10)       /* Size of lexeme array, - arbitrary long */
	#define  OUTMAX (SYMMAX*2)        /* Size of outputstring, - arbitrary long */
 

	/* Structure for symbol table */
	BOOL_CLASS struct entry {	  /* Structure of entry in symbol table :   */
		BYTE        *pzLexptr;	  /*     Ptr. to symbol string (ie. lexeme) */
		FLAG		fValue;	  /*     Attribute value : boolean [T | F]  */
	} symtable[SYMMAX];               /* Symbol table is Array of sych entries  */


	/******************************************************************************/
	/************************** FUNCTION PROTOTYPES *******************************/
	/******************************************************************************/

	BOOL_CLASS BYTE*
		pzParse     P((BYTE * pzStr));

	BOOL_CLASS FLAG
		fInterpret  P((BYTE * pzStr));

	BOOL_CLASS int
		iSymLookup  P((BYTE * pbStr, int len));

	BOOL_CLASS void
		vSymReset   P((void));


#endif  /* #ifndef BOOL_H */
/* END module bool.h                                                        */ 
/*-1========================================================================*/
