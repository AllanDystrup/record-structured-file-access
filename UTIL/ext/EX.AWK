# "(c) Copyright KommuneData I/S 1993"
# "%Z% PRJ=%Q% MOD=%M%[%Y%] SID=%I% CI=%E%-%U% %Z%"
# "@(#)$Id: ex.awk 1.2 1993/05/11 09:05:44 Allan_Dystrup Exp $"
#=+1=========================================================================#
#  MODULE                      EX.AWK                                        #
#============================================================================#
#  FUNCTION    AWK program for extracting documentation from program modules.
#
#              Program-lines for extraction must be enclosed in selection
#              statements (comments) according to the following syntax :
#                  LINE POS.:  12345
#                              ..+#
#                              ..-#
#                  WHERE:      .    is any character in the alphabet
#                              + -  turns-ON / turns-OFF extraction at level #
#                              #    is a digit [0-9] indicating extraction level
#                  EXAMPLE:	12345
#                              /*-2 This line turns ON extraction at level 2*/
#                              This line will be extracted at level 2
#                              And this line and any other lines in this block
#                              /*-2 This line turns OFF extraction at level 2*/
#
#              The actual extraction levels to use is read from the file EX.DAT
#              as a sequence of separate lines, eg the following example speci-
#              fies extraction at level 2 and 3 :
#                              2 
#                              3
#  			
#  SYSTEM      MKS AWK on PC/MSDOS
#              STD AWK on UN*X 
#              (Set OS = [MSDOS | UNIX] in program section "BEGIN" below)
#
#  PROGRAMMER  Allan Dystrup
#
#  COPYRIGHT   (c) Allan Dystrup, Kommunedata I/S, August 1991.
#
#  VERSION     $Header: d:/cwork/index/RCS/ex.awk 1.1 92/02/17 12:03:26
#              Allan_Dystrup Exp Locker: Allan_Dystrup $
#              $Log: ex.awk $
#              ----------------------------------------------------------------
#              TRIGGER   OBJECTIVE    PROGRAM-FIX                  FUNCTION
#              ----------------------------------------------------------------
#              Revision 1.2  1993/05/11  09:05:44  Allan_Dystrup
#              PREREL-UX (ALFA1)
#              BCAD      Portability  Repl. EXON/EXOFF with regexp EXTRACT-ON/OFF
#                                     Introduced OS & MSDOS/UNIX   BEGIN
#              ----------------------------------------------------------------
#              Revision 1.1    92/06/12  12:48:14  Allan_Dystrup
#              PREREL-SU (ALFA1)
#              BCAD                   Initial revision
#              ----------------------------------------------------------------
#
#  USAGE       awk  -f  ex.awk  <program.fil>
#              with actual extraction levels specified in file : <ex.dat>
#
#  BUGS        This version is a quick and clean hack to do the basic job; -
#              I have had no time to add bells and whistles to the script.
#=-1=========================================================================#



