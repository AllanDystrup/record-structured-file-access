#============================================================================#
# Project 	C:\Users\allan\CLionProjects\ACCESS\VA.Makefile
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
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$ cd ./CLionProjects/ACCESS
#
#		----- Clean and build default target
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS$ make -f VA.Makefile realclean
#			rm -f va/*
#			ls -al va
#			total 0
#			drwxrwxrwx 1 allan allan 512 Feb 15 08:13 .
#			drwxrwxrwx 1 allan allan 512 Feb 15 08:10 ..
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS$ make -f VA.Makefile /*default*/
#			gcc -DMAIN -DDEBUG  -g -c va.c -o va/va.o
#			# LINK EXE ------------
#			gcc va/va.o -Wall -lm -o va/va
#
#		----- Test the build executable
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS$ ./va > va.create.log
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
#			allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS$ ./va > va.lookup.log
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
#		----- Extract documentation
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS$ make -f VA.Makefile doc

#
#=============================================================================

# FILES ----------------------------------------------------------------------
ROOT    = va
HEADER  = $(ROOT).h 
SOURCE  = $(ROOT).c
OBJECT  = $(ROOT)/$(ROOT).o
TARGET  = $(ROOT)/$(ROOT)
.PRECIOUS: $(SOURCE) $(HEADER)

default: $(TARGET)


# [1] ---------- make -f VA.makefile [clean | realclean]
clean:
	-rm -f $(OBJECT)
	-rm -f $(TARGET)
	-ls -al $(ROOT)

	# ---------- make -f VA.makefile realclean
realclean:
	-rm -f $(ROOT)/*
	-ls -al $(ROOT)


# [2] ---------- make -f VA.makefile /*default*/
CC = gcc

# EXE
CFLAGS = -DMAIN -DDEBUG  -g
# OBJ
# CFLAGS = -g

	# COMPILE OBJ ----------
$(OBJECT):	$(SOURCE) $(HEADER)
	$(CC) $(CFLAGS) -c $(SOURCE) -o $(OBJECT) 


	# LINK EXE ------------
LIBS = -lm
$(TARGET):	$(OBJECT)
	$(CC) $(OBJECT) -Wall $(LIBS) -o $(TARGET)


# [2] ---------- make -f VA.makefile test ----------
test_create:
	@echo "Patience, please... (CTRL-Z to abort)"
	$(TARGET) >$(ROOT)/$(ROOT).create.log
	# more $(ROOT)/$(ROOT).create.log

test_lookup:
	cp $(ROOT)/$(ROOT).key .
	$(TARGET) >$(ROOT)/$(ROOT).lookup.log
	rm -f ./$(ROOT).key
	#more $(ROOT)/$(ROOT).lookup.log



# [4] ---------- make -f VA.makefile doc ----------	
doc:
	cp ./UTIL/ext/EX* .
	-rm -f $(ROOT)/$(ROOT).doc
	awk -f EX.AWK  $(HEADER)   > $(ROOT)/$(ROOT).doc
	awk -f EX.AWK  $(SOURCE)  >> $(ROOT)/$(ROOT).doc
	-rm -f ex.* EX.*
	ls -al ./$(ROOT)
	more $(ROOT)/$(ROOT).doc
	
		
# END makefile
#=============================================================================
