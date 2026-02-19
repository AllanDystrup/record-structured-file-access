#============================================================================#
# Project 	C:\Users\allan\CLionProjects\util\BOOL\Makefile for BOOL
#		For command-line build, i.e. outside an IDE
#============================================================================#
# FUNCTION 	Makefile for proj. BOOL command-line generation
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	BOOL.h BOOL.c
#
# PROGRAMMER 	Allan Dystrup.
# 		COPYRIGHT (c) Allan Dystrup, 1991, 2026.
#
# VERSION 	Revision 1.0 1992/03/20 13:40	 	Allan_Dystrup
#		Initial revision
#
# 		Revision 1.8 2026/02/10 17:00	 	Allan_Dystrup
# 		Port to Win.10 native/terminal, Using CLion for Windows
#		[Using UBUNTU wsl.localhost/Ubuntu/usr/bin build tools
#           	 for: gcc (compile), ld (link) and make, using Makefile]
#		Windows Terminal (PowerShell 7 x64 Cmd Prompt):
#		Microsoft Windows [Version 10.0.19045.6691]
#		(c) Microsoft Corporation. All rights reserved.
#
#		----- Switch to Ubuntu for Linux build tools
#		C:\Users\allan>		wsl -d Ubuntu -u allan
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$ cd CLionProjects/ACCESS/UTIL/bool
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/bool$
#
#		----- Clean and build default target
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/bool$ make -f BOOL.Makefile realclean
#			rm -f bool
#			rm -f *.o
#			rm -f booltst.log
#			ls -al .
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/bool$ make -f BOOL.Makefile
#			gcc -DMAIN -DDEBUG -c bool.c -o bool.o
#			gcc -DDEBUG -g -c ../err/error.c -o error.o
#			gcc -DDEBUG -g -c ../stack/stack.c -o stack.o
#			gcc bool.o error.o stack.o -Wall -lm -o bool
#			ls -al .
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/bool$ make -f BOOL.Makefile test
#			./bool "ape & (bee / cow)"
#			: etc
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/bool$ make -f BOOL.Makefile log
#			: etc.
#
#============================================================================#

# [0] FILES -----------------------------------------------------------------
HEADERS = BOOL.h ../err/error.h ../stack/stack.h ../../general.h 
SOURCES = BOOL.c ../err/error.c ../stack/stack.h
OBJECTS = bool.o error.o stack.o
TARGET  = bool

.PRECIOUS: $(TARGET) $(SOURCES)

default: $(TARGET)


# [1] CLEAN -----------------------------------------------------------------

#	----- make -f BOOL.Makefile clean -----
clean:
	rm -f $(TARGET)
	rm -f $(TARGET).o
	ls -al .

#	----- make -f BOOL.Makefile realclean -----	
realclean:
	rm -f $(TARGET)
	rm -f *.o
	rm -f *.doc
	rm -f $(TARGET)tst.log
	ls -al .
	
# [2] BUILD ---------------------------------------------------------------

#	----- make -f BOOL.Makefile -----
CC = gcc
#CFLAGS = -DMAIN -DDEBUG -g	# Debug trace
CFLAGS =  -g			# NO trace

#	----- Compile OBJ -----
$(TARGET).o:	$(TARGET).c $(HEADERS)
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET).o
	$(CC) -DDEBUG -g -c ../err/error.c -o error.o
	$(CC) -DDEBUG -g -c ../stack/stack.c -o stack.o

#	----- Link EXE -----
LIBS = -lm
$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(TARGET)
	ls -al .


# [3] TEST ------------------------------------------------------------------

#	-----  make -f BOOL.Makefile [test|log] -----
test:
	./$(TARGET) "ape & (bee / cow)"
	
log:
	rm -f booltst.log
	touch booltst.log 
	echo ------------------------------------------------- >> booltst.log
	echo SCANNER - BOOLEAN OPERATORS  > booltst.log 

	# Obs: XOR operator % must be escaped as a batchfile metachar (ie: %%)
	./$(TARGET) ape                       	>> booltst.log
	./$(TARGET) "^ape"                    	>> booltst.log
	./$(TARGET) "^^ape"                   	>> booltst.log
	./$(TARGET) "ape & bee"               	>> booltst.log
	./$(TARGET) "ape / bee"               	>> booltst.log
	./$(TARGET) "ape % bee"               	>> booltst.log
	./$(TARGET) "^ape % bee"              	>> booltst.log
	./$(TARGET) "ape & ^bee"              	>> booltst.log


	echo -------------------------------------------------- >> booltst.log
	echo SCANNER - PARENTHESIS     		>> booltst.log
	./$(TARGET)  "ape & bee / cow" 		>> booltst.log
	./$(TARGET)  "ape & (bee / cow)"        >> booltst.log
	./$(TARGET)  "ape / bee & cow % dodo & eel"        >> booltst.log
	./$(TARGET)  "((ape / bee) & (cow % dodo)) & eel"  >> booltst.log

	echo --------------------------------------------------	>> booltst.log
	echo SCANNER - ESCAPE          		>> booltst.log
	./$(TARGET)  "ape\&&bee"               	>> booltst.log
	./$(TARGET)  "ape\\&bee"               	>> booltst.log
	./$(TARGET)  "ape\\\&bee"              	>> booltst.log
	
	./$(TARGET)  "\&ape&bee"               	>> booltst.log
	./$(TARGET)  "\\\&ape&bee"             	>> booltst.log
	./$(TARGET)  "ape&bee\\"               	>> booltst.log
	# ./$(TARGET)  "ape&bee\\\&"            >> booltst.log
	
	#./$(TARGET)  "ape&bee\"                >> booltst.log
	             
	echo -------------------------------------------------- >> booltst.log
	echo SCANNER - QUOTE           		>> booltst.log
	./$(TARGET)  ":ape&:&bee"              	>> booltst.log
	#./$(TARGET)  "ape&:bee&:&"cow"         >> booltst.log
	#./$(TARGET)  "ape&:bee&:"              >> booltst.log
	
	./$(TARGET)  "::ape&bee"               	>> booltst.log
	./$(TARGET)  "ape&::bee"               	>> booltst.log
	./$(TARGET)  "ape&bee::"               	>> booltst.log
	
	#./$(TARGET)  "ape&:bee"                >> booltst.log
	#./$(TARGET)  ":ape & bee"              >> booltst.log
	#./$(TARGET)  "ape & bee:"              >> booltst.log
	
	./$(TARGET)  "ape &\:bee\Heeve\:& cow" 	>> booltst.log
	./$(TARGET)  "ape& :bee\\\Heeve: &cow" 	>> booltst.log
		
	echo ------------------------------------------------- >> booltst.log
	echo INTERPRETER               		>> booltst.log
	./$(TARGET)  "a"                       	>> booltst.log
	./$(TARGET)  "a&b"                     	>> booltst.log
	./$(TARGET)  "a\b"                     	>> booltst.log
	#./$(TARGET)  "a^b"                     >> booltst.log
	
	more booltst.log

# [6] DOC ------- make -f SS.makefile doc ----------	
doc:	
	cp ../ext/EX*  .
	-rm -f $(TARGET).doc
	awk -f EX.AWK  $(TARGET).h   > $(TARGET).doc
	awk -f EX.AWK  $(TARGET).c  >> $(TARGET).doc
	-rm -f ex.* EX.*
	ls -al ./$(TARGET)
	more $(TARGET).doc
	


# END makefile for BOOL
#=============================================================================
