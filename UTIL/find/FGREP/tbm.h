/*+1 =======================================================================*/
/*                                MODULE TBM.H                              */
/*==========================================================================*/
/* FUNCTION      Headerfile for module TBM.C (and user modules);
 *               #include this file to access the PUBLIC functions in TBM.C
 *		 for performing a "Tuned Boyer-Moore" single string search.
 *
 * SYSTEM         Standard (ANSI/ISO) C
 *                Tested on PC/MS DOS v.3.3 & UNIX SYS V.3, 1992
 *                Tested on Windows 10 and WSL UBUNTU Linux, 2025.
 *
 * SEE ALSO       Module TBM.C
 *
 * PROGRAMMER     Allan Dystrup
 *
 * COPYRIGHT (c)  Allan Dystrup, Mar. 1992
 *
 * VERSION        $Header: d:/cwork/index/RCS/key.h 1.5 92/03/03 11:25:04
 *                Allan_Dystrup Exp Locker: Allan_Dystrup $
 *                -----------------------------------------
 *                $Log: tbm.h $
 *                Revision 1.2 92/03/11 14:53:51 Allan_Dystrup
 *                hack'ed UCASE
 *                Revision 1.3 92/03/13 11:15:46 Allan_Dystrup
 *                integrated UCASE
 *                Revision 1.4 92/03/13 13:34:23 Allan_Dystrup
 *                linted DOS/UX
 *                ------------------------------------------
 *                Revision 1.2 2025/12/09 11:00:00	Allan_Dystrup
 *                Port to UBUNTU  on Win.10 / WSL, Using CLion for Linux
 *		 Port to Win. 10 native/terminal, Using CLion for Windows
 *
 *-1==========================================================================*/

