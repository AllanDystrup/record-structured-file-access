/*+1========================================================================*/
/* MODULE                          AC.H                                     */
/*==========================================================================*/
/* FUNCTION    Headerfile for AC.C (and user modules)                       */
/*             (include after general.h)                                    */
/*                                                                          */
/* PROGRAMMER  Allan Dystrup                                                */
/*                                                                          */
/* COPYRIGHT   (c) Allan Dystrup, Sept. 1991                                */
/*                                                                          */
/* VERSION                                                                  */
/*             Revision 1.2 2025/12/02	11:00:00	Allan_Dystrup	    */
/*             Port to UBUNTU Linux on Windows10/WSL, Using CLion	    */
/*-1========================================================================*/

#ifndef AC_H             	/* Make sure AC,H is included only once   */
#   define AC_H                	/* Matching #endif is at End-Of-File      */

 
#   ifndef GENERAL_H           	/* AC.H depends on defs in file general.h */
#      include "../../../general.h"
#   endif

#   ifdef AC_ALLOC             	/* Allocate & Initialize data structï ?   */
#      define CLASS
#      define I(x) x
#   else
#      define CLASS extern
#      define I(x)
#   endif

CLASS void  vBuildFsa(int type);

CLASS FLAG  fRunFsa(int type, register BYTE * str, BYTE *postfix);

CLASS void  vDelFsa(void);

#endif  /* #ifndef AC_H */

/* END module AC_H */
/*==========================================================================*/
