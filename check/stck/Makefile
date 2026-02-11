#============================================================================#
# Project 	CLionProjects\access\stck\MAKEFILE
#	For command-line build, i.e. outside the:
#     	C:\Program Files\JetBrains\CLion 2025.3\bin\clion64.exe IDE ninja env.
#============================================================================#
# FUNCTION 	Plain Vanilla Makefile for proj. stck : check of runtime stack
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	stck.c stck.h
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
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$      cd ./CLionProjects/access/check/stck
#		----- Clean and build default target
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/check/stck$ make clean
#			rm -f stck.o
#			rm -f stck
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/stck$ make /*default*/
#			gcc -DMAIN -DDEBUG  -g -c stck.c -o stck.o
#			gcc stck.o -Wall -lm -o stck
#		----- Test the build executable
#		Allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/stck$ ./stck
#			
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/stck$ /* DONE */
#
#=============================================================================

# FILES ----------------------------------------------------------------------
HEADERS = stck.h 
OBJECTS = stck.o
TARGET  = stck
.PRECIOUS: $(TARGET) $(OBJECTS)

# COMPILER-LINKER ------------------------------------------------------------
CC = gcc
# TEST DRIVER
CFLAGS = -DMAIN -DDEBUG -DSDEBUG -g
# LIB
# CFLAGS = -g
LIBS = -lm
default: $(TARGET)

# COMPILE --------------------------------------------------------------------
$(TARGET).o:	$(TARGET).c $(HEADERS)
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET).o

# LINK -----------------------------------------------------------------------
$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(TARGET)

# CLEAN ---------------------------------------------------------------------
clean:
	-rm -f stck.o
	-rm -f stck
	
# END makefile
#=============================================================================
