#============================================================================#
# Project 	CLionProjects\accessSU\Makefile for utils: GETOPT, CRC
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
#=============================================================================

# ============================================================================
#				ALL
#=============================================================================
HDRS = ./va/va.h ./ss/ss.h ./index/index.h ./key/key.h ../find/tbm/tbm.h getopt.h  general.h access.h 
SRCS = ./va/va.c ./ss/ss.c ./index/index.c ./key/key.c ../find/tbm/tbm.c getopt.c
OBJS = ./va/va.o ./ss/ss.o ./index/index.o ./key/key.o ../find/tbm/tbm.o getopt.o
EXEC = ./va/va   ./ss/ss   ./index/index   ./key/key   ../find/tbm/tbm     			


# ============================================================================
#				GETOPT
#=============================================================================
#  allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU$ make clean1
#		rm -f getopt.o
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU$ make getopt.o
#		gcc -g -c getopt.c -o getopt.o
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU$ /* DONE */

# FILES ----------------------------------------------------------------------
HEADERS1 = getopt.h 
OBJECTS1 = getopt.o
TARGET1  = getopt
.PRECIOUS: $(TARGET1) $(OBJECTS1)

default: $(TARGET1).o

# COMPILE --------------------------------------------------------------------
CC = gcc
# OBJ
CFLAGS = -g

$(TARGET1).o:	$(TARGET1).c $(HEADERS1)
	$(CC) $(CFLAGS) -c $(TARGET1).c -o $(TARGET1).o

# CLEAN ---------------------------------------------------------------------
clean1:
	-rm -f $(TARGET1).o


# ============================================================================
#				CRC
#=============================================================================
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU$ make clean2
# 		rm -f crc.o
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU$ make crc
# 		gcc -DMAIN -DNDEBUG -g -c crc.c -o crc.o
# 		gcc crc.o -Wall -lm -o crc
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU$ make test
#		./crc CRC.C
#		CCITT CRC (REVERSE) for    CRC.C   is   [396B]
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionPro  /* DONE */

# FILES ----------------------------------------------------------------------
HEADERS2 = crc.h 
OBJECTS2 = crc.o
TARGET2  = crc
.PRECIOUS: $(TARGET2) $(OBJECTS2)

TMPCRC   = accessSU.crc
default: $(TARGET2)

# COMPILE --------------------------------------------------------------------
CC = gcc
# OBJ
# ----- Testdriver for module CRC.C
#CFLAGS = -DMAIN -g
# ----- Standalone application for computing file checksums
CFLAGS = -DMAIN -DNDEBUG -g

$(TARGET2).o:	$(TARGET2).c $(HEADERS2)
	$(CC) $(CFLAGS) -c $(TARGET2).c -o $(TARGET2).o

# LINK -----------------------------------------------------------------------
LIBS = -lm
$(TARGET2):	$(OBJECTS2)
	$(CC) $(OBJECTS2) -Wall $(LIBS) -o $(TARGET2)

# TEST -----------------------------------------------------------------------
test:
	rm -f $(TMPCRC)

	@echo ===CHECKSUMMING SRC===
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo DATE:       >   $(TMPCRC)
	date          	  >>  $(TMPCRC)
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo SIZE.H:     >>  $(TMPCRC)
	ls -Fglp $(HDRS)  >>  $(TMPCRC)
	@echo LINES WORDS BYTES   				 >>  $(TMPCRC)
	wc  $(HDRS)       >>  $(TMPCRC)
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo SIZE.C:     >>  $(TMPCRC)
	ls -Fglp $(SRCS)  >>  $(TMPCRC)
	@echo LINES WORDS BYTES   				 >>  $(TMPCRC)
	@echo LINES WORDS BYTES   				 >>  $(TMPCRC)
	wc  $(SRCS)       >>  $(TMPCRC)
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo SIZE.O:     >>  $(TMPCRC)
	ls -Fglp $(OBJS)  >>  $(TMPCRC)
	@echo LINES WORDS BYTES   				 >>  $(TMPCRC)
	wc  $(OBJS)       >>  $(TMPCRC)
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo SIZE EXEC:  >>  $(TMPCRC)
	ls -Fglp $(EXEC)  >>  $(TMPCRC)
	@echo LINES WORDS BYTES   				 >>  $(TMPCRC)
	wc  $(EXEC)       >>  $(TMPCRC)
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo CRC:        >>  $(TMPCRC)
	./crc ./va/va.h		>>  $(TMPCRC)
	./crc ./va/va.c		>>  $(TMPCRC)
	./crc ./ss/ss.h		>>  $(TMPCRC)
	./crc ./ss/ss.c		>>  $(TMPCRC)
	./crc ./index/index.h	>>  $(TMPCRC)
	./crc ./index/index.c	>>  $(TMPCRC)
	./crc ./key/key.h	>>  $(TMPCRC)
	./crc ./key/key.c	>>  $(TMPCRC)
	./crc ../find/tbm/tbm.h	>>  $(TMPCRC)
	./crc ../find/tbm/tbm.c	>>  $(TMPCRC)
	cat $(TMPCRC)

log:
	./$(TARGET2) CRC.C 2>&1 >$(TARGET2).log


# CLEAN ---------------------------------------------------------------------
clean2:
	-rm -f $(TARGET2).o
		
# END makefile
#=============================================================================
