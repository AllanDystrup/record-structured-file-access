# record-structured-file-access
Simple, fast and flexible access method to record-structured and key-indexed datafiles.

`
/*+1========================================================================*/
/*   MODULE                       ACCESS.D                                  */
/*==========================================================================*/
/*   FUNCTION      Design documentation for project ACCESS.
 *
 *   SYSTEM        The design is system independent (portable).
 *                 The implementation will be targeted at Standard C,
 *                 on the PC/MS DOS >=v.3.0 and UNIX Sys V.3 platforms.
 *
 *   SEE ALSO      Modules : VA.x, SS.x, INDEX.x, KEY.x, where x in [H,C] 
 *
 *   PROGRAMMER    Allan Dystrup
 *
 *   COPYRIGHT     (c) Allan Dystrup, Feb. 1992, Feb. 2026
 *
 *   VERSION       $Header: d:/cwork/index/RCS/access.d 0.1 92/07/01 10:11:17
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 ----------------------------------------------------------
 *                 $Log:	access.d $
 *                 Revision 0.1  92/07/01  10:11:17  Allan_Dystrup
 *                 PreRelease (ALFA1)
 *
 *==========================================================================*/



/*============================================================================

   This file contains the design spec. for project "ACCESS".
  
   *THE AIM* of the project is to provide a simple, fast and flexible access
   method to record structured and key-indexed datafiles, thereby minimizing
   application memory requirement and dependency on commercial suppliers.

                             CONTENTS

   +--------------------------------------------------------------------+
   |                                                                    |
   |     GENERAL DESIGN .........: Layering and data abstraction        |
   |                                                                    |
   |     MODULAR DESIGN 1 .......: Module diagram                       |
   |        LEVEL 1 .............: Access Methods                       |
   |        LEVEL 2 .............: Index Modules                        |
   |        LEVEL 3 .............: Application Modules                  |
   |                                                                    |
   |     MODULAR DESIGN 2 .......: Interface Functions                  |
   |        LEVEL 1 .............: Access Methods                       |
   |        LEVEL 2 .............: Index Modules                        |
   |        LEVEL 3 .............: Application Modules                  |
   |                                                                    |
   |     PERFORMANCE ............: Overall program efficiency           |
   |        STATISTICS ..........: Timing of index generation & lookup  |
   |        CONCLUSIONS .........: Time & space tradeoff for VA/SS      |
   |        RECOMMENDATION ......: When to use VA resp. SS access       |
   |                                                                    |
   |     OPTIMIZATION ...........: Detailed function profiling          |
   |        STATISTICS ..........: Timing of single program functions   |
   |        CONCLUSIONS .........: Suggested targets of optimization    |
   |                                                                    |
   |     MAINTENANCE ............: Guidelines for repair & extension    |
   |        DESIGN ..............: Suggested design improvements        |
   |        TEST/DEBUGGING ......: Testdriver, stack/heap-check & trace |
   |        DOCUMENTATION .......: Module- & function-description       |
   |        PORTABILITY .........: Standard C, DOS/UN*X                 |
   |                                                                    |
   +--------------------------------------------------------------------+

   ============================================================================

                              GENERAL DESIGN

   The general design will be structured in three layers, using data-abtraction
   to hide implementation details of each layer :

      +----LEVEL 1-----+   The basic layer implementing a low-level strategy 
      | ACEESS METHODS |   for accessing a record-structured key-indexed file.
      |                |   The design will provide two fundamental strategies :
      |      VA        |    - Virtual Array   ("lookup" : data=index[key])
      |      SS (hash) |    - Scatter Storage ("hashing": data=index[f(key)])
      |      ...       |   The actual strategy will be hidden behind a general
      +-<general API>--+   API, and the design may easily be extended by new
              |            methods "plugged into" this generic access interface.
              |
      +----LEVEL 2-----+   A "system level" toolbox using the basic access
      |  INDEX TOOLS   |   methods to build functions for generating, main-
      |                |   taining and using indexfiles. We will provide :
      |     INDEX      |    - A stand-alone program to create an indexfile
      |      KEY       |    - Interface routines for cache'd and buffer'd
      |                |      dataaccess using the key entries of the index.
      +-<general API>--+      The routines wrap the data- and index-file into
              |               a simple database interface for applications.
              |  
      +----LEVEL 3-----+   This is where the user comes into the picture.
      |   APPLICATION  |   The user will typically :
      |    	       |
      | generate index |    - call the INDEX program to build an index file
      |    	       |
      |  lookup data   |    - call the KEY interface functions for a generic
      +----------------+      way to open, access and close the index'ed data.


   ============================================================================

                              MODULAR DESIGN 1
                              Module Diagram

   And now for a more detailed design of layers 1-2 of the project; -
   The first design spec. will show the structuring into separate modules
   and the interdependency between these modules (files).
   We will use the following characters to indicate :
      ---  inclusion of one source file into another
      ***  translation of one file format into another
      +++  interaction (read/write) of a program with a file
      -Dx  required compile-time switch <x> 
      [x]  required linking of module <x.O(BJ)>
   The flow of information in the diagram is from top to bottom.

   ............................................................................
   LEVEL 1
   Access Modules              +--- ACCESS.H --+
                               |               |
                               |               |	
                      +----- VA.H            SS.H -------+
                      |        |               |         |
                 +---------+   |               |     +---------+
                 |   VA.C  |   |               |     |   SS.C  |
                 | Virtual |   |               |     | Scatter |
                 |  Array  |   |               |     | Storage |
                 +---------+   |               |     +---------+
                      *        |               |          *
                      *        |               |          *
          ******** VA.O(BJ)    |               |       SS .O(BJ) ********      
   -DMAIN *                    |               |                        * -DMAIN
          *                    |               |                        *
      VA(.EXE)                 |               |                      SS(.EXE)
      TestDriver               |               |                     TestDriver
                               |               |
                               |               |
                          -DVA |               | -DSS
                            .......................
   Access API               :  Generic Acces API  :
   .........................:                     :............................
   LEVEL 2            +------- INDEX.H    KEY.H ---------+ 
   Index Modules      |     :..............|......:      | 
                      |                    |             |   
                      |                    |             |  
                    +---------+            |             |  
                    | INDEX.C |            |             |   
                    +---------+            |             |   
   data.file           *                   |             |  
   +   +               *                   |             |  
   +   +   ********* INDEX.O(BJ)           |             |  
   +   +   *    [VA.O(BJ) | SS.O(BJ)]      |             |
   +   v   *                               |             |      
   +  INDEX(.EXE)                          |             |  
   +  Program                              |             |
   +   +                                   |             |      
   +   +                                   |             |  
   +   v                                   |             |  
   +  index.file                           |             |
   +      +                                |             |  
   +      +                                |             |
   +      +++++++++++++++           +---------+          |
   +                    +           |  KEY.C  |          |
   ++++++++++++++++++   +           +---------+          |
                    +   +                *               |
                    +   +                *               |
                    +   +  ********* KEY.O(BJ)           |   
                    +   +  *   [VA.O(BJ) | SS.O(BJ)]     |
                    v   v  *                             |
                    KEY(.EXE)                            |   
                   TestDriver                            |
                                                         |
                                                         |
   Index API                                             |
   ......................................................|.....................
   LEVEL 3                                               |
   Application Modules                                   |
                                          APP.H ------+  | 
                                      Application     |  |
                                      specific Hdr    |  |
                                                   +---------+
                                                   |  APP.C  | 
                                                   +---------+ 
                                                         *
                                                         *
                                            *********  APP.O(BJ)
                                            *          [VA.O(BJ) | SS.O(BJ)]
                                            *          [KEY.O(BJ)]
                                            *
                                        APP(.EXE)               


   ============================================================================

                              MODULAR DESIGN 2
                              Interface Functions

   The other side of the coin for the modular design is specifying the 
   public interface functions of the modules.

   LEVEL 1
   THE AIM of level 1 (access modules) is to provide methods for easy and fast
   retrieval of data stored sequentially in file-records, each record identified
   by a unique key.

   The aim is attained by providing :
    - a program to traverse the datafile and build an indexfile mapping each
      record (identified by a keyvalue) to its corresponding datafile-offset
    - a set of functions for indirectly accessing the data-records by using
      their key-values to look up the record-positions in the indexfile.

   The strategy for structuring the indexfile may be chosen from several well
   proven techniques, depending on the need of the actual application program;
   We will provide :
    - Virtual Array index-file (VA) for very fast index-access (1 lookup/key).
      This strategy requires numeric key-values and preferrably a continous
      key-space (not well suited, if there are many "holes" in the key-range).
    - Scatter Storage index-file (SS) for fast index-access (2-3 lookup/key).
      This strategy will accept any characters in the keystring, and will 
      generally require less space on disk for the index-file.
   A natural extension would be to implement an index-sequential access method
   (ISAM) based on the B-tree technique. This strategy would be more general in
   combining properties from the VA and the SS methods, but it would probably
   perform significantly slower than both of these.

   For each strategy at level 1 (the access methods) we will thus have to
   define a set of basic functions to interact with :
    - an index-file (create, open, close) 
    - separate key-records in an index-file (insert, delete, find)
   These basic functions must be defined for both access methods : VA and
   SS, and the call-interface must be tailored, so it can be mapped directly
   to the syntax defined for the general access API (which is the generic
   interface used by LEVEL 2 : the Index Modules).
   Furthermore we may want to implement some misc. utility functions for the
   access methods (eg. index GetSize, GetLoad) and possibly some functions
   specific for each strategy.


   1.1                    1.2                   1.3
   GENERIC API	VIRTUAL ARRAY (VA)    		SCATTER STORAGE (SS)

   FILE LEVEL ................................................................

		/* .......... ACCESS: [VA|SS].H  DATA STRUCTS .......... */

   $(ACCESS)=[VA|SS]	 -DVA				-DSS
  
   # include:	"../va/va.h" 			"../ss/ss.h"
		
   Idx Size:	struct stVAsize {		struct stHsize {
    		   DWORD dwArSize;		   WORD  wKsize;  
   		   DWORD dwArUsed;		   DWORD dwIsize; 
   		   WORD  wArElSize;	 	   DWORD dwIsize; 
    		   char  cFill;			};
		};
		typedef struct stVAsize		typedef struct stHsize 
			SIZEINFO;			SIZEINFO;

   Idx runtime:	struct stVACore {		struct stHcore {	
    		    		    		   enum  { ICLOSED=0,
             	   	  				   IOPEN=1 
          	  		   		   } indexstatus;
    		   enum  { RW=0,		   enum  { RW=0,
    		   	   RO=1	    	           	   RO=1 
		    }  indexmode;		   } indexmode;
		   FILE *pfArFile;		   FILE *fd; 
    		 				   char *filename;
    		   SIZEINFO stSize; 		   SIZEINFO indexsize; 
    		   WORD wBfSize; 		};
    		   WORD wBfElSize; 
    		   char *pcBf; 
    		   char *pcArElInit; 
 		};
 		typedef struct stVACore 	typedef struct stHcore 
			*VACB;				*HASH;

		/* .......... GENERIC API map to ACCESS: [VA|SS].H .......... */

		typedef VACB ITYPE;		typedef HASH ITYPE;

   (*peIdxCreate)(ITYPE*,char*,WORD,DWORD)  (*peIdxCreate) (ITYPE*,char*,WORD,DWORD)
	=eVAIdxCreate;				=eHashIdxCreate;
   (*peIdxOpen) (ITYPE*,char*,char*)	    (*peIdxOpen) (ITYPE* char*,char*)
   	=eVAIdxOpen;		       		=eHashIdxOpen; 
   (*peIdxClose)(ITYPE*)		    (*peIdxClose) (ITYPE *)
    	=eVAIdxClose;				=eHashIdxClose;			
   (*peIdxRead) (ITYPE*,char*,DWORD*)	    (*peIdxRead) (ITYPE*,char*,DWORD*)
   	=eVAKeyFind;				=eHashKeyFind;
   (*peKeyInsert)(ITYPE*,char*,DWORD)	    (*peKeyInsert)(ITYPE*, char*, DWORD)
   	=eVAKeyInsert;				=eHashKeyInsert;
   (*peKeyDelete) (ITYPE*,char*)	    (*peKeyDelete)(ITYPE*,char *)
   	=eVAKeyDelete;				=eHashKeyDelete;
   (*peKeyFind) (ITYPE*,char*,DWORD *)	    (*peKeyFind) (ITYPE*,char*,DWORD*)
 	=eVAKeyFind);				=eHashKeyFind;
   (*peIdxGetSize) (ITYPE*,DWORD*,DWORD*)   (*peIdxGetSize)(ITYPE*,DWORD*,DWORD*)
   	=eVAIdxGetSize;				=eHashIdxGetSize;
   (*peIdxGetLoad) (ITYPE*,WORD*)	    (*peIdxGetLoad)(ITYPE*,WORD*)
   	=eVAIdxGetLoad;			 	=eHashIdxGetLoad;
					    (*peIdxResize)(ITYPE*,int)
					 	=eHashIdxResize;					
       					    (*peIdxProcess (ITYPE*, WORD (*)(char*,DWORD))
   						=eHashIdxProcess;    

   (*peIdxCreate) (       eVAIdxCreate (        eHashIdxCreate (
      void  *pI,             VACB  *ppVA,          HASH  *pH,
      char  *pzIdxFile,      char  pzIdxFile,      char  *pzIdxFile,
      WORD  wSize,           WORD  wElSize,        WORD  wKsize,
      DWORD dwSize           DWORD dwDummy         DWORD dwISize
      )                      )                     )  

   (*peIdxOpen) (         eVAIdxOpen (          eHashIdxOpen (
      void  *pI,             VACB  *ppVA,          HASH  *pH,
      char  *pzIdxFile,      char  pzIdxFile,      char  *pzIdxFile,
      char  pzIdxMode        char  pzAccess        char  *pzAccess
      )                      )                     )

   (*peIdxClose) (        eVAIdxClose (         eHashIdxClose (
      void  *pI              VACB  *ppVA           HASH  *pH
      )                      )                     )

   RECORD LEVEL .............................................................
                 
   (*peKeyInsert) (       eVAKeyInsert (        eHashKeyInsert (
      void  *pI,             VACB  *ppVA,          HASH  *pH,
      char  *pzKey,          char  *pzKey,         char  *pzKey,
      DWORD dwOffset         DWORD dwDatOffset     DWORD dwDatOffset
      )                      )                     )

   (*peKeyDelete) (       eVAKeyDelete (        eHashKeyDelete (
      void  *pI,             VACB  *ppVA,          HASH  *pH,
      char  *pzKey           char  *pzKey          char  *pzKey
      )                      )                     )

   (*peKeyFind) (         eVAKeyFind (           eHashKeyFind (
      void  *pI,             VACB  *ppVA,           HASH  *pH,
      char  *pzKey,          char  *pzKey,          char  *pzKey,
      DWORD dwOffset         DWORD dwDatOffset      DWORD dwDatOffset
      )                      )                      )

   MISCELLANEOUS .............................................................

   (*peIdxGetSize) (      eVAIdxGetSize (        eHashIdxGetSize (
      void  *pI,             VACB  *ppVA,           HASH  *pH,
      DWORD *pdwSize,        DWORD *pdwSize,        DWORD *pdwSize,
      DWORD *pdwUsed         DWORD *pdwUsed         DWORD *pdwUsed
      )                      )                     )  

   (*peIdxGetLoad) (      eVAIdxGetLoad (        eHashIdxGetLoad (
      void  *pI,             VACB  *ppVA,           HASH  *pH,
      WORD *pwLoad           WORD  *pwLoad,         WORD  *pwLoad
      )                      )                     )

                                                 eHashIdxResize (
                                                    HASH  *pH
                                                    )

                                                 eHashIdxProcess (
                                                    HASH  *pH,
                                                    WORD (*ufunc) (
                                                       char  *pzKey,
                                                       DWORD *dwDatOffset
                                                       )
                                                    )

   ...........................................................................


   LEVEL 2
   Level 2 (Index Modules) is built "on top of" the Level 1 functions,
   using the GENERIC ACCESS API to obtain an implementation-independent
   method of accessing record structured files; The actual access strategy
   is choosen at compile time by linking in one of the implemented access
   modules (as indicated by a compiler switch, - at the time of writing :
   -DVA for VirtualArray or -DSS for hashed ScatterStorage).

   Level 2 applies the basic access method to implement a "high level" toolbox
   of functions for working with index- and datafiles :
    - creating an indexfile from a record-structured datafile
      (functions contained in module : INDEX.C )
    - fast access to datafile records through the indexfile, using a cache
      for datarecord file-offsets and a buffer for retrieving datarecords.
      (functions defined in module : KEY.C)
                                     
   Level 2 offers a data-abstract program interface (API) to index- and
   datafile-operations for use by application programs: The actual implemention
   of the operations (ie. access-strategy, indexcache & databuffering method)
   is "hidden" from the user by the set of public datastructures and highlevel
   interface functions presented in the Level 2 headers index.h and key.h.

   2.1                              2.2
   INDEX.C                          KEY.C
   ...........................................................................

                                    typedef struct stDataBase {
                                       FILE   *fdData;          // DATA  HANDLE
                                       ITYPE  pIndex;           // INDEX HANDLE
                                       struct stKeyCache {      // INDEX CACHE
                                          DWORD  (*padwData)[]; // Cache array
                                          DWORD  dwCsize;       // Max. entries
                                          DWORD  dwCused;       // Used entries
                                          DWORD  dwCbwin[2];    // Buf. window
                                       };
                                    } DBASE;

                                    typedef struct stDataBuff { // BUFFER
                                       char  *pzBaddr;          // Buffer area
                                       WORD  wBsize;            // Buffer size
                                    } BUFFER;


   eIdxMake (                       eKeyDBOpen (			
      ITYPE  *hINDEX,                  DBASE   *pstDBx,
      char   *pzDatFile,               char    *pzDatFile,
      WORD   wKeyMark,                 char    *pzIdxFile,
      WORD   wKeySize,                 BUFFER  *pstBF,
      char   *pzIdxFile,               char    *pzDatBuf,
      DWORD  dwIdxSize                 int     iBufLen
   )                                )

   eIdxTest (                       eKeyDBRead (	
      ITYPE  *hINDEX,                  DBASE   *pstDBx,
      char   *pzDatFile,               BUFFER  *pstBF,
      WORD   wKeyMark                  long    lSetPos,
   )                                   long    lSetSiz
                                    )

                                    eKeyDBClose (
                                       DBASE   *pstDBx,
                                       BUFFER  *pstBF
                                    )

   ...........................................................................


   LEVEL 3
   Level 3 (Application Modules) call on the services of Level 2 to retrieve
   data stored on disk in flat/sequential files of variable-length records,
   each record identified by a unique key-value in 1. position :

    - First the application must generate an indexfile for each datafile to
      access; - the application may call the index API (eIdxMake) directly, or
      the user may choose to call/spawn the program index.exe with appropriate
      command line parameters (cf. module documentation in index.c).
      The indexfile must be updated, whenever changes are made to the datafile.
      The design assumes relatively static datafiles, so user-programs are
      expected to always regenerate indexfiles "in extenso" using the index.c
      API. Dynamic "incremental" updating of indexfiles is however possible,
      by directly calling the access-method through the generic access API, -
      eg. (*peKeyInsert), (*peKeyDelete) etc.

    - Then the application will go through the following sequence of operations
      (cf. module documentation in key.c for a detailed explanation of syntax
      and semantics of the API; - eKeyDBRead has many options for resetting,
      scrolling and searching the index-cache!)

            // Allocate structures for database(s) and buffer(s)
            DBASE  stDB1;           // 1. database (indexfile, datafile and cache)
            BUFFER stBF;            // Common buffer for databases (stDB1, stDB2...)
            const int BUFLEN=512;   // Length of buffes area
            char cBuf[BUFLEN];      // Buffer area

            // Open database(s), instantiate resources.
            eKeyDBOpen(&stDB1, "datfile.1", "idxfile.1", &stBFx, cBuf, BUFLEN);

      LOOP  // Access & treat data (compute, transform, display or whatever)
      +---> strcpy(stBFx.pzBaddr, "20200-20299,20400,20500"); // Set up key-list
      |     eKeyDBRead(&stDB1, &stBF, K_LIST, 0);             // Initialize cache
      | +-> eKeyDBRead(&stDB1, &stBF, 1, 10);                 // Read to buffer
      | |   ...                                               // (scroll cache)
      +-+-- MyDisplayFunc(&stBF);     // User's own function for displaying data

            // Close database(s), release resources.
            eKeyDBClose(&stDB1, &stBF);


 			    MODULE STRUCTURE 
 
                             Data model
    To define a database object in memory we need a descriptor datastructure
    for connecting the data- & key-fileptr's with a cache array of datafile
    offsets. The buffer for datarecords is defined as a separate structure,
    so the same buffer may be reused by several "databases".
 
    (1) Database descriptor :
 
                            RAM (incore)
                            struct stDataBase DBASE
                            +------------------+
                  +---------- CACHE   stCache  | cache datastructure
                  |         | FILE    *fpData  | data file pointer
                  |         | ITYPE   pIndex  -----------+ handle for key 
                  |         +------------------+         | 
                  |                                      | Compile switch : 
      RAM (incore)                                       | -DVA: ITYPE= VACB
      struct stKeyCache CACHE                            | -DSS: ITYPE= HASH
      +-------------------+ Ptr to array of offsets      | 
      | DWORD (*padwData)[]--->[ malloc'ed area ]        |
      |                   |                              |
      | DWORD  dwCsize    | max. entries in array        |             
      | DWORD  dwCused    | used entries in array        |          
      | DWORD  dwCbwin[2] | range of array in buffer     |                  
      +-------------------+                              |
                                                         |
                                                RAM (incore)
                                                struct ... ITYPE 
                                                +---------------------+
                             open mode [RO|RW]  | enum      keymode   |
                             key file pointer   | FILE      *fpIndex  | 
                                      +---------- SIZEINFO  stSize    |
                                      |         |<Cf. va.h and ss.h>  |
                                      |         +---------------------+
                          RAM (incore)
                          struct ... SIZEINFO
                          +---------------------+
                          |<Cf. va.h and ss.h>  |
                          +---------------------+
 
  
    (2) Data buffer:
                            RAM (incore)
                            struct stDataBuffer BUFFER
                            +----------------+
                            | char  *pzBaddr | Ptr to buffer area 
                            | WORD  wBsize   | Size of buffer area
                            +----------------+  
 
  
                            Function decomposition
    The key-module functions to access these datastructures may be grouped
    into two levels and three seperate categories :
    ( 1 ) API functions providing an external interface for calling modules
    (2.1) Functions operating internally on the CACHE datastructure,
    (2.2) Functions operating internally on the BUFFER datastructure.
    Thus the low-level CACHE & BUFFER functions (level 2) handle the direct
    access to files and "incore" descriptor structures, but these internal
    functions are wrapped in a high-level API (level 1) offering the basic
    operations of opening, reading and closing an abstract "database" object.
    An application (level 0) will call on the KEY API for data access, and
    typically introduce further functionality for user interaction (ie. data
    selection and buffer display).
 
    The main calling hierachy is outlined in the following diagram, where
    the signatures -> and <- indicate input resp. output parameters, and
    the signature (__) represents repetition.
 
    .........................................................................
    LEVEL 0
    APPLICATION
    (example)                  main  
                                 |
              +------------------+-------------------+
              |                  |                   |
              |             eKeyDBAccess             |
              |                  |                   |
              |            eKeyListScroll            |
              |             |    |    |              |
              |            (___________)             |
              |             |    |   |               |
              |        getchar   |  eKeyBufDump      |
              |                  |                   |
    ..........|..................|...................|.......................
    LEVEL 1   |                  |                   |
    KEY API   |                  |                   |
              |                  |                   |
       eKeyDBOpen            eKeyDBRead          eKeyDBClose
                               /     \
                              |       |
       -> [K_LIST | K_EXPR]   |       |  -> (initialized cache)
       <- (initialized cache) |       |  <- (filled buffer)
                           __/\__     |___________________
                           | \/ |                        |
                    K_EXPR |    | K_LIST                 |
    .......................|....|........................|...................
    LEVEL 2                |    |                        | 
    KEY LOW-LEVEL          |    |                        | 
                           |    |                        | 
           +<--------------+    |                        |
           |                    |                        |
           |   +<---------------+                        |
           |   |                                         |
           |   |  (2.1) CACHE              (2.2) BUFFER  |
           |   |                                         |
           |   |  ->(keylist)                            |
           |   |  <-(cache)                              |
           |   +->eKeyCacheFill------------>eKeyBufScan  |
           |                |                            |
           |   +------------+                            |
           |   |                                         |
           |   +->eKeyCacheFree                          |
           |   |                                         |
           |   +->eKeyCacheAlloc                         |
           |                                        eKeyBufFill
           |
           |      ->(cache,pattern)
           |      <-(new cache)
           +----->eKeyCacheSearch --------->eKeyBufRead
                            |                          
    ........................|................................................
    LEVEL 3                 |
    SEARCH SERVICE          |
                            |               GENERIC SEARCH              
                            |
                            +-------------->(*pvBldSearch)()
                                            (*pfRunSearch)()
                                            (*pvDelSearch)()
                                            ----------------
                                                   |
                                            SPECIFIC SEARCH
                                         one of several methods
                                           cf. search modules
 
   ===========================================================================
                             PERFORMANCE

   The following data is intended to help in chosing the components of the
   ACCESS program package most suited for your particular task.
   (The data is obtained from the DOS environment using small memory model,
   but the conclusions based on relative program sizes and performance are
   immediately transferable to UN*X).                 

   STATISTICS
   +=========================================================================+
   | Program     |        Program Size (KB)          | Relative |  Comment   |
   | Module      | OBJ:-driver | OBJ:+driver |  EXE  | Perform. |            |
   +=============+=============+=============+=======+==========+============+
   | va          |      5      |     17      |   29  | //////// | Virtual Ar.|
   | hash        |     10      |     31      |   62  | //////// | Scatter St.|
   +-------------+-------------+-------------+-------+----------+------------+
   | index[VA]   | /////////// |      8.5    |   20  |     1    | Index-     |
   | index[SS]   | /////////// |      9      |   41  |     5    | generation |
   +-------------+-------------+-------------+-------+----------+------------+
   | key[VA]     |      8.5    |     17.5    |   50  |     1    | Index-     |
   | key[SS]     |      8.5    |     17.5    |   53  |    33    | lookup     |
   +=================================================+==========+============+
   | Rel. size of generated index, using index[VA]   |     4    | Index-file |
   |                                -"-  index[SS]   |     1    | size       |
   +=========================================================================+

   CONCLUSIONS
   Based on the table above we may now draw the following conclusions :

   1: a VA-indexfile takes up approx. 4x the diskspace of an equivalent
      SS-indexfile; the VA-index must contain an entry for each and every
      value in the keyspace, while a SS-index will "pack" the valid key-
      values into a smaller indexrange. On the other hand each hashindex
      keyrecord must contain the keyvalue besides the datafile-offset, while
      the latter suffices for VA-records (which are stores sequentially).
      The net effect of these tradeoffs is a smaller size of the SS-index,
      and this consideration (conservation of disk space) is the main moti-
      vation for preferring the SS access method.

   2: generation of an index-file from a data-file i 5x faster using the
      VA-accessmethod than using SS-access. Furthermore lookup-operations
      on a VA-index are significantly faster than hash'ed access, due to :
      (a) local buffering is possible (and implemented) in the VA-method,
          thus speeding up sequential access to the index-records
      (b) possibility of collisions in the SS-access: in a VA-index there
          is a 1:1 correspondence between any keyvalue and its indexfile
          record-position, while hashing causes a certain clustering which
          requires collision resolution involving reprobing a chain of index-
          records. The average length of the hash reprobing chains and the
          probability of looking up non-existing keys (no-hit) will determine
          the actual performance characteristic of the SS lookup-operation.
      In the example above, the index was extremely sparse (4% of the key-
      range defined), and the lookup-statistics was gathered from sequential
      lookup over the whole key-range. This explains the poor performance of
      the SS-method as compared to the VA-lookup: 33x slower access time.
      This value must however be regarded as an extreme; In a more general
      case (say 50% of the key-range defined) I would expect a performance
      ratio of VA/SS of approx. 1:10. Anyway the conclusion will stand :
      the VA-access method is significantly faster than HASH'ed (SS) access.
      This is the main motivation for using VA-access.


   RECOMMENDATION
   Use the VA method if you require ultra-fast response, frequent sequential
   index-access, and if you can predict a low "hit-ratio" on lookup-operations.
   
   Use the SS method if disk-space is of primary concern or if key-values are
   non-numeric.

   Implement a new access-method (eg. a B-Tree ISAM) if you have special demands
   not met by the currently offered access-methods. Due to the generic access API,
   it should be easy to integrate a new method as an alternative to VA and SS,
   while preserving the functionality of the high-level modules INDEX and KEY.



                             OPTIMIZATION
                                 
   To provide a guideline for potential program optimization, I list some
   profiling statistics gathered from a typical run of key.c (w/testdriver).
   (Again the data is obtained from the DOS environment / small memory model, 
   but the conclusions based on relative data (ie.%) are tansferable to UN*X).                 

   STATISTICS
   Profile, - Function counting, sorted by counts :
   +----------------------------------------------------------------------+
   |      Hit     |                                                       |
   | count |  %   | Function    (Module)                                  |
   +-------+------+-------------------------------------------------------+
   | 11021 | 32.2 | eKeyBufScan (key.c:1606)                              |
   | 11018 | 32.2 | eVAKeyFind  (va.c:679)                                |
   | 10698 | 31.3 | pvVAread    (va.c:1156)                               |
   |   ... |  ... | ...          ...                                      |
   +-------+------+-------------------------------------------------------+

   Profile, - Function timing, sorted by time :
   +-------------------+-------------------+------------------------------+
   |           Func    |       Func+Child  |      Hit                     |
   |   Time     |  %   |  Time      |  %   | count  Function     (Module) |
   +------------+------+------------+------+------------------------------+
   | 287205.151 | 42.2 | 373216.858 | 54.9 |     3 eKeyListScroll (key.c) |
   | 127721.661 | 18.8 | 127721.661 | 18.8 | 11123 eKeyBufScan    (key.c) |
   | 122915.573 | 18.1 | 310857.946 | 45.7 |     4 eKeyCacheFill  (key.c) |
   | 41562.379  |  6.1 | 41562.379  | 6.1  |  238  eKeyBufFill    (key.c) |
   |    ...     |  ... |    ...     | ...  |  ...  ...             ...    |                                              |
   +------------+------+------------+------+------------------------------+

   CONCLUSIONS
   Function eKeyListScroll resides in the testdriver portion of the key.c
   module, and is thus relatively uninteresting in this context; The principal
   code to target in order to improve program performance is the key.c functions: 
   eKeyBufScan, eKeyCacheFill and eKeyBufFill.

   The function eKeyBufScan may be considerably simplified, if you don't 
   require the advanced key-expansion facilities (overkill ?).
   The functions eKeyCacheFill and eKeyBufFill access the index-file resp.
   data-file on disk, so disk-I/O is responsible for their rather high time-
   consumption. The performance of eKeyBufFill may be improved by using low
   level file access functions, eg. read() instead of fread(); This strategy
   however is not recommendable, since it isn't ANSI-compliant and therefore
   will affect the portability.


   ========================================================================


                             MAINTENANCE

   DESIGN (RESTRUCTURING)
   The design of the access program package may be improved in several areas,
   notably :

   1: Error handling
      Errorchecking & -processing is currently implemented in the headerfiles
      (access.h etc.) as a MACRO facility. This strategy yields maximal run-
      time efficiency at the expense of some codesize overhead. A cleaner and
      more flexible design would be to collect all error-handling for the
      project files into ONE file with a functional interface (possibly with the
      errormessages residing in a separate and modyfiable textfile).

   2: LIB vs. OBJ
      The functions of the ACCESS project are grouped into separately compiled
      units to be linked into an application program. An alternative would be
      to offer the functions (esp. in the access-modules VA & SS) in the form
      of libraries. This will in general provide a more "fine grained" inclusion
      mechanism and thus a smaller size of the application program ; - I doubt
      though, that it will be worth the effort for this specific project.


   TEST/DEBUGGING

   1: Testdriver
      Each program module (ie. compilation unit) contains a testdriver, that
      may be switched ON/OFF via the project makefile (compile switch -DMAIN). 
      The testdriver provides a basic test of the module functionality, and
      typically allows for user interaction in specifying the test conditions
      (ie. data input and program control flow).

   2: Consistency-Check
      To aid in error-trapping at runtime, I have implemented two small utility
      programs for transparent consistency-checking of the heap (hpck.c/h) resp.
      stack (stck.c/h). These runtime-checkers may be switched ON/OFF at program
      generation via the project makefile: switch -DDEBUG. (The hpck-program 
      fulfills essentially the same purpose as KMDALLOC, but was developped before
      that utility was generally available.)

   3: Trace/Debug
      For the purpose of assuring correct program functionality and to help
      hunting down logical errors at runtime, I have incorporated numerous
      explicit debugging statements (printf-trace of selected variables),
      which may be optionally compiled into the program, again using switch
      -DDEBUG in the makefile. Activate this facility whenever you modify or
      extend the program modules.


   DOCUMENTATION
   The ACCESS project has been fully documented at the module and functional
   level : In each major program module (ie. compilation unit) you will find
   a full description of basic concepts, design decisions, module structure
   (data model & functional decomposition) et. al. Furthermore each function
   is preceeded by a standard header describing parameters, algorithm and return
   value(s). The documentation may be extracted from a module at one of several
   levels, using the small documentation utility "ex.awk" (which relies on the
   AWK-program).


   PORTABILITY
   All modules have been coded in STANDARD C (ANSI X3.158-1989/ISO-IEC 9899)
   and comply with the X/Open Posix.1 spec. (IEE 1003.1 and ISO/IEEE 1003.1).
   The project makefile and a master include-file with appropriate preprocessor-
   definitions (general.h) provide for portability between the DOS and UN*X
   environments and for flexibility in choosing ANSI or "old" K&R style.

   The primary development and target-platform of the modules has been PC/MS
   DOS 5.0 using tools from MicroSoft (MS C 6.00A compiler & MC LINK 5.10 linker),
   MKS (MAKE 2.5 & RCS 5.1) and GIMPEL (PClint 4.00).
   The modules have been moved (FTP'd) to a UN*X environment (DG AViiON) and
   compiled/linted here without problems. They should pass any ANSI-compliant
   compiler with flying colors, but porting to K&R is not guaranteed (deferred
   to a UN*X port of the modules).


   ========================================================================


                             THAT's ALL

   The simple ACCESS modules should be directly usable, and easily maintainable
   given the available documentation. The user should refer to the PUBLIC func-
   tion descriptions; The maintainer should first study the makefile, general.h
   and the module level design spec's and header-files. Have fun!

   Allan Dystrup, Ballerup June 1992.

 *
 *
 *-1 END OF FILE ACCESS.D ==================================================*/
`
