#============================================================================#
# Project 	CLionProjects\accessSU\Makefile for utils: GETOPT
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
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$  cd ./CLionProjects/ACCESS/UTIL/getopt
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/getopt$ make -f GETOPT.Makefile clean
# 			rm -f getopt.o
#			ls -al
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/getopt$ make -f GETOPT.Makefile
# 			gcc -g -c getopt.c -o getopt.o
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/getopt$ make -f GETOPT.Makefile doc
# 			cp ../ext/EX*  .
#			rm -f getopt.doc
#			awk -f EX.AWK  getopt.h   > getopt.doc
#			awk -f EX.AWK  getopt.c  >> getopt.doc
#			rm -f ex.* EX.*
#			ls -al .
#	
#============================================================================#


# [0] FILES ------------------------------------------------------------------
TARGET = getopt
HEADER = $(TARGET).h
SOURCE = $(TARGET).c
OBJECT = $(TARGET).o

.PRECIOUS: $(TARGET) $(SOURCE) $(HEADER)

default: $(OBJECT)


# [1] CLEAN ------------------------------------------------------------------
clean:
	-rm -f $(OBJECT)
	ls -al


# [2] BUILD ------------------------------------------------------------------
CC = gcc
CFLAGS = -g

$(OBJECT):	$(SOURCE) $(HEADER)
	$(CC) $(CFLAGS) -c $(SOURCE) -o $(OBJECT)
	ls -al


# [3] DOC ----- make -f ERROR.Makefile doc -----------------------------------
doc:	
	cp ../ext/EX*  .
	-rm -f $(TARGET).doc
	awk -f EX.AWK  $(HEADER)   > $(TARGET).doc
	awk -f EX.AWK  $(SOURCE)  >> $(TARGET).doc
	-rm -f ex.* EX.*
	ls -al
	more $(TARGET).doc
		
# END GETOPT.Makefile
#=============================================================================
