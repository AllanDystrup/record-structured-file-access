/*+1========================================================================*/
/* 								MODULE BM.H 								*/
/*==========================================================================*/
/* 
*  FUNCTION 	Headerfile for BM.C (and user modules)
*
*  PROGRAMMER 	Allan Dystrup
*
*  COPYRIGHT(c)	Allan Dystrup, Sept 1991.
*
*  VERSION		$Header: d:/cwk/kf/bm/RCS/bm.h 1.1 92/10/25 17:02:25
* 				Allan_Dystrup Exp Locker: Allan_Dystrup $
*				----------------------------------------------------------
* 				$Log: bm.h $
* 				Revision 1.1 92/10/25 17:02:25 Allan_Dystrup
* 				Initial revision
*				--------------------
*				Revision 1.2 2025/12/02	11:00:00	Allan_Dystrup
*				Port to UBUNTU Linux on Windows10/WSL, Using CLion
*				Port to Windows 10 native, Using CLion for Windows
*
*-1========================================================================*/

#ifndef _BM_H 	/* Make sure bm.h is included only once */

	#define _BM_H 	/* Matching #endif is at End-Of-File    */

	#ifndef GENERAL_H /* bm.c depends on defs. in general.h */
		#include "../../../general.h"
	# endif
	
	#ifndef ERROR_H /* bm.c depends on defs. in error.h */
		#include "../../err/error.h"
	#endif

	# ifndef BOOL_H /* bm.c depends on defs. in bool.h */
		#include "../../bool/bool.h"
	#endif

	#ifdef _BM_ALLOC /* Allocate and Init. data structures ? */
		#define _GLOBAL
		#define _INIT(x) x
	#else
		#define _GLOBAL extern
		#define _INIT(x)
	#endif


	enum scanType {
		eScanUD, /* Undefined scan order, - don't use ! */
		eScanQS, /* Force BM Quick Search algorithm 	*/
		eScanMS, /* Force BM Maximal Shift algorithm 	*/
		eScanOM  /* Force BM Optimal Mismatch algorithm */
		/* eScanNew Add new type of scan order here ... */
	};
	
	_GLOBAL void vBuildBM(enum scanType type, struct entry * symtab);
	_GLOBAL FLAG fRunBM(BYTE * text);
	_GLOBAL void vDelBM(void);
	
#endif /* #ifndef _BM_H */


/* End module bm.h */
/*==========================================================================*/