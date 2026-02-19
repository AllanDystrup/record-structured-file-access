#============================================================================#
# Project 		<ac-text-search>\BM\MAKEFILE
# 		For command-line build (i.e. outside CLion IDE ninja env.)
#============================================================================#
# FUNCTION 	plain vanilla Makefile for proj. Boyer-Moore (ac) search
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	ac.c ac.c
#
# PROGRAMMER 	Allan Dystrup.
# 		COPYRIGHT (c) Allan Dystrup, 1991, 2025.
#
# VERSION 	Revision 1.0 1992/03/20 13:40	 	Allan_Dystrup
#		Initial revision
# 		Revision 1.8 2025/12/09 11:00	 	Allan_Dystrup
# 		Port to UBUNTU  on Win.10 / WSL, Using CLion for Linux
#
# 		Port to Win.10 native/terminal,  Using CLion for Windows
#		[Using UBUNTU wsl.localhost/Ubuntu/usr/bin build tools
#               for: gcc (compile), ld (link) and make, using Makefile]
#		Windows Terminal (‘Cmd Prompt’):  
#		Microsoft Windows [Version 10.0.19045.6691]
#		(c) Microsoft Corporation. All rights reserved.
#
#		----- Switch to Ubuntu for Linux build tools
#		C:\Users\allan>		wsl -d Ubuntu -u allan
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$ 	cd ./CLionProjects/ac
#
#		----- Clean generated project files -----
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/find/ac$ make -f BM.Makefile clean
#			rm -f ac
#			rm -f ac.o ../../err/error.o ../../bool/bool.o
#			# ls -al . ../../err ../../bool
#
#		----- Build executable (test driver) -----	
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/find/ac$ make -f BM.Makefile
#			gcc -g -c ../../err/error.c -o ../../err/error.o
#			gcc -g -c ../../bool/boo.c  -o ../../bool/bool.o
#			gcc -g -DMAIN -c ac.c -o ac.o
#
#			gcc ac.o ../../err/error.o ../../bool/bool.o -Wall -lm -o ac
#
#		----- Test the build executable
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/find/ac$ make -f BM.Makefile log
# 			rm -f ac.log
#			./ac p  ac.c  >ac.log
#			ls -al
#			more ac.log
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/find/ac$ make -f BM.Makefile doc
# 			cp ../../ext/EX*  .
# 			rm -f ac.doc
#			awk -f EX.AWK  ac.h   > ac.doc
#			awk -f EX.AWK  ac.c  >> ac.doc
#			rm -f ex.* EX.*
#			ls -al ./ac
#			more ac.doc
#
#=============================================================================


# [0] FILES ------------------------------------------------------------------
TARGET  = ac
ERROR   = ../../err/error
BOOL    = ../../bool/bool
HEADERS = $(TARGET).h $(ERROR).h $(BOOL).h
SOURCES = $(TARGET).c $(ERROR).c $(BOOL).c
OBJECTS = $(TARGET).o $(ERROR).o $(BOOL).o

# TARGETS
default:   $(TARGET)
.PRECIOUS: $(HEADERS) $(SOURCES)
.PHONY:    default clean


# [1] CLEAN -------------------------------------------------------------------
clean:
	-rm -f $(TARGET)
	-rm -f $(OBJECTS)
	ls -al . ../../err ../../bool


# [2] BUILD -------------------------------------------------------------------
CC = gcc
# CFLAGS = -DMAIN -g
CFLAGS = -g

# 	----- COMPILE ----
$(TARGET).o:	$(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $(ERROR).c -o $(ERROR).o
	$(CC) $(CFLAGS) -c $(BOOL).c  -o $(BOOL).o
	#$(CC) $(CFLAGS) -DMAIN -DDEBUG -c $(TARGET).c -o $(TARGET).o
	$(CC) $(CFLAGS) -DMAIN -c $(TARGET).c -o $(TARGET).o
	ls -al . ../../err ../../bool

# 	----- LINK -----
LIBS = -lm
$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(TARGET)
	ls -al

# [3] TEST --------------------------------------------------------------------
# USAGE       ac [<options>] <boolexpr> <file>, where :
#                <options>  -N to force a NFSA search (default DFSA)
#                           -U to force a case-INsensitive search
#                <boolexpr> is a boolean expression of search phrases
#                           example: This&^(That/Those)
#                <file>     is the file to search for boolexpr, line by line.

test:
	-./ac -N "FSA&^(Dfsa/Nfsa)"     ./ac.c
	echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	-./ac -U "(general/bool/error)" ./ac.c  
	ls -al	

log:
	-rm -f $(TARGET).log
	-./ac -N "FSA&^(Dfsa/Nfsa)" ./ac.c        >$(TARGET).log
	-./ac -U "(general/bool/error)" ./ac.c   >>$(TARGET).log
	ls -al	
	more $(TARGET).log


# [4] DOC ---------------------------------------------------------------------
doc:	
	cp ../../ext/EX*  .
	-rm -f $(TARGET).doc
	awk -f EX.AWK  $(TARGET).h   > $(TARGET).doc
	awk -f EX.AWK  $(TARGET).c  >> $(TARGET).doc
	-rm -f ex.* EX.*
	ls -al 
	more $(TARGET).doc

# END makefile
#=============================================================================
