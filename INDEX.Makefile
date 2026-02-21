#============================================================================#
# Project 	CLionProjects\access\index\MAKEFILE
#	For command-line build, i.e. outside the:
#     	C:\Program Files\JetBrains\CLion 2025.3\bin\clion64.exe IDE ninja env.
#============================================================================#
# FUNCTION 	Plain vanilla Makefile for proj. index 
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	index.c index.h
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
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$  cd ./CLionProjects/access/index

#		----- Clean and build default target
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/ACCESS$ make -f INDEX.Makefile realclean ACCESS=VA
#			rm -f INDEX/*.o
#			rm -f INDEX/INDEX.$(ACCESS)
#			#-rm INDEX/SOUL.IDX

#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/index$ make /*default*/
#			----------   SS   ----------
#			gcc -DMAIN -DDEBUG -DSS -g -c index.c -o index.o
#			gcc index.o ../SS/SS.o ../../find/tbm/tbm.o -Wall -lm -o index.SS
#			cp index index.SS
#			----------   VA   ----------
#			gcc -DMAIN -DDEBUG -DVA -g -c index.c -o index.o
#			gcc index.o ../VA/VA.o ../../find/tbm/tbm.o -Wall -lm -o index.VA
#			ccp index index.VA
#
#
#		Allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/index$ 
#		./index -k5 -h6144 -d SOUL.DAT -v -t 2>&1 >idx_create.txt	
#
#			KMD Index Generator, Version 0.1.0
#			MOD[index.c] VER[0.1.0 Pre] DAT[92/07/10] DEV[ad dec]
#			Copyright (c) KommuneData I/S 1992
#	
#				Option = 'k'    Argument = "5"
#				Option = 'h'    Argument = "6144"
#				Option = 'd'    Argument = "SOUL.DAT"
#				Option = 'v'    Argument = "<empty>"
#				Option = 't'    Argument = "<empty>"
#
#			Genererer indexfil til start-st?rrelse, vent venligst ...
#			Index keyrecords : Size=[6199], Used=[0]
#			Index loadfactor : Load=[0]
#
#		----------   SS   ----------          	----------   VA   ----------     
#		Entering: KEY[00000] <-> OFFSET[0]  	Entering: KEY[00000] <-> OFFSET[0]                 
#		Entering: KEY[00001] <-> OFFSET[13]    	Entering: KEY[00001] <-> OFFSET[13]             
#		Entering: KEY[00002] <-> OFFSET[450]   	Entering: KEY[00002] <-> OFFSET[450]                       
#		:					:
#		Entering: KEY[69678] <-> OFFSET[460986] Entering: KEY[69678] <-> OFFSET[460986]                
#		Entering: KEY[69679] <-> OFFSET[461057] Entering: KEY[69679] <-> OFFSET[461057]	
#
#	Key[01140]-(lookup)->FlatfileOffset[200050]	                            
#	Key[01141]-(lookup)->FlatfileOffset[200206]	 
#	Index keyrecords : Size=[6199], Used=[3322]	Index keyrecords : Size=[6199], Used=[3322]
#	Index loadfactor : Load=[53]			Index loadfactor : Load=[53]
#
#			Enter a key value 5 chars -> 01140
#			Key[01140]-(lookup)->FlatfileOffset[200050]
#
#	  		 01140
#	   		Indkomst, der ikke vedrrer erhvervsmssig 
#	   		virksomhed, beskattes kun i det omfang den 
#	   		overstiger <I> kr., jf. fondsbeskatnings-
#	   		lovens paragraf 3.
#	
#			Enter a key value 5 chars ->
#			:
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/access/index$ /* DONE */
#
#=============================================================================

# [0] FILES ------------------------------------------------------------------
# ACCESS METHOD: [VA | SS], for Virtual Array or Scatter Storage (HASH)
# cmdline use: 	(default $(TARGET).XX where XX is [AA|SS]) 
#		make -f INDEX.Makefile realclean ACCESS=XX
#		make -f INDEX.Makefile ACCESS=XX	    
#             	make -f INDEX.Makefile test ACCESS=XX
#		make -f INDEX.Makefile log  ACCESS=XX 
# ACCESS  = [VA | SS]

