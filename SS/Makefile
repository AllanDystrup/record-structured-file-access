#============================================================================#
# Project 	CLionProjects\acceHASH\HASH\MAKEFILE
#		For command-line build, i.e. outside the:
#     	C:\Program Files\JetBrains\CLion 2025.3\bin\clion64.exe IDE ninja env.
#============================================================================#
#
# FUNCTION 	Plain vanilla Makefile for proj. SS (Hash Scatter Storage)
#
# SYSTEM 	Standard: ANSI/ISO C99 (1992); Ported to ISO/IEC C11 (2025).
#		Tested on PC/MS DOS 5.0 (MSC 600A), Windows 10 and WSL/UBUNTU.
#
# SEE ALSO 	SS.c SS.h
#
# PROGRAMMER 	Allan Dystrup.
# 		COPYRIGHT (c) Allan Dystrup, 1991, 2026.
#
# VERSION 	Revision 1.0 1992/03/20 13:40	 	Allan_Dystrup
#		Initial revision
#
# 		Revision 1.8 2025/12/09 11:00	 	Allan_Dystrup
# 		Port to Win.10 native/terminal,  Using CLion for Windows
#		[Using UBUNTU wsl.localhost/Ubuntu/usr/bin build tools
#                for: gcc (compile), ld (link) and make, using Makefile]
#		Windows Terminal (PowerShell 7 x64 Cmd Prompt):  
#		Microsoft Windows [Version 10.0.19045.6691]
#		(c) Microsoft Corporation. All rights reserved.
#
# WSL		----- Switch to Ubuntu for Linux build tools
#		C:\Users\allan>		wsl -d Ubuntu -u allan
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan$  cd CLionProjects/accessSU/SS
#
#		----- Clean and build default target
# MAKE		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU/SS$  make clean
#			rm -f SS.o
#			rm -f SS
#			rm -f file.hsh
#
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU/SS$  make /*default*/
#			gcc -DMAIN -DDEBUG -DRANDOM -g -c SS.c -o SS.o
#			gcc SS.o -Wall -lm -o SS
#
#		----- Test the build executable
#		allan@LAPTOP-6UIHQ2QE:/mnt/c/Users/allan/CLionProjects/accessSU/SS$ ./SS
#
#		---- HASH index creation -----
#		HashIndex Functions (Testdriver), Version 0.1.0
#		MOD[ss.c] VER[0.1.0 Pre] DAT[92/07/10] DEV[ad dec]
#		Copyright (c) Allan Dystrup 1992
#
#		 /* 2.1: Create & open the hash index (Read/Write mode) */
#		        11 prime!
#		MAXKEY2[11]
#		        13 prime!
#		MAXKEY2[13]
#		MAXKEY1[11]
#		Hashindex keyrecords : Size=[13], Used=[0]
#		Loadfactor=0
#		------------------------------------------------
#		Key[vpqzabjhKg]-(hash)->HashfileKeyRecord[3]
#					HashfileKeyOffset[166]
#		Key[ftyhattnqf]-(hash)->HashfileKeyRecord[0]
#					HashfileKeyOffset[50]
#		Key[zhkkydCEem]-(hash)->HashfileKeyRecord[10]
#					HashfileKeyOffset[454]
#			:
#			Key[gUgowpoygm]-(HASH)->HASHfileKeyRecord[3]
#
#				HASHfileKeyOffset[10
#				MODUL: Fil[HASH.c] - Linie[735] ; VERSION: Dato[Feb  5 2026] - Tid[09:41:44]
#				ID: [access-15-2002]	WARNING [A_XPAND].....:  expansion of index recommended
#				2]
#				HASHfileKeyOffset[252]
#			Key[Yqplvdxrbq]-(HASH)->HASHfileKeyRecord[6]
#				HASHfileKeyOffset[304]
#				:
#			Key[iaariKuzsf]-(hash)->HashfileKeyRecord[8]
#						HashfileKeyOffset[540]
# 			       29        29 prime!
#  			      31 prime!
#			MAXKEY2[31]
#			MAXKEY1[29]
#						HashfileKeyOffset[50]
#			Key[ftyhattnqf]-(hash)->HashfileKeyRecord[1]
#						HashfileKeyOffset[102]
#						HashfileKeyOffset[102]
#
#				:
#			Key[Mvihdzknkd]-(HASH)->HASHfileKeyRecord[0]
#				HASHfileKeyOffset[1176]
#				HASHfileKeyOffset[1230]
#				HASHfileKeyOffset[1262]
#				:
#		Opening SS-file[FILE.HSH], mode[RW] :
#			Read SS-header: dwIsize[31], dwIused[10], wKsize[10]
#		Hashindex keyrecords : Size=[31], Used=[10]
#		Loadfactor=33
#				:
#		Opening SS-file[FILE.HSH], mode[RW] :
#			Read SS-header: dwIsize[73], dwIused[24], wKsize[10]
#		Hashindex keyrecords : Size=[73], Used=[24]
#		Loadfactor=33
#				:
#		/*-------- 3: Find (pseudorandom) entries in the hash index -------------*/
#
#		Opening SS-file[FILE.HSH], mode[RO] :
#			Read SS-header: dwIsize[151], dwIused[100], wKsize[10]
#
#			Key[ienawklIox]-(hash)->HashfileKeyRecord[46]
#						HashfileKeyOffset[2324]
#			SUM OK : lookup=[1], probe=[1], probe/lookup=[1.00]
#			SUM ALL: lookup=[2], probe=[2], probe/lookup=[1.00]
#			Key[ienawklIox]-(lookup)->FlatfileOffset[0]
#
#			Key[nqawaxralu]-(hash)->HashfileKeyRecord[103]
#						HashfileKeyOffset[5236]
#			Key[nqawaxralu]-(hash)->HashfileKeyRecord[97]
#						HashfileKeyOffset[2474]
#						HashfileKeyOffset[7460]
#						HashfileKeyOffset[4716]
#			SUM OK : lookup=[1], probe=[1], probe/lookup=[1.00]
#			SUM ALL: lookup=[3], probe=[6], probe/lookup=[2.00]
#				:
#			Key[bbyzbrhcex]-(hash)->HashfileKeyRecord[144]
#						HashfileKeyOffset[7310]
#			Key[bbyzbrhcex]-(hash)->HashfileKeyRecord[108]
#						HashfileKeyOffset[5204]
#						HashfileKeyOffset[3100]
#						HashfileKeyOffset[774]
#						HashfileKeyOffset[6416]
#						HashfileKeyOffset[4312]
#						HashfileKeyOffset[2206]
#			SUM OK : lookup=[1], probe=[1], probe/lookup=[1.00]
#			SUM ALL: lookup=[13785], probe=[40433], probe/lookup=[2.93]
#
#				: etc.
#
#		----- HASH index LOOKUP -----
#
#		MODUL: Fil[HASH.c] - Linie[752] ; VERSION: Dato[Feb  5 2026] - Tid[10:59:23]
#		ID: [acceHASH-2- 801]     ERROR   [A_FILEEXIST].:  index allready exists
#		HASHindex keyrecords : Size=[151], Used=[100]
#			Loadfactor=66
#		Enter a key value 10 chars; [q] to quit-> lglEuztspK
#			Key[lglEuztspK]-(HASH)->HASHfileKeyRecord[148]
#                        			HASHfileKeyOffset[7460]
#			SUM OK : lookup=[2], probe=[2], probe/lookup=[1.00]
#			SUM ALL: lookup=[2], probe=[2], probe/lookup=[1.00]
#			Key[lglEuztspK]-(lookup)->FlatfileOffset[5]
#
#		Enter a key value 10 chars; [q] to quit-> harevjbafp
#			Key[harevjbafp]-(HASH)->HASHfileKeyRecord[82]
#                        			HASHfileKeyOffset[4174]
#			Key[harevjbafp]-(HASH)->HASHfileKeyRecord[95]
#                        			HASHfileKeyOffset[1346]
#                        			HASHfileKeyOffset[6246]
#			SUM OK : lookup=[3], probe=[5], probe/lookup=[1.67]
#			SUM ALL: lookup=[3], probe=[5], probe/lookup=[1.67]
#			Key[harevjbafp]-(lookup)->FlatfileOffset[69]
#
#		Enter a key value 10 chars; [q] to quit->q
#		Bye Bye...
#
#=============================================================================

# FILES ----------------------------------------------------------------------
HEADERS = SS.H 
OBJECTS = SS.o
TARGET  = SS
.PRECIOUS: $(TARGET) $(OBJECTS)

default: $(TARGET)

# COMPILE --------------------------------------------------------------------
CC = gcc
# EXE
# CFLAGS = -DMAIN -DDEBUG -g
CFLAGS = -DMAIN -DDEBUG -DRANDOM -g
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
	./$(TARGET)

log:
	./$(TARGET) 2>&1 >ss.log

# Source Control (GitHub)
git:
	

# CLEAN ---------------------------------------------------------------------
clean:
	-rm -f SS.o

realclean:
	-rm -f SS.o
	-rm -f SS
	-rm -f FILE.HSH

# END makefile
# ============================================================================