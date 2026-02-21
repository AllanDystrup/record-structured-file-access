#============================================================================#
# Project 	C:\Users\allan\CLionProjects\util\crc\Makefile for CRC
#		For command-line build, i.e. outside an IDE
#============================================================================#
# FUNCTION 	Makefile for proj. CRC command-line CRC generation
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	crc.h crc.c
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
# ============================================================================


# ============================================================================
#				ALL
#=============================================================================

# FILES
B =  ../..
U =  ..

HDRS = $B/va.h $B/ss.h $B/index.h $B/key.h $U/find/TBM//tbm.h $U/getopt/getopt.h $B/access.h $B/general.h
SRCS = $B/va.c $B/ss.c $B/index.c $B/key.c $U/find/TBM//tbm.c $U/getopt//getopt.c
OBJS = $B/va/va.o $B/ss/ss.o $B/index//index.o $B/key/key.o $U/find/TBM/tbm.o $U/getopt/getopt.o
EXEC = $B/va/va   $B/ss/ss   $B/index/index    $B/key/key   $U/find/TBM/tbm  			


# ============================================================================
#				CRC
#=============================================================================
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/util/crc$ make -f CRC.Makefile clean
# 		rm -f crc.o
# 		ls -al
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/util/crc$ make -f CRC.Makefile
#		gcc -DMAIN -g -c crc.c -o crc.o
#		gcc crc.o -Wall -lm -o crc
#		ls -al
#
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/util/crc$ make -f CRC.Makefile test
#		./crc crc.c
#		CCITT CRC (REVERSE) for    crc.c   is   [99F0]
#		ls -al
#
#  /* DONE */


# [0] FILES ----------------------------------------------------------------
HEADERS = crc.h 
OBJECT  = crc.o
TARGET  = crc

TMPCRC  = access.crc
.PRECIOUS: $(TARGET) $(OBJECTS)
default: $(TARGET)


# [1] CLEAN ----- make -f CRC.Makefile clean --------------------------------
clean:
	rm -f $(TARGET).o
	ls -al


# [2] BUILD -----  make -f CRC.Makefile -------------------------------------
CC = gcc
# 	----- COMPILE -----
#CFLAGS = -DMAIN -DDEBUG -g
CFLAGS = -DMAIN -g
$(TARGET).o:	$(TARGET).c $(HEADERS)
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET).o

# 	----- LINK -----
LIBS = -lm
$(TARGET):	$(OBJECT)
	$(CC) $(OBJECT) -Wall $(LIBS) -o $(TARGET)
	ls -al

# TEST ----- make -f CRC.Makefile test ---------------------------------------

test:
	./crc crc.c
	ls -al


# GENERATE ----- make -f CRC.Makefile access ---------------------------------
# Calculate CRCs for all project ACCESS source files
# (Assumes .o and exe images are available for all sub-projects)
access:
	rm -f $(TMPCRC)

	@echo ===CHECKSUMMING SRC===
	@echo --------------------------------------------------  >  $(TMPCRC)
	@echo DATE:       		>>  $(TMPCRC)
	date          	  		>>  $(TMPCRC)
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo SIZE.H:     		>>  $(TMPCRC)
	ls -Fglp $(HDRS)  		>>  $(TMPCRC)
	@echo LINES   WORDS   BYTES   				 >>  $(TMPCRC)
	wc  $(HDRS)       		>>  $(TMPCRC)
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo SIZE.C:     		>>  $(TMPCRC)
	ls -Fglp $(SRCS)  		>>  $(TMPCRC)
	@echo LINES   WORDS   BYTES   				 >>  $(TMPCRC)
	wc  $(SRCS)       		>>  $(TMPCRC)
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo SIZE.O:     		>>  $(TMPCRC)
	ls -Fglp $(OBJS)  		>>  $(TMPCRC)
	@echo LINES   WORDS   BYTES   				 >>  $(TMPCRC)
	wc  $(OBJS)       		>>  $(TMPCRC)
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo SIZE EXEC:  		>>  $(TMPCRC)
	ls -Fglp $(EXEC)  		>>  $(TMPCRC)
	@echo LINES   WORDS   BYTES   				 >>  $(TMPCRC)
	wc  $(EXEC)       		>>  $(TMPCRC)
	
	@echo -------------------------------------------------- >>  $(TMPCRC)
	@echo CRC:        	>>  $(TMPCRC)
	./crc $B/va.h		>>  $(TMPCRC)
	./crc $B/va.c		>>  $(TMPCRC)
	./crc $B/ss.h		>>  $(TMPCRC)
	./crc $B/ss.c		>>  $(TMPCRC)
	./crc $B/index.h	>>  $(TMPCRC)
	./crc $B/index.c	>>  $(TMPCRC)
	./crc $B/key.h		>>  $(TMPCRC)
	./crc $B/key.c		>>  $(TMPCRC)
	./crc $U/find/TBM//tbm.h >> $(TMPCRC)
	./crc $U/find/TBM/tbm.c	 >> $(TMPCRC)

	more $(TMPCRC)

log:
	./$(TARGET) CRC.C 2>&1 >$(TARGET).log


# [6] DOC ------- make -f CRC.Makefile doc ----------	
doc:	
	cp ../ext/EX*  .
	-rm -f $(TARGET).doc
	awk -f EX.AWK  $(TARGET).h   > $(TARGET).doc
	awk -f EX.AWK  $(TARGET).c  >> $(TARGET).doc
	-rm -f ex.* EX.*
	ls -al ./$(TARGET)
	more $(TARGET).doc
	
xref:
	  ------- make -f CRC.Makefile xref -------
	rm -f $(TARGET).xrf
	../xrf/XRF $(TARGET).c -o $(TARGET).xrf
	ls -al
	more $(TARGET).xrf

		
# END makefile for CRC
#=============================================================================