TARGET  = INDEX
HEADERS = INDEX.h  GENERAL.h $(ACCESS).h  UTIL/find/TBM/tbm.h UTIL/getopt/GETOPT.H
OBJECTS =  $(TARGET)/$(TARGET).o  $(TARGET)/$(ACCESS).o $(TARGET)/tbm.o $(TARGET)/getopt.o

.PRECIOUS: $(HEADERS)
default: $(TARGET)


# [1] CLEAN -------------------------------------------------------------------
  
# 	----- make -f INDEX.Makefile realclean ACCESS=[VA | S] -----
.PHONY: realclean
realclean:
	-rm -f $(TARGET)/$(TARGET)
	-rm -f $(TARGET)/*.o
	#-rm -f $(TARGET)/$(TARGET).o
	#-rm -f $(TARGET)/$(ACCESS).o
	#-rm -f $(TARGET)/tbm.o
	#-rm -f $(TARGET)/getopt.o

	-rm -f $(TARGET)/SOUL.IDX
	-rm -f $(TARGET)/SOUL.IDX.$(ACCESS)
	-rm -f $(TARGET)/$(TARGET).$(ACCESS)

	ls -al . ./$(TARGET)

#	----- make -f INDEX.Makefile clean ACCESS=[VA | SS] -----
.PHONY: clean
clean:
	-rm -f $(TARGET)/$(TARGET).o

	-rm -f $(TARGET)/SOUL.IDX
	-rm -f $(TARGET)/SOUL.IDX.$(ACCESS)


# [2] BUILD -------------------------------------------------------------------

#	----- make -f INDEX.Makefile ACCESS=[VA | SS] -----
CC = gcc
# TEST DRIVER 
CFLAGS = -g -DMAIN -DDEBUG -D$(ACCESS)
# LIB
# CFLAGS = -g

# 	----- OBJ -----
$(TARGET)/$(TARGET).o:	$(TARGET).c $(HEADERS)
	$(CC) -g -DDEBUG -c $(ACCESS).c -o $(TARGET)/$(ACCESS).o
	$(CC) -g -DDEBUG -c UTIL/find/TBM/TBM.c -o $(TARGET)/tbm.o
	$(CC) -g -DDEBUG -c UTIL/getopt/GETOPT.c -o $(TARGET)/getopt.o
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET)/$(TARGET).o
	

#	----- EXE -----
LIBS = -lm
$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(TARGET)/$(TARGET)
	cp $(TARGET)/$(TARGET) $(TARGET)/$(TARGET).$(ACCESS)
	ls -al . ./$(TARGET)


# [3] TEST -------------------------------------------------------------------

#	----- make -f INDEX.Makefile test ACCESS=[VA | SS] -----
.PHONY: test
test:
	# Generate new SOUL.IDX and copy to SOUL.IDX.[VA|SS]
	rm -f SOUL.DAT
	cp $(TARGET)/SOUL.DAT .
	./$(TARGET)/$(TARGET).$(ACCESS) -k5 -h6144 -d SOUL.DAT -v -t

testclean:
	cp SOUL.IDX ./$(TARGET)
	cp SOUL.IDX ./$(TARGET)/SOUL.IDX.$(ACCESS)
	rm -f SOUL*
	ls -al . ./$(TARGET)
.PHONY: log
log:
	-./index -k5 -h6144 -d SOUL.DAT -v -t 2>&1 >idx_create_$(ACCESS).txt
	-ls -al
	-head -n 30 idx_create_$(ACCESS).txt


# [4] DOC ------- make -f INDEX.makefile doc ----------	
doc:
	cp ./util/ext/EX* .
	-rm -f $(TARGET)/$(TARGET).doc
	awk -f EX.AWK  $(TARGET).h   > $(TARGET)/$(TARGET).doc
	awk -f EX.AWK  $(TARGET).c  >> $(TARGET)/$(TARGET).doc
	-rm -f ex.* EX.*
	ls -al ./$(TARGET)
	more $(TARGET)/$(TARGET).doc

xref:
	rm -f $(TARGET)/$(TARGET).xrf
	./UTIL/xrf/XRF $(TARGET).c -o $(TARGET)/$(TARGET).xrf
	ls -al
	more $(TARGET)/$(TARGET).xrf

	
# END makefile INDEX.Makefile
#=============================================================================
