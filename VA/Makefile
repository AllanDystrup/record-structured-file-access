#============================================================================#
# Project 	CLionProjects\access\va\MAKEFILE
#		For command-line build, i.e. outside the:
#     	C:\Program Files\JetBrains\CLion 2025.3\bin\clion64.exe IDE ninja env.
#============================================================================#
# FUNCTION 	Plain vanilla Makefile for proj. VA (Virtual Array) storage
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	va.c va.h
#
# PROGRAMMER 	Allan Dystrup.
# 		COPYRIGHT (c) Allan Dystrup, 1991, 2025.
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
#”
#		----- Switch to Ubuntu for Linux build tools
#		C:\Users\allan>		wsl -d Ubuntu -u allan
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$  cd ./CLionProjects/access/va
#		----- Clean and build default target
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/va$ make clean
#			rm -f va.o
#			rm -f va
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/va$ make /*default*/
#			gcc -DMAIN -DDEBUG  -g -c va.c -o va.o
#			gcc va.o -Wall -lm -o va
#		----- Test the build executable
#		Allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/va$ ./va
#			----- VA index creation -----
#			:	Creating VA-file[KEY.VA]
#			:	Opening VA-file[KEY.VA], mode[RW] :
#			:	Creating 10000 elements...
#			:	Deleting 10 of 10000 elements: [#10-#19]...
#			:	Re-initializing 10 of 10000 elements: [#5-#14] to #+100...
#			Closing VA-file
#        		Flushing VA cache to disk
#       		Releasing all VACB resources
#
#			----- VA index LOOKUP -----
#			Creating VA-file[KEY.VA] :
#			Opening VA-file[KEY.VA], mode[RO] :
#				Read  VA-header: dwArSize[9999], dwArUsed[5003], wArElSize[8], cFill[ ]
#			Index Keyrecords:	Size=[9999],	Used=[5003]
#			Index Loadfactor:	Load=[50]
#			Accessing 10000 elements...
#			ACCESSING :	Array[   0] =    0
#			NOTFOUND  :	Array[   1] = 18446744073709551615
#			ACCESSING :	Array[   2] =    2
#			NOTFOUND  :	Array[   3] = 18446744073709551615
#			:
#			Closing VA-file
#				Releasing all VACB resources
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/va$ /* DONE */
#
#=============================================================================

# FILES ----------------------------------------------------------------------
HEADERS = va.h 
OBJECTS = va.o
TARGET  = va
.PRECIOUS: $(TARGET) $(OBJECTS)

default: $(TARGET)

# COMPILE --------------------------------------------------------------------
CC = gcc
# EXE
CFLAGS = -DMAIN -DDEBUG  -g
# OBJ
# CFLAGS = -g

$(TARGET).o:	$(TARGET).c $(HEADERS)
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET).o

# LINK -----------------------------------------------------------------------
LIBS = -lm
$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(TARGET)

# TEST -----------------------------------------------------------------------
test:
	./va

# CLEAN ---------------------------------------------------------------------
clean:
	-rm -f va.o

realclean:
	-rm -f va.o
	-rm -f va
		
# END makefile
#=============================================================================