#ifndef _TBM_H        /* Make sure tbm.h is included once  */
     #define _TBM_H   /* Matching #endif is at end-of-file */


    /*==========================================================================*/
    /*                        Setup for DEBUG [ON|OFF]                          */
    /*==========================================================================*/
    #ifdef DEBUG
        #define PRIVATE
        #define D(x) x
        #include <assert.h>
    #else
        #define PRIVATE static
	#define D(x)      /* ignore */
	#define assert(x) /* ignore */
    #endif /* DEBUG */


    /*==========================================================================*/
    /*                        Setup for portable data types                     */
    /*==========================================================================*/
    #define PUBLIC /* ignore */
    #define BYTE   unsigned char
    #define WORD   unsigned int
    #define DWORD  unsigned long
    #define ASIZE  256 /* Size of alphabet : extended (8-bit) ASCII */

    
    /*==========================================================================*/
    /*							CONFIGURATION		*/
    /*			 DEFAULT COMPILE TIME  options for TBM			*/
    /*==========================================================================*/
    #define TBM_FQ		/* define to include guard-test based on char freq. */

    #define _TBM_ALLOC		/* allocate character frequency table arrays   */
    //#define DANISH		/* - define to choose DANISH char.freq. table  */
    #define ENGLISH		/* - define to chose ENGLISH char.freq. table  */

    //#define TBM_TF		/* define to force boolean return over match count */
    #define TBM_UC		/* define to force uppercase (case insensitive) match */
    
    
    #ifdef TBM_FQ
    /*==========================================================================*/
    /*					Frequency-array for chars in alphabet					*/
    /*==========================================================================*/
    static float freq[ASIZE] =
 
	#ifdef ENGLISH
	/* Frequencies of letters in English written text (case insensitive) */
	/* Encoded in IBM Code Page 437 (US:United States) */
	{
	/*--------0-----1-----2-----3-----4-----5-----6-----7--*/
	/*00*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*08*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*10*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*18*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*20*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*28*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*30*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*38*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*40*/ 0.0f, 8.9f, 2.3f, 4.5f, 3.2f,11.1f, 1.5f, 2.4f, /*@-G*/
	/*48*/ 2.9f, 7.8f, 0.2f, 1.1f, 5.5f, 3.2f, 6.8f, 6.9f, /*H-O*/
	/*50*/ 3.1f, 0.2f, 7.4f, 5.6f, 7.1f, 3.6f, 1.0f, 1.1f, /*P-W*/
	/*58*/ 0.3f, 2.0f, 0.2f, 1.5f, 1.5f, 1.5f, 0.0f, 0.0f, /*X-� ?*/
	/*60*/ 0.0f, 8.9f, 2.3f, 4.5f, 3.2f,11.1f, 1.5f, 2.4f, /*`-g*/
	/*68*/ 2.9f, 7.8f, 0.2f, 1.1f, 5.5f, 3.2f, 6.8f, 6.9f, /*h-o*/
	/*70*/ 3.1f, 0.2f, 7.4f, 5.6f, 7.1f, 3.6f, 1.0f, 1.1f, /*p-w*/
	/*78*/ 0.3f, 2.0f, 0.2f, 1.5f, 1.5f, 1.5f, 0.0f, 0.0f, /*x-� ?*/
	/*80*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*88*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*90*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*98*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*A0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*A8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*B0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*B8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*C0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*C8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*D0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*D8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*E0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*E8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*F0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*F8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };
    #endif /*ENGLISH*/
    
    #ifdef DANISH
    /* Frequencies of letters in Danish written text (case insensitive) */
    /* cf. "Nyt fra Sprognaevnet", Nr.4 Matrs 1970 */
    /* Encoded in IBM Code Page 850 (Multilingual) */
    {
	/*--------0-----1-----2-----3-----4-----5-----6-----7--*/
	/*00*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*08*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*10*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*18*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*20*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*28*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*30*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*38*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*40*/ 0.0f, 5.6f, 1.4f, 0.1f, 6.7f,16.6f, 2.6f, 4.4f, /*@-G*/
	/*48*/ 2.1f, 5.8f, 0.6f, 3.2f, 5.1f, 3.7f, 7.7f, 4.5f, /*H-O*/
	/*50*/ 1.3f,0.02f, 8.0f, 5.5f, 7.2f, 1.6f, 2.7f,0.02f, /*P-W*/
	/*58*/0.02f, 0.6f,0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, /*X-�*/
	/*60*/ 0.0f, 5.6f, 1.4f, 0.1f, 6.7f,16.6f, 2.6f, 4.4f, /*`-g*/
	/*68*/ 2.1f, 5.8f, 0.6f, 3.2f, 5.1f, 3.7f, 7.7f, 4.5f, /*h-o*/
	/*70*/ 1.3f,0.02f, 8.0f, 5.5f, 7.2f, 1.6f, 2.7f,0.02f, /*p-w*/
	/*78*/0.02f, 0.6f,0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, /*x-�*/
	/*80*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.3f, 0.0f, /*�*/
	/*88*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.3f, /*�*/
	/*90*/ 0.0f, 0.8f, 0.8f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, /*��*/
	/*98*/ 0.0f, 0.0f, 0.0f, 0.9f, 0.9f, 0.0f, 0.0f, 0.0f, /*��*/
	/*A0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*A8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*B0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*B8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*C0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*C8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*D0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*D8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*E0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*E8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*F0*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	/*F8*/ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };
    #endif /*DANISH*/
    
    #endif /*TBM_FQ*/

    #ifdef TBM_UC
		#define UCASE(x) bDKupper(x) /* Convert to upper case */
    #else
		#define UCASE(x) x           /* Pass on without conversion */
    #endif /*TBM_UC*/


/*==========================================================================*/
/*                          Public function prototypes                      */
/*==========================================================================*/
    PUBLIC void
	vBuildTBM(BYTE *pb, int len);
	
    PUBLIC int 
	iRunTBM(BYTE *base, int n);
	
	
#endif /* #ifdef _TBM_H */
/* End of module tbm.h */
/*==========================================================================*/