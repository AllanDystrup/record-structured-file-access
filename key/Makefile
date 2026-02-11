#============================================================================#
# Project 	CLionProjects\access\key\MAKEFILE
#	For command-line build, i.e. outside the:
#     	C:\Program Files\JetBrains\CLion 2025.3\bin\clion64.exe IDE ninja env.
#============================================================================#
# FUNCTION 	Plain vanilla Makefile for proj. key 
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	key.c key.h
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
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$  cd ./CLionProjects/accessSU/key

#		----- Clean and build default $(TARGET).XX where XX is [AA|SS] 
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU/key$ make realclean ACCESS=XX
#		   ----------   VA   ----------			----------   SS   ----------
#			rm -f key.o					rm -f key.o
#			rm -f key.VA					rm -f key.SS				
#			rm SOUL.idx					rm SOUL.idx
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU/key$ make ACCESS=XX
#		----------   VA   ----------			----------   SS   ----------
#    cp      SOUL.idx.VA SOUL.idx				cp      SOUL.idx.SS SOUL.idx
#    gcc -DDEBUG -c ../VA/VA.c -o ../VA/VA.o			gcc -DDEBUG -c ../SS/SS.c -o ../SS/SS.o
#    gcc -DMAIN -DDEBUG -DVA -g -c key.c -o key.o		gcc -DMAIN -DDEBUG -DSS -g -c key.c -o key.o
#    gcc key.o ../VA/VA.o ../../find/tbm/tbm.o -Wall -lm -o key gcc key.o ../SS/SS.o ../../find/tbm/tbm.o -Wall -lm -o key
#    cp key key.VA						cp key key.SS
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU/key$ make test ACCESS=XX
#
#		----------   test VA   ----------			----------   test SS   ----------
#   		cp      SOUL.idx.VA SOUL.idx			cp      SOUL.idx.SS SOUL.idx
#   		./key.VA					./key.SS
#
#    					KMD Index access testdriver, Version 0.1.0
#    					MOD[key.c] VER[0.1.0 Pre] DAT[92/07/10] DEV[ad dec]
#   					Copyright (c) KommuneData I/S 1992
#
#    	Opening VA-file[soul.idx], mode[RO] :			Opening SS-file[soul.idx], mode[RO] :
#    	Read VA-header: dwArSize[69680], dwArUsed[3322],        Read SS-header: dwIsize[6199], dwIused[3322],
#			wArElSize[8], cFill[ ]			 		wKsize[5]
#    	Index Keyrecords : Size=[69680], Used=[3322]		Index Keyrecords : Size=[6199], Used=[3322]
#    	Index Loadfactor=4					Index Loadfactor=53
#
#    	========== DUMP OF INDEX [VA] ==========		========== DUMP OF INDEX [SS] ==========
#								INDEX:  openstatus[IOPEN]
#							        openmode[RO]
#							        filename[soul.idx] size: key[5] key[6199] used[3322]
#
#    					========== DUMP OF BUFFER [VA & SS] =========
#
#    				20200-20202,20203,20204-20206,20207,2099#-**************
#   		 		********************************************************
#				: etc.
#    				********************************************************]
#
#    				eKeyCacheAlloc: dwCsize[99], rResize[0.00], dwNewSize[100]
#    				Expanded KeyValue : [20200]
#    				Cache#[1] Key#[20200]
#
#    		Key[20200]-(lookup)->FlatfileOffset[293258]	Key[20200]-(hash)->HashfileKeyRecord[3186]
#									HashfileKeyOffset[202602]
#								SUM OK : lookup=[2], probe=[2], probe/lookup=[1.00]
#								SUM ALL: lookup=[2], probe=[2], probe/lookup=[1.00]
#   						 20200
#    						<I>
#   						: etc.
#
#					========== DUMP OF CACHE [VA & SS] ==========
#					CACHE:  Index[   1] : Offset[293258]
#					CACHE:  Index[   2] : Offset[293271]
#					CACHE:  Index[   3] : Offset[293317]
#					CACHE:  Index[   4] : Offset[293360]
#					CACHE:  Index[   5] : Offset[293405]
#					CACHE:  Index[   6] : Offset[293445]
#					CACHE:  Index[   7] : Offset[293501]
#					CACHE:  Index[   8] : Offset[293550]
#
#					========== DUMP OF CACHE [VA & SS] ==========
#					CACHE:  Index[   1] : Offset[293258]
#					20200
#					<I>
#					: etc.
#
#					BufFil: TOP old[0]->new[1]
#					        BOT old[0]->new[8]
#					wBFrest[512] - wBFBlock[511]
#					wBFrest[453] - wBFBlock[452]
#					wBFrest[410] - wBFBlock[409]
#					wBFrest[365] - wBFBlock[364]
#					wBFrest[325] - wBFBlock[324]
#					wBFrest[269] - wBFBlock[268]
#					wBFrest[220] - wBFBlock[219]
#
#					========== DUMP OF BUFFER =========
#						[20200
#						<I>
#						: etc.
#
#    			Enter code (H:HELP) [F|P|U|C|D|N|L|R|M|S|K|X|H|Q] -> H
# 
#			eKeyListScroll() function SCROLL CODES :
#      			+================================================================+
#        		|  F : MOVE   pos. of    bufferwindow first   8 slots   in cache |
#        		|  P : MOVE   pos. of    bufferwindow up      8 slots   in cache |
#        		|  U : MOVE   pos. of    bufferwindow up      1  slot   in cache |
#        		|  C : KEEP   pos. of    bufferwindow curr    8 slot    in cache |
#        		|  D : MOVE   pos. of    bufferwindow down    1  slot   in cache |
#        		|  D : MOVE   pos. of    bufferwindow down    1  slot   in cache |
#        		|  N : MOVE   pos. of    bufferwindow down    8 slots   in cache |
#        		|  L : MOVE   pos. of    bufferwindow last    8 slots   in cache |
#        		+----------------------------------------------------------------+
#        		|  M : ENTER  pos.   of  bufferwindow  :   new   slot   in cache |
#        		|  R : ENTER  height of  bufferwindow  :   new   #slots in cache |
#        		|  S : ENTER  pos/height bufferwindow  :   new   slot & #slots   |
#        		+----------------------------------------------------------------+
#       		|  K : ENTER  list       of key values :   new   array for cache |
#        		|  X : ENTER  expr.      of key-class  :   new   array for cache |
#       		 +----------------------------------------------------------------+
#        		|  H : HELP   options f. eKeyListScroll()                        |
#        		|  Q : QUIT   function   eKeyListScroll()                        |
#       		|  A : ABORT  program    key.c                                   |
#        		+================================================================+
#
#    			Enter code (H:HELP) [F|P|U|C|D|N|L|R|M|S|K|X|H|Q] ->
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU/key$/* DONE */
#
#=============================================================================


