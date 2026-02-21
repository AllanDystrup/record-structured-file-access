#============================================================================# 
#                           <PROJ>\ERROR\MAKEFILE                            # 
#============================================================================# 
#  FUNCTION     "Plain vanilla" makefile for project error : 
#     		Simple error-handling routines. 
#  SYSTEM       Standard (ANSI/ISO) C. 
#               Tested on PC/MS DOS 5.0 (MSC 600A). 
#  SEE ALSO     Modules : ERROR.C, BOOL.C 
#  PROGRAMMER   Allan Dystrup. 
#  COPYRIGHT    (c) Allan Dystrup, 1991 
#  VERSION      $Header: d:/cwk/kf/error/RCS/makefile 1.1 92/10/25 17:14:20 
#               Allan_Dystrup Exp Locker: Allan_Dystrup $ 
#               ------------------------------------------------------------- 
#               $Log: makefile $ 
#               Revision 1.1  92/10/25  17:14:20  Allan_Dystrup 
#              	Initial revision 
#=============================================================================   

#============================================================================= 
# Directories and files 
#============================================================================= 
HF  =  ..\h\general.h error.h 
CF  =  error.c 
OF  =  error$O  


#============================================================================ 
# Compilation options 
#============================================================================ 
# Normal 
CFLAGS +=       /c /I..\h /Za /Od /W4 
LFLAGS +=  

# For CodeView 
#CFLAGS +=      /c /I..\h /Za /Zi /Od /W4 
#LFLAGS +=      /CO   


#============================================================================= 
# Dependencies 
#============================================================================= 


#============================================================================= 
# Error  

error$E: makefile $(OF)   
		cp makefile makefile.s   
		$(LD) $(LDFLAGS) $(OF),error$E,,;

error$O:  error.c  $(HF)   
		cp error.c errormd.s   
		$(CC) -c $(CFLAGS) error.c  
#  		$(CC) -c $(CFLAGS) /Fo.\$@ -DDEBUG error.c 
#  		$(CC) -c $(CFLAGS) /Fo.\$@ -DMAIN -DDEBUG error.c   


#============================================================================= 
# Miscelaneous  

print :  
	pr -n -f error.h error.c | lp 
 
clean :  
	rm -f *.s *.bak *.map slt ls -Cqp 

