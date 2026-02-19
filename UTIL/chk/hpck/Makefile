#============================================================================#
# Project 	CLionProjects\access\hpck\MAKEFILE
#	For command-line build, i.e. outside the:
#     	C:\Program Files\JetBrains\CLion 2025.3\bin\clion64.exe IDE ninja env.
#============================================================================#
# FUNCTION 	Plain Vanilla Makefile for proj. hpck : check of runtime heap
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	hpck.c hpck.h
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
#           	for: gcc (compile), ld (link) and make, using Makefile]
#		Windows Terminal (?Cmd Prompt?):
#		Microsoft Windows [Version 10.0.19045.6691]
#		(c) Microsoft Corporation. All rights reserved.
#
#		----- Switch to Ubuntu for Linux build tools
#		C:\Users\allan>		wsl -d Ubuntu -u allan
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$      cd ./CLionProjects/access/check/hpck
#
#		----- Clean ---
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/util/chk/hpck$ make -f HPCK.Makefile clean
# 			rm -f hpck.o
# 			rm -f hpck
#			ls -al
#
#		----- Build default target -----
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/util/chk/hpck$ make -f HPCK.Makefile
#			gcc -DMAIN -DDEBUG  -g -c hpck.c -o hpck.o
#			gcc hpck.o -Wall -lm -o hpck

#		----- Test the build executable
#		Allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/hpck$ ./hpck
#			string1 : [ABCDEFGHIJ]
#			string2 : [12345]
#			ERROR: Malloc Area Corrupted [vfree]
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/hpck$ /* DONE */
#
#=============================================================================

# [0] FILES ------------------------------------------------------------------
HEADERS = hpck.h 
OBJECTS = hpck.o
TARGET  = hpck
.PRECIOUS: $(TARGET) $(OBJECTS)

default: $(TARGET)


# [1] CLEAN ----- make -f HPCK.Makefile clean --------------------------------
clean:
	-rm -f hpck.o
	-rm -f hpck
	ls -al
	
# [2] BUILD ----- COMPILER-LINKER -------------------------------------------
CC = gcc
# TEST DRIVER
CFLAGS = -DMAIN -DDEBUG  -g
# LIB
# CFLAGS = -g
LIBS = -lm
default: $(TARGET)
		
# 	----- COMPILE -----
$(TARGET).o:	$(TARGET).c $(HEADERS)
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET).o

# 	----- LINK -----
$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(TARGET)
	ls -al
	
# [3] TEST -------------------------------------------------------------------
test:
	./$(TARGET)


# END makefile
#=============================================================================
