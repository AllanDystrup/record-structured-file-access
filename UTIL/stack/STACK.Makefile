#============================================================================#
# Project 	C:\Users\allan\CLionProjects\util\stack
#		For command-line build, i.e. outside the:
#     	C:\Program Files\JetBrains\CLion 2025.3\bin\clion64.exe IDE ninja env.
#============================================================================#
# FUNCTION 	Plain vanilla Makefile for proj. stack 
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	stack.c stack.h
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
#		C:\Users\allan>	wsl -d Ubuntu -u allan
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$
#
#		----- Clean
#	allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/util/stack$ make -f STACK.Makefile realclean
#			rm -f stack.o
#			ls -al
#
#		----- Build
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/util/stack$ make
#		gcc -DMAIN -DDEBUG -g -c stack.c -o stack.o
#
#		----- Test
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/util/stack$ make test
#			./stack
#				stack[0]=49	// push #0-49 elem
#				stack[1]=48
#				   :
#				stack[48]=1
#				stack[49]=0
#				Stack error 1	// pop #50: error!
#				stack[50]=14
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/util/stack$
#
#
#=============================================================================


# [0] FILES ------------------------------------------------------------------
HEADERS = $(TARGET).h 
OBJECTS = $(TARGET).o 
.PRECIOUS: $(HEADERS)

TARGET  = stack
default: $(TARGET)

# [1] CLEAN ----- make -f STACK.Makefile realclean -----------------------------
.PHONY: realclean
realclean:
	-rm -f $(TARGET).o
	-rm -f $(TARGET)
	ls -al

.PHONY: clean
clean:
	-rm -f $(TARGET).o


# [2] BUILD ----- make -f STACK.Makefile ---------------------------------------
CC = gcc
CFLAGS = -DMAIN -DDEBUG -g
# OBJ
# CFLAGS = -D DEBUG -g

#	----- Compile
$(TARGET).o:	$(TARGET).c $(HEADERS)
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET).o

# 	----- LINK -------------------------------------------------------------
LIBS = -lm
$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(TARGET)
	ls -al


# [3] TEST ------ make -f STACK.Makefile test ----------------------------------
.PHONY: test
test .IGNORE :
	./$(TARGET)
	ls -al

.PHONY: log
log .IGNORE :
	./$(TARGET)  >$(TARGET).txt		
	ls -al
	cat $(TARGET).txt


# [6] DOC ------- make -f STACK.makefile doc ----------	
doc:	
	cp ../ext/EX*  .
	-rm -f $(TARGET).doc
	awk -f EX.AWK  $(TARGET).h   > $(TARGET).doc
	awk -f EX.AWK  $(TARGET).c  >> $(TARGET).doc
	-rm -f ex.* EX.*
	ls -al ./$(TARGET)
	more $(TARGET).doc

xref:
	  ------- make -f STACK.Makefile xref -------
	rm -f $(TARGET).xrf
	../xrf/XRF $(TARGET).c -o $(TARGET).xrf
	ls -al
	more $(TARGET).xrf

	

# END makefile
#=============================================================================
