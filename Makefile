#============================================================================#
# Project 	CLionProjects\accessSU\Makefile for GETOPT.C
#		For command-line build, i.e. outside an IDE
#============================================================================#
# FUNCTION 	Makefile for proj. GETOPT gcommand-line option-parsing 
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	getopt.h getopt.c
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
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$  cd ./CLionProjects/accessSU
#
#		----- Clean and build default target
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU$ make clean
#			rm -f getopt.o
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU$ make
#			gcc -g -c getopt.c -o getopt.o
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU$ /* DONE */
#
#=============================================================================

# FILES ----------------------------------------------------------------------
HEADERS = getopt.h 
OBJECTS = getopt.o
TARGET  = getopt
.PRECIOUS: $(TARGET) $(OBJECTS)

default: $(TARGET).o

# COMPILE --------------------------------------------------------------------
CC = gcc
# OBJ
CFLAGS = -g

$(TARGET).o:	$(TARGET).c $(HEADERS)
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET).o

# CLEAN ---------------------------------------------------------------------
clean:
	-rm -f getopt.o

		
# END makefile
#=============================================================================
