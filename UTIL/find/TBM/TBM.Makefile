#============================================================================#
# Project 		<bm-text-search>\TBM\MAKEFILE
#				For command-line build (i.e. outside the/mnt/c/Users/allan/CLionProjects$ CLion IDE ninja env.)
#============================================================================#
# FUNCTION 	    Plain vanilla Makefile for proj. Tuned Boyer-Moore (tbm) search
# SYSTEM 	    Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		        Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
# SEE ALSO 	    tbm.c tbm.c
# PROGRAMMER 	Allan Dystrup.
# 		        COPYRIGHT (c) Allan Dystrup, 1991, 2025.
# VERSION 	    Revision 1.0 1992/03/20 13:40	 	Allan_Dystrup
#				   Initial revision
# 			Revision 1.8 2025/12/09 11:00	 	Allan_Dystrup
# 		        Port to UBUNTU  on Win.10 / WSL, Using CLion for Linux
#
# 			Port to Win.10 native/terminal,  Using CLion for Windows
#			[Using UBUNTU wsl.localhost/Ubuntu/usr/bin build tools
#                       for: gcc (compile), ld (link) and make, using Makefile]
#			Windows Terminal (‘Cmd Prompt’):  
#				Microsoft Windows [Version 10.0.19045.6691]
#				(c) Microsoft Corporation. All rights reserved.
#				----- Switch to Ubuntu for Linux build tools
#				C:\Users\allan>		wsl -d Ubuntu -u allan
#	allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$ 	cd ./CLionProjects/tbm
#				----- Clean and build default target
#	allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/tbm$ make clean
#				rm -f *.o
#				rm -f tbm
#	allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/tbm$ make /*default*/
#				gcc -std=c11 -Wall -c tbm.c -o tbm.o
#				...
#				gcc  tbm.o  -lm -o tbm
#				----- Test the build executable
#	allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/tbm$ make test
#				./tbm p ./tbm.c
#				TBM Search Functions (Testdriver), Version 1.8
#				OD[tbm.c] VER[1.8] DAT[2025/12/08] DEV[ad]
#				Copyright (c) Allan Dystrup 1992, 2025
#
# [6:3]    *                      This module implements a compact, portable and fast BM
# [7:2]    *                      algorithm (ie. a classic BM including a skip-loop).
#				...
# [574:2] } /* END function bDKupper */
#
# Total match of [p] in input stream [./tbm.c]: [510]
#				----- All done!
#	allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/tbm$ /* DONE */
#
#=============================================================================

# ----------------------------------------------------------------------------
# FILES
HEADERS = tbm.h 
OBJECT  = tbm.o
TARGET  = tbm


# TARGETS
.PRECIOUS: $(TARGET) $(SOURCE)
.PHONY: default all clean
default: $(TARGET)
all: default


# CLEAN	----- make -f TBM.Makefile clean -----
clean:
	-rm -f *.o
	-rm -f $(TARGET)


# BUILD -----  make -f TBM.Makefile -----
# BUILD --------------------------------------------------------------------
CC = gcc
# 	----- COMPILE -----
# CFLAGS = -DMAIN -DDEBUG -g
CFLAGS = -g
$(TARGET).o:	$(TARGET).c $(HEADERS)
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET).o

# 	----- LINK -----
LIBS = -lm
$(TARGET):	$(OBJECT)
	$(CC) $(OBJECT) -Wall $(LIBS) -o $(TARGET)
	ls -al

# TEST
test:
	./tbm p ./tbm.c



# END makefile
#=============================================================================