# ACCESS METHOD passed as make parameter -------------------------------------
# ACCESS METHOD: [VA | SS], for Virtual Array or Scatter Storage (HASH)
# cmdline use: 	(default $(TARGET).XX where XX is [AA|SS]) 
#		make realclean ACCESS=XX
#		make ACCESS=XX	    
#             	make test ACCESS=XX
#		make log  ACCESS=XX 
# ACCESS  = [VA | SS]

TARGET  = key

# PROJECT FILES --------------------------------------------------------------
HEADERS = $(TARGET).h ../$(ACCESS)/$(ACCESS).h ../../find/tbm/tbm.h ../../find/ac/ac.h ../../find/bm/bm.h

OBJECTS = $(TARGET).o ../$(ACCESS)/$(ACCESS).o ../../find/tbm/tbm.o


.PRECIOUS: $(HEADERS)

default: $(TARGET)


# COMPILE --------------------------------------------------------------------
CC = gcc
# EXE
CFLAGS = -DMAIN -DDEBUG -D$(ACCESS) -g
# OBJ
# CFLAGS = -D$(ACCESS) -g

# EXAMPLE (default TARGET): make ACCESS=VA   or   make ACCESS=SS
$(TARGET).o:	$(TARGET).c $(HEADERS)
	cp SOUL.idx.$(ACCESS) SOUL.idx
	$(CC) -DDEBUG -c ../$(ACCESS)/$(ACCESS).c -o ../$(ACCESS)/$(ACCESS).o
	$(CC) $(CFLAGS) -c $(TARGET).c -o $(TARGET).o


# LINK -----------------------------------------------------------------------
LIBS = -lm
$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $(TARGET)
	cp $(TARGET) $(TARGET).$(ACCESS)

# TEST -----------------------------------------------------------------------
# EXAMPLE: make test ACCESS=VA   or   make test ACCESS=SS
.PHONY: test
test .IGNORE :
	cp SOUL.idx.$(ACCESS) SOUL.idx
	cp SOUL.idx SOUL2.idx
	-./$(TARGET).$(ACCESS)
	ls -al

# LOG ------------------------------------------------------------------------
.PHONY: log
log .IGNORE :
	-rm -f key$(ACCESS).txt
	-./key > key$(ACCESS).txt		
	-ls -al
	-head -n 30  key$(ACCESS).txt

# CLEAN ----------------------------------------------------------------------
.PHONY: realclean
realclean:
	-rm -f key.o
	-rm -f key.$(ACCESS)
	-rm SOUL.idx
	-rm SOUL2.idx

.PHONY: clean
clean:
	-rm -f key.o


# END makefile
#=============================================================================
