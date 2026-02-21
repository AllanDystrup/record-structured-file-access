#============================================================================#
# Project 		<bm-text-search>\BM\MAKEFILE
# 		For command-line build (i.e. outside CLion IDE ninja env.)
#============================================================================#
# FUNCTION 	plain vanilla Makefile for proj. Boyer-Moore (bm) search
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	bm.c bm.c
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
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$ 	cd ./CLionProjects/bm
#
#		----- Clean generated project files -----
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/find/bm$ make -f BM.Makefile clean
#			rm -f bm
#			rm -f bm.o ../../err/error.o ../../bool/bool.o
#			# ls -al . ../../err ../../bool
#
#		----- Build executable (test driver) -----	
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/find/bm$ make -f BM.Makefile
#			gcc -g -c ../../err/error.c -o ../../err/error.o
#			gcc -g -c ../../bool/boo.c  -o ../../bool/bool.o
#			gcc -g -DMAIN -c bm.c -o bm.o
#
#			gcc bm.o ../../err/error.o ../../bool/bool.o -Wall -lm -o bm
#
#		----- Test the build executable
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/find/bm$ make -f BM.Makefile log
# 			rm -f bm.log
#			./bm p  bm.c  >bm.log
#			ls -al
#			more bm.log
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/find/bm$ make -f BM.Makefile doc
# 			cp ../../ext/EX*  .
# 			rm -f bm.doc
#			awk -f EX.AWK  bm.h   > bm.doc
#			awk -f EX.AWK  bm.c  >> bm.doc
#			rm -f ex.* EX.*
#			ls -al ./bm
#			more bm.doc
#
#=============================================================================


# [0] FILES ------------------------------------------------------------------
TARGET  = bm
ERROR   = ../../err/error
BOOL    = ../../bool/bool
HEADERS = $(TARGET).h $(ERROR).h $(BOOL).h
SOURCES = $(TARGET).c $(ERROR).c $(BOOL).c
OBJECTS = $(TARGET).o $(ERROR).o $(BOOL).o

# TARGETS
default:   $(TARGET)
.PRECIOUS: $(TARGET) $(SOURCE)
.PHONY:    default clean


# [1] CLEAN -------------------------------------------------------------------
clean:
	-rm -f $(TARGET)
	-rm -f $(OBJECTS)
	ls -al . ../../err ../../bool


# [2] BUILD -------------------------------------------------------------------
CC = gcc
# CFLAGS = -DMAIN -DDEBUG -g
CFLAGS = -g

# 	----- COMPILE ----
$(TARGET).o:	$(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $(ERROR).c -o $(ERROR).o
	$(CC) $(CFLAGS) -c $(BOOL).c  -o $(BOOL).o
	$(CC) $(CFLAGS) -DMAIN -c $(TARGET).c -o $(TARGET).o
	ls -al . ../../err ../../bool

# 	----- LINK -----
LIBS = -lm
$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(TARGET)
	ls -al

# [3] TEST --------------------------------------------------------------------
test:
	./bm p ./bm.c
	ls -al

log:
	-rm -f $(TARGET).log
	-./$(TARGET) p  $(TARGET).c  >$(TARGET).log
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


xref:
	  ------- make -f STACK.Makefile xref -------
	rm -f $(TARGET).xrf
	../../xrf/XRF $(TARGET).c -o $(TARGET).xrf
	ls -al
	more $(TARGET).xrf

# END makefile
#=============================================================================