#=+2 MODULE EX.AWK ==========================================================#
#    NAME                       BEGIN                                        #
#=== SYNOPSIS ===============================================================#
#    1. Setup for choice of operating system     [MSDOS/MKS | UNIX | ...]
#    2. Define manifest constants                [DEBUG-switch, EXFILE-config.] 
#    3. Initialize arrays for extraction state   [EXLEVEL, INLEVEL]
#    4. Print header page for extracted doc.
#=-2  
BEGIN       {
               # (1) Setup for actual Operating System :
               MSDOS  = 0;          #  - PC/MSDOS
               UNIX   = 1;          #  - UNIX
               OS     = UNIX;       #  Choose ONE 


               # (2) Define manifest constants :
               DEBUG  = "OFF";      # ["ON"|"OFF"] to turn-on/suppress debugging
               EXFILE = "ex.dat";   # File containing actual levels to extract


               # (3) Initialize Extract-level and Inside-level arrays :
               #  - EXLEVEL[i] : 1 if extract on this level, cf. file <ex.dat>
               #  - INLEVEL[i] : 1 if we are currently inside this extr.-level

               # 3.1 FIRST Clear level arrays
               for (i = 0; i <= 9; i++)
                  EXLEVEL[i] = INLEVEL[i] = 0;

               # 3.2 Then read Extract-levels specified by user from file EXFILE
               while ( (getline < EXFILE) > 0)
                  if ( 0 <= $1 &&$1 <= 9) {
                     EXLEVEL[$1] = 1;
                     DUMP("EXLEVEL", EXLEVEL);
                  }


               # (4) Print header page :
               printf("\n\n\n\n\n");
               system("rm -f ex.hd");
               if (OS == MSDOS) 
                  system("dato" "> ex.hd");
               if (OS == UNIX) 
                  system("date" "> ex.hd");
		  # Path to crc depende on project directory structure...
		  system("./util/crc/crc " ARGV[1] ">> ex.hd");
		  system("../crc/crc " ARGV[1] ">> ex.hd");
		  system("../../crc/crc " ARGV[1] ">> ex.hd");
		  system("cat ex.hd");

               printf("\n\n\n\n\n");
               if (OS == MSDOS) {            	# MKS utilities
                  system("banner " "-c # DOC");
                  system("banner " "-c = ===");
               }
               if (OS == UNIX) {		# sudo apt install sysvbanner
                  system("banner " "DOC");
                  system("banner " "===");
               }
               printf("\n\n\n\n\n");
               system("banner " ARGV[1]);

               printf("\n\n\n\n\n\f");

            } # BEGIN



#=+2 MODULE EX.AWK ==========================================================#
#    NAME                      EXTRACT-ON                                    #
#=== SYNOPSIS ===============================================================#
#	IFF inputline ($0) contains ExtractOn (ie. $0 ~ EXON) 
#       Turn ON extraction at indicated Inside-level
#=-2
$0 ~ /^..\+[0-9].*$/	{
               INLEVEL[substr($0, 4, 1) + 0] = 1;
               DUMP("INLEVEL", INLEVEL);
            } # EXON



#=+2 MODULE EX.AWK ==========================================================#
#    NAME                      EXTRACT-OFF                                   #
#=== SYNOPSIS ===============================================================#
#   	IFF inputline ($0) contains ExtractOff (ie. $0 ~ EXOFF) 
#       Turn OFF extraction at indicated Inside-level
#=-2
$0 ~ /^..\-[0-9].*$/	{
               INLEVEL[substr($0, 4, 1)+0] = 0;
               DUMP("INLEVEL", INLEVEL);
               if (PRTFLG) {
                 printf("%s\f", $0);
		 system("cat ex.hd");
                 printf("\n", $0);
               }
            } # EXOFF


#=+2 MODULE EX.AWK ==========================================================#
#    NAME                      MAIN                                          #
#=== SYNOPSIS ===============================================================#
#    For all inputlines ($0) from stdin :
#       IFF (for all levels i : IF INLEVEL[i] THEN EXLEVEL[i])
#          Print the inputline
#       ELSE
#          Ignore (continue with next line)   
#=-2
           {  PRTFLG = 0;

               for (i=0; i <= 9; i++)
                 if (INLEVEL[i])
                    if(EXLEVEL[i])
                       PRTFLG = 1;
                    else {
                       PRTFLG = 0;
                       break;
                    }
                 
               if (PRTFLG && !($0 ~ /^..\-[0-9].*$/))  	# not EXOFF
                  print $0;
            } # ALL



#=+3 MODULE EX.AWK ==========================================================#
#    NAME                       DUMP                                         #
#=== SYNOPSIS ===============================================================#
#    IFF (DEBUG-switch set)
#       Dump string S (header-text) and array A[] (level-info) on stdout
#=-3
function   DUMP(S, A) {
               if (DEBUG == "ON") {
                  printf("\t%s: [ ", S);
                  for (i = 0; i <= 9; i++)
                     if (A[i])
                        printf("%d ", i);
                  printf(" ]\n");
               }
           }
           


#=+2 MODULE EX.AWK ==========================================================#
#    NAME                      END                                           #
#=== SYNOPSIS ===============================================================#
#    At end of file :
#       print "END-EXTRACTION" and exit.
#=-2    
END        {   printf("\n\n\n\n\nEND EXTRACTION\n\f\n");
               printf("\007");
           }




# END module EX.AWK														  #
#============================================================================#
