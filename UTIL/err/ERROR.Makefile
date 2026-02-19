#============================================================================#
# Project 	C:\Users\allan\CLionProjects\util\error
#		For command-line build, i.e. outside the:
#     	C:\Program Files\JetBrains\CLion 2025.3\bin\clion64.exe IDE ninja env.
#============================================================================#
# FUNCTION 	Plain vanilla Makefile for proj. error 
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	error.c error.h
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
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/err$ make -f ERROR.Makefile realclean
# 			rm -f error.o
# 			rm -f error
#			ls -al
#
#		----- Build
# allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS/UTIL/err$ make -f ERROR.Makefile
#			gcc -DMAIN -DDEBUG -g -c error.c -o error.o
#			gcc error.o  -Wall -lm -o error
#
#		----- Test
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/util/error$ make test
#			./error
#			======= Testudskrift af samtlige fejlmeddelelser =======
#			=>Force Continue
#			  STOP : [BOOL|BM] : E[000]ARG
#			  Argumentfejl i inddata : forkert aktuel parameter i funktionskald
#			  Programmør: Læs manualside. --- Bruger: Underret Udvikler.
#			    : ETC.
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/util/error$
#
#
#=============================================================================


# [0] FILES --------------------------------------------------------------
TARGET  = error
HEADERS = $(TARGET).h ../../general.h
SOURCE  = $(TARGET).c
OBJECT  = $(TARGET).o 

.PRECIOUS: $(SOURCE) $(HEADERS)
.PHONY: clean realclean test doc log
.IGNORE: test
default: $(TARGET)


# [1] CLEAN ----- make -f ERROR.Makefile realclean ---------------------------
clean:
	-rm -f $(OBJECT)
	-rm -f $(TARGET)
	ls -al

realclean:
	-rm -f $(OBJECT)
	-rm -f $(TARGET)
	-rm -f $(TARGET).log
	-rm -f $(TARGET).doc
	ls -al


# [2] BUILD ------ make -f ERROR.Makefile ------------------------------------
CC = gcc
CFLAGS = -DMAIN -DDEBUG -g
#CFLAGS = -DDEBUG -g
	
#	----- COMPILE  -----
$(OBJECT):	$(SOURCE) $(HEADERS)
	$(CC) $(CFLAGS) -c $(SOURCE) -o $(OBJECT)
	
# 	----- LINK -----
LIBS = -lm
$(TARGET):	$(OBJECT)
	$(CC) $(OBJECT) -Wall $(LIBS) -o $(TARGET)


# [3] TEST -----  make -f ERROR.Makefile test --------------------------------
# EXAMPLE: make test ACCESS=[VA|SS]
test:
	./$(TARGET)
	ls -al

log:
	export SHELL=`which bash`
	-rm -f $(TARGET).log
	-./$(TARGET) 2>$(TARGET).log
	ls -al
	cat $(TARGET).log


# [4] DOC ----- make -f ERROR.Makefile doc -----------------------------------
doc:	
	cp ../ext/EX*  .
	-rm -f $(TARGET).doc
	awk -f EX.AWK  $(TARGET).h   > $(TARGET).doc
	awk -f EX.AWK  $(TARGET).c  >> $(TARGET).doc
	-rm -f ex.* EX.*
	ls -al ./$(TARGET)
	more $(TARGET).doc

# END makefile
#=============================================================================
