#============================================================================ 
#	<PROJ>\AC\MAKEFILE 
#============================================================================ 
#  FUNCTION      "Plain vanilla" makefile for project ac : 
#                Aho-Corasic (N)FSA for multiple string search. 
#  SYSTEM        Standard (ANSI/ISO) C. 
#                Tested on PC/MS DOS 5.0 (MSC 600A). 
#  SEE ALSO      Modules : ERROR.C, BOOL.C 
#  PROGRAMMER    Allan Dystrup. 
#  COPYRIGHT     (c) Allan Dystrup, 1991 
#  VERSION       To Be Done for LINUX
#============================================================================= 


#=============================================================================
# File Types
#=============================================================================
# DOS
#O = .o
#E = .exe
# LINUX
O =
E =

#============================================================================= 
# Directories and files 
#============================================================================= 
HF =  ./general.h  ./stack.h  ./error.h  ./bool.h  ./ac.h  
CF =  ac.c  ..\error\error.c ..\bool\bool.c
OF =  ac$O  ..\error\error$O ..\bool\bool$O 

#============================================================================  
# Compilation options 
#============================================================================ 
# 
Normal 
# K&R 
# CFLAGS += /c /I..\h /I..\error /I..\bool /Zi /Od /W3       
# ANSI 
  CFLAGS += /c /I. /Za /Od /W4        
  LFLAGS += 

# For CodeView 
#CFLAGS +=       /c /I..\h /I..\error /Za /Zi /Od /W4 
#LFLAGS +=       /CO 

#============================================================================ 
# Dependencies 
#============================================================================ 


#============================================================================ 
# Make ac.exe : executable ac testdriver 
ac$E : makefile $(OF) 
	cp makefile makefile.s 
	$(LD) $(LDFLAGS) $(OF),$@,,; 


#============================================================================ 
# Make ac$O : (1:linkable object  / 2: linkable debug object) 
#             (3: main testdriver / 4: main debug testdriver) 

ac$O : $(HF) $(CF) 
	cp ac.c ac.s 
#  	$(CC) -c $(CFLAGS) /Fo.\$@ ac.c 
#	$(CC) -c $(CFLAGS) /Fo.\$@ -DDEBUG ac.c 
	$(CC) -c $(CFLAGS) /Fo.\$@ -DMAIN  ac.c 
#	$(CC) -c $(CFLAGS) /Fo.\$@ -DMAIN -DDEBUG ac.c 


#============================================================================= 
# Miscelaneous 
 
print : 	pr -n -f ac.h ac.c | lp

clean :		rm -f *.s *.bak *.map slt 
		ls -Cqp 

realclean:   	rm -f *$O *.s *.bak *.map slt 
		ls -Cqp 

# END makefile (project ac)
#=============================================================================  
