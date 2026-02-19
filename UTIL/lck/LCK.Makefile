#============================================================================#
# Project 	CLionProjects\access\lck\MAKEFILE
#	For command-line build, i.e. outside the:
#     	C:\Program Files\JetBrains\CLion 2025.3\bin\clion64.exe IDE ninja env.
#============================================================================#
# FUNCTION 	Plain Vanilla Makefile for proj. VA scatter storage
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	lck.c lck.h
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
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$ cd ./CLionProjects/accessMU/lck
#
#		----- Clean and build default target
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/lck$ make -f LCK.Makefile realclean
# 			rm -f lck.o
# 			rm -f lck
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/lck$ make -f LCK.Makefile
# 			gcc -DMAIN -DDEBUG -DUNIX -g -c lck.c -o lck.o
# 			gcc lck.o -Wall -lm -o lck
#
#		----- Test the build executable
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/lck$ make -f LCK.Makefile test
#			./lck
#			KMD Portable Locking Functions (Testdriver), Version 0.1
#			MOD[LCK.C] VER[0.1.0 Exp] DAT[92/08/31] DEV[ad divdec]
#			Copyright (c) Allan Dystrup 1992
#
#			Enter code (H:HELP) [r|w|R|W|uU|sS|tT|mM|fF|gG|bB|hH|qQ] -> h
#
#=============================================================================

# TARGETOS -------------------------------------------------------------------
TARGETOS = UNIX


# [0] FILES ------------------------------------------------------------------
TARGET = lck
HEADER = $(TARGET).h
SOURCE = $(TARGET).c
OBJECT = $(TARGET).o

.PRECIOUS: $(SOURCE) $(HEADER)
default: $(TARGET)


# [1] CLEAN ----- make -f LCK.Makefile realclean ------------------------------
clean:
	-rm -f $(OBJECT)
	ls -al

realclean:
	-rm -f $(OBJECT)
	-rm -f $(TARGET)
	ls -al


# [2] BUILD ----- make -f LCK.Makefile ---------------------------------------
CC = gcc
CFLAGS = -DMAIN -DDEBUG -D$(TARGETOS) -g
# CFLAGS =  -D$(TARGETOS) -g

#	----- OBJ
$(OBJECT):	$(SOURCE) $(HEADER)
	$(CC) $(CFLAGS) -c $(SOURCE) -o $(OBJECT)

# 	----- EXE ------------------------------------------------------------ 
LIBS = -lm
$(TARGET):	$(OBJECT)
	$(CC) $(OBJECT) -Wall $(LIBS) -o $(TARGET)
	ls -al
	

# [3] TEST ----- make -f LCK.Makefile test -----------------------------------
test:
	./lck


# END makefile
#=============================================================================
