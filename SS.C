/*+1========================================================================*/
/* MODULE                         HASH.C                                    */
/*==========================================================================*/
/* FUNCTION    Toolbox for building diskbased hash-index'es.
 *             This module contains a collection of basic routines for :
 *             creating, accessing and maintaining index'es for fast access
 *             to simple datafiles.
 *             A datafile must be record-structured (arbitrary record size)
 *             with each record identified by a unique key-value (a fixed
 *             length string of arbitrary ASCII chars).
 *
 * SYSTEM      Standard Ansi C.
 *             Tested on UNIX V.3 (DG) and PC/MS DOS V.3.3.
 *
 * SEE ALSO    Modules:
 *             GENERAL.H, ACCESS.H, KEY.H for macros & errorhandling
 *             INDEX.C/H ................ for building index'es from datafiles.
 *             KEY.C/H .................. for accessing datarec's via index'es.
 *
 * PROGRAMMER  Allan Dystrup
 *
 * COPYRIGHT   (c) Allan Dystrup, October 1991.
 *
 * VERSION     $Header: d:/cwork/index/RCS/hash.c 0.1 92/07/06 14:28:10
 *             Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *             ----------------------------------------------------------
 *             $Log:	hash.c $
 *             Revision 0.1  92/07/06  14:28:10  Allan_Dystrup
 *             PREREL (ALFA1)
 *
 * REFERENCES  Knuth, Donald E. [1980,1973] : "The art of computer programing"
 *                 Vol.2 : "Seminumerical Algortihms"
 *                 Vol.3 : "Sorting and searching"
 *                 Addison-Wesley Publishing Company.
 *             Floyd, E.T. [1987] : "Hashing for high-performance searching",
 *                 Dr. Dobb's Journal, Feb. 1987
 *             Lum, V.Y. et.al. [1971] : "Key-to-address transform techniques:
 *                 A fundamental performance study on large existing formatted
 *                 files", CACM April 1971, Vol.14, No.4.
 *             Lum, V.Y. [1973] : "General performance analysis of key-to-
 *                 address transformation methods using an abstract file
 *                 concept", CACM October 1973, Vol.16 No.10.
 *             Maurer, W.D. & Lewis, T.G. [1975] : "Hash table methods",
 *                 Comp. Surveys, March 1975, Vol.7, No.1.
 *             Park, Stephen K. & Miller, Keith W. (1988) :
 *                 "Random number gererators: good ones are hard to find"
 *                 Communications of the ACM
 *                 October 1988, Volume 31 Number 10 (pp. 1192-1201)
 *
 * USAGE       Module hash.c features the following public routines for
 *             building and working with indexfiles; - See headerfile hash.h
 *             and the function documentation for a detailed description of
 *             the user accesible datastructures and interface functions.
 *                 eHashIdxCreate()        // Create a new indexfile
 *                 eHashIdxOpen()          // Open an existing indexfile
 *                 eHashIdxClose()         // Close an open indexfile
 *
 *                 eHashKeyInsert()        // Insert new (key,offset) in index
 *                 eHashKeyDelete()        // Delete (key,offset) from index
 *                 eHashKeyFind()          // Look up (key,offset) in index
 *
 *                 eHashIdxResize()        // Resize open indexfile
 *                 eHashIdxGetLoad()       // Get loadfactor for open indexfile
 *                 eHashIdxGetSize()       // Get size for open indexfile
 *                 eHashIdxProcess()       // Process all records in indexfile
 *
 * DOC         Documentation is incorporated into the module and may be
 *             selectively extracted (using a utility such as ex.awk) :
 *                Level 1: Module documentation (history, design, testdriver)
 *                Level 2: PUBLIC functions (module program interface, "API")
 *                Level 3: major PRIVATE functions (design)
 *                Level 4: minor PRIVATE functions (support)
 *
 * BUGS        The module is coded in STANDARD C (ANSI/ISO), which is a
 *             feature! -  It is however prepared to compile under "old"
 *             K&R C (if you prefer bugs ...)
 *
 *
 * =============================== SEARCHING =================================
 *
 *
 * A fundamental operation intrinsic to a great many computational tasks is
 * SEARCHING : retrieving some particular information from a large amount of
 * previously stored data. Normally we think of the information as divided
 * into smaller BLOCKs, each block having a KEY for use in searching.
 *
 * The task may be formulated directly as "find the index of the BLOCK
 * identified by the KEY K in an array of BLOCKs", or it may be camouflaged
 * in different ways, for instance as as a request to "retrieve the RECORD in
 * a file corresponding to the record-KEY K".
 *
 * No matter how camouflaged, the task eventually boils down to finding a
 * solution to the general searching problem, for which we have several well
 * known algortihms, cf. Knuth[1977].
 *
 *
 *                         Sequential search.
 * A simple "brute force" method where we simply scan a table of keys from
 * one end, until we "hit" K (or exhaust the table). Only suitable when the
 * number of keys is small (< approx. 20).
 * Worst case.... : n, where n = the table size
 * Average case.. : (n+1)/2
 * Example,n=1000 : W=1000, A=500
 *
 *                         Binary search.
 * This method requires an ORDERED table (sorted numerically or alphabetically).
 * We now recursively look at the middle of the table, each time discarding
 * the upper/lower half of the table depending on the value of K (less/greater
 * than the middle entry). The algorithm requires more bookkeeping, but
 * guaranties a low search time in both the worst and average case (in fact
 * binary search is provably the best possible algorithm for a COMPARISON
 * based search).
 * Worst case.... : k, where k = smallest integer such that 2**k > n
 * Average case.. : k -[(2**k -k-1)/n] = k-1 for large values of n
 * Example,n=1000 : W=10, A=9
 *
 *                         Binary tree search.
 * The binary search algorithm implicitly defines a tree structure (on the
 * stack) during the recursive activation of the search procedure.
 * This tree structure can be defined explicitly by incorporating a directed
 * binary graph (where each node has 2 pointers) in the table of keys.
 * The setuptime of the binary tree is larger than for the simple key table,
 * but the search is faster since arithmetic calculation is eliminated.
 * Furthermore the tree may be arranged in an "optimal" way to reflect the
 * expected frequency of searches for each key (placing the most frequent
 * keys near the root of the tree).
 * The basic performance characteristics of Binary tree search are identical
 * to those stated for Binary search, - searching  an "optimal" Binary tree
 * however may perform significantly better (depending on the "skewness" of
 * the searching frequency distribution of the keys).
 *
 *
 *                         Balanced tree search.
 * The performance measures of the Binary search algorithms described above
 * rely on the assumption that the search tree (implicit or explicit) is
 * well balanced, ie. all branches on the same level in the tree have
 * approximately the same number of descendants.
 * This assumption can not always be fulfilled : in the case of a dynamic
 * data store with many insertions and/or deletions a balanced tree quickly
 * degrades to a skewed datastructure (a "heap") with a bad worst-case
 * performance.
 *
 * The solution to this problem is to "re-balance" the tree whenever skewness
 * emerges, and several algorithms have been devised for this purpose :
 * the general idea is to allow more than 2 pointers per node in the tree
 * to get extra flexibility for the pointer reassignments of the rebalancing
 * operation. Some well known algorithms are AVL-trees, 2-3-4 trees and
 * the most general method known as B-trees (M keys per node).
 * This search strategy leads directly to the concept of index sequential
 * access method ("ISAM" files) and more complex hierachial databases.
 *
 *
 *                         Hashing
 * Binary search methods suffer from two general defects that limit their
 * usability in most simple applications :
 *    (1) the keyword entries have to be ordered (numerically/alphabetically)
 *    (2) the ordering implies restructuring of the data in case of updating
          by addition/deletion
 *
 * The search methods known as "hashing" (or scatter storage technique)
 * solves these two defects, - and usually perform better as well !
 * Hashing methods are based on the computers ability to do arithmetic at high
 * speeds. The idea is to treat the letters of words as if they were numbers
 * and then to "hash" (ie. scramble) the numbers in some way to generate a
 * single number for each word. In this way each key "maps" to a table address
 * (index) where the key has been stored, - if it is an element of the table!
 *
 *
 * =============================== HASHING ===================================
 *
 *
 * A hash function allows DIRECT REFERENCING of datablocks by using a simple
 * arithmetic transformation on the keys into table addresses. The hash table
 * may store the DATArecords directly, or use "indirect addressing" (ie. the
 * hashrecords store : the key + a POINTER to the datafile record).
 * The goal of hashing is to define a function, that will :
 *  - generate a FULL period (a mapping spanning the whole index range),
 *  - generate a UNIFORM mapping, ie. an even probability of mapping over the
 *    whole index range. The hash function should thus approximate a "random"
 *    function : it should perform as a fast (pseudo-) randomnumber generator
 *    mapping a large set of key-values into a smaller set of key-record
 *    index'es. Ideally the mapping should have the property that : h(K1) !=
 *    h(K2), so that different words K1 and K2 will hash directly to different
 *    addresses (so called "perfect hashing").
 *  - generate a UNIQUE mapping, ie. even keys hashing to the same initial
 *    index-value should trace different probing sequences during rehashing
 *  - perform FAST and be relatively easy to compute.
 *
 *                         Perfect hashing
 * If we know the key-values in advance, it is always possible to define a
 * SPECIALLY TAILORED "minimal perfect" (MP) hash function that will locate
 * any desired table element in a SINGLE probe without requiring extra space
 * in the table. A MP hash function uses a transformation based on a statis-
 * tical analysis of the actual set of key values :
 *  - (1) Digit analysis
 *    The distribution of values of the keys in each position or digit is
 *    determined. The positions with the most skewed distributions are then
 *    deleted from the key until the number of remaining digits is equal to
 *    the address length.
 *  - (2) Algebraic coding
 *    Each digit of a key is considered to be a polynomial coefficient. The
 *    polynomial so obtained is divided by another polynomial, which is
 *    invariant for all the keys. The coefficients of the remainder poly-
 *    nomial form the address. Based on the theory of error-correcting codes
 *    ("Hamming distances"), the dividing polynomial may be chosen to yield
 *    a UNIFORM mapping.
 *
 * MP hash functions are mostly of theoretical interest. In practice they are
 * used in situations, where we have a relatively short, static list of keys
 * (for instance reserved word lists of programming languages). Even in these
 * special cases, one often uses simpler methods such as :
 *  - setting up a table of carefully selected CODES derived from empirical
 *    tests of transforming the actual key-characters to unique index-values
 *    (as opposed to a complete statistical analysis)
 *  - using fast and simple arithmetic operations to obtain a "nearly minimal
 *    perfect" hash function with a single probe and a limited amount of extra
 *    /unused table space (as opposed to complex polynomial computations).
 *
 *
 *                         General hashing
 * Perfect hashing is seldom possible for longer lists of keywords : unless
 * the table size (m) is much larger than the number of keys (n), nearly all
 * GENERAL hash functions will lead to at least a few "collisions" between
 * values of h(Ki) and h(Kj).
 *
 *
 *                         Hash functions
 * General Hash functions must "SCRAMBLE" the key to obtain a more UNIFORM
 * mapping and "FOLD" the key-values onto the (smaller) set of index-values.
 * To obtain these goals, two basic functions have been used (with several
 * variations) : division and multiplication; - In practice one often uses a
 * combination of these methods :
 *
 *    (1) Division method
 *        H(key) = key MOD table-size
 *        where table-size must be prime.
 *         - Radix conversion
 *           Express the key in radix p, use : p MOD q**m as index.
 *           p and q must be relatively prime, and m a positive integer.
 *         - Folding and compression
 *           Divide ("fold") the key into smaller components - often separate
 *           characters - , and combine ("compress") these into a final index
 *           value, - for instance by ADD or XOR operations.
 *
 *    (2) Multiplication method
 *        H(key) = TRUNC( table-size * ((key * R) MOD 1) )
 *        where R : [0...1],  and ((key*R) MOD 1) = remainder of (key*R)
 *         - Mid-Square
 *           Multiply the key by itself (square), and extract the middle bits
 *           as index value.
 *         - Shifting
 *           A simple fold/compress-function does not take into account any
 *           permutation of key components (the order of characters). To
 *           counteract this kind of redundancy in the keyword set, we may use
 *           bit-shifting (=multiplication) of PARTS of the keyword before
 *           folding/compression.
 *
 *
 *                         Collision resolution.
 * Collisions are usually handled in one of two ways :
 *
 *    (1)  Indirect addressing ("chaining")
 *         Each entry of the table is treated as the first link in a chain of
 *         entries, all having keys that hash to the same value.
 *
 *    (2)  Open addressing ("rehashing")
 *         In case of collision we generate another hash code from the same
 *         key, and then reprobe the table (until key found/or empty entry) :
 *         REHASH(key, i) =  ( HASH(key) + F(i) ) MOD table-size.
 *         The reprobing term F(i) can be implemented in various ways to
 *         obtain (some of) these goals :
 *            (2.1) Linear probing
 *                  F(i) = a CONSTANT value added to the hash code each time.
 *                  To generate a FULL period either F(i) and table-size
 *                  should be "co-prime" (have no common factor), or the
 *                  table-size should be prime. Linear probing with F(i) = 1
 *                  degrades to a simple sequential search.
 *                  Linear probing is simple, but often produces "primary
 *                  clustering" (several sequences of collisions are stored
 *                  in the same range of hashindex locations).
 *            (2.2) Quadratic probing
 *                  F(i) = the EXPRESSION : (a + b*i + c * i**2), where a,b,c
 *                  are constants. This quadratic sequence yields more separated
 *                  chains of probes for keys hashing to nearby locations; -
 *                  however keys hashing to exactly the same location will
 *                  still trace the same probing sequence (known as "secondary
 *                  clustering"). Quadratic probing requires a table of size
 *                  of 2**m, - it won't search all locations if the table-size
 *                  is prime!
 *            (2.3) Double hashing
 *                  F(i) = the FUNCTION : (key MOD P2) + 1
 *                  By letting F(i) depend on the "key", we can effectively
 *                  reduce secondary clustering. The idea is to chose TWO
 *                  hash functions :
 *                     HASH(key) = key MOD tablesize -> [0...tablesize]
 *                     F(i) = 1+(key MOD (tablesize-2)) -> [1...tablesize-1]
 *                     REHASH(key) = (HASH(key) + F(i)) MOD tablesize
 *                  For this scheme to work, we must choose two "TWIN-PRIMES"
 *                  tablesize and tablesize-2 (example : 1021 and 1019).
 *            (2.4) Random probing
 *                  F(i) = a PSEUDO-RANDOM number generator producing a full,
 *                  uniform and unique mapping.
 *                  This is the ultimate solution to the collision problem,
 *                  but parameters for a well suited pseudo-random number
 *                  generator are not as easy to calculate, as those for a
 *                  good double hashing function. Furthermore the performance
 *                  of double hashing closely approximates random probing, so
 *                  solution 2.3 is often preferred for 2.4
 *
 *
 * ============================== DESIGN =====================================
 *
 *                     Primary design decisions
 * The primary goal of module hash.c has been to provide a set of SIMPLE and
 * yet GENERAL functions for building and using indexfiles as a means of fast
 * access to "flat" datafiles (arbitrary length records identified by uniqueue
 * keys) :
 *    (1)  SCATTER STORAGE indexing ("hashing") is preferred to the binary
 *         search techniques, thereby gaining speed in the general case of
 *         random access and index updating, at the cost of somewhat slower
 *         performance when sequential access is required.
 *
 *    (2)  GENERAL HASHING - as oposed to "perfect hashing" - is nessecitated
 *         by the dynamic characteristic of the datafiles (and as a conse-
 *         quence : our inability to determine the key-values in advance).
 *
 *    (3)  SEVERAL HASHING TRANSFORMATIONS & COLLISSION RESOLUTION ALGORITHMS
 *         have been provided in order to enable the user to select a method
 *         suited to the particular key-characteristic and datafile-size.
 *         Furthermore the module has been designed in a way that makes it
 *         very easy to "plug in" new algorithms for hashing and/or collision
 *         resolution, should the need to do so arise.
 *
 *
 *                     Choice of hash algorithms
 * Guidelines for the selection of a proper hashing strategy is stated below:
 *
 * (1) Hashing transformation.
 * GENERAL hash transformations must all be "distribution independent", ie.
 * the transformations can not take into account the actual distribution of
 * keys over the key space (since the datafiles are not known in advance!).
 * In many/most cases the key-distributions are NOT uniform, resulting in
 * more collisions in "dense" areas of the key-space, - regardless of which
 * hashing transformation we use!
 * Practical studies as well as theoretical analysis thus indicate that -
 * faced with an arbitrary key set - the choice of hashing transformation is
 * of less importance than a good collision resolution method.  Only one
 * general conclusion can be made [Lum 71/73]: A hash-transformation based
 * on division gives the best overall performance. All of the following
 * functions include a division : MOD by the (prime) size of the indextable:
 * +-------------------------+----------------------------------------------+
 * | METHOD:                 |            Characteristic                    |
 * +-------------------------+----------------------------------------------+
 * | FLL (First+Last+Length) | fast, NOT suited for fixed-length keys       |
 * | ADD (Sum of chars)      | fast, NOT good if many key-permutations      |
 * | PJW (P.J.Weinburger)    | slower (approx. 10 x ADD), more general      |
 * +-------------------------+----------------------------------------------+
 * The time needed to calculate the hash function is not important in the
 * context of this module, where the index'es are kept on secondary storage
 * (disk access time is a factor 10-100 x slower than a hash calculation).
 * We therefore choose P.J.WEINBURGER (PJW) as default hashing transformation.
 *
 * (2) Collision resolution :
 * Let n=number of keys, m=number of slots in index, L="Loadfactor"=n/m <= 1.
 * The number of probes required to look up a key in a hash index is then a
 * function of the Loadfactor, as stated in the following table :
 * +-------------+--------------------------------------+-------------------+
 * |      METRIC:|              # Probes                |  Examples: A W    |
 * | METHOD:     | Average (A)     | Worstcase (W)      | L=0.5   | L=0.75  |
 * +-------------+-----------------+--------------------+---------+---------+
 * | Chaining    | 1 + L/2         | e**(-L) + L        | 1.3 1.4 | 1.1 1.2 |
 * | Linear Prob.| [1 + 1/(1-L)]/2 | [1 + 1/(1-L)**2]/2 | 1.5 2.5 | 2.5 8.5 |
 * | Double Hash.| -[ln(1-L)] / L  | 1 / (1-L)          | 1.4 1.8 | 2.0 4.0 |
 * +-------------+-----------------+--------------------+---------+---------+
 * In general hashing provides VERY FAST CONSTANT search times for L < 0.75,
 * ie. when space is available for a large enough table; - Contrast this to
 * linear search with O=n and binary search with O=log2(n). Note however that
 * the performance degrades rapidly, as the index fills up (L approaches 1).
 * When this happens, the index should be resized to obtain a lower L factor.
 *
 * Chain-based lookup compares favorably to the other collision resolution
 * strategies, but implies maintenance of linked lists; - this method is well
 * suited for RAM-based index'es (dynamic storage allocation) with a high data
 * "turnover rate" and an unpredictable number of keys and updates. It is
 * however inconvenient for more static index'es placed on secondary storage.
 * (Chaining is NOT offered as a collision resolution method in this module).
 *
 * Linear probing is simple to compute, and the method may be preferable for
 * small index'es with a low loadfactor. For larger index'es the overhead
 * imposed by double hashing is justified by a reduced collision rate.
 * We make DOUBLE HASHING the default choice for collision resolution.
 *
 *
 *                     Multiuser considerations
 * The file access methods for the hash functions in this module have been
 * designed with the following goals in mind :
 *  1  SINGLE USER exclusive access to multiple dynamic hash index'es
 *     with full (READ/WRITE) permissions.
 *     This option may be used by a system administrator in a multiuser
 *     environment to create and maintain hash index'es for subsequent
 *     sharing by multiple end-users (cf. pt. 2)
 *  2  MULTIUSER concurrent access to multiple static hash index'es
 *     with limited (READ-ONLY) permissions.
 *
 * TRUE MULTIUSER access with full permissions on hash index'es has NOT
 * been implemented;  This requires some changes to the design :
 *  -  the code in the basic hash index write-functions (cf. eWriteIdxHdr,
 *     eWriteIdxKey) must be "sandwich'ed" between statements for locking
 *     resp. unlocking the range of bytes to be updated in the file (ie.
 *     setting up "critical regions" for the basic record-write operations).
 *     Nb: lock/unlock-functions have not been standardized by ANSI/ISO,
 *     so a generic interface should be implemented to hide system dependency.
 *  -  the simple method of checking a "boolean" integrity value (indicating
 *     proper close of the index) must be substituted by a multivalued
 *     integrity variable : a count of open-operations on the index-file.
 *     Updating of the index (cf. eHashKeyInsert, eHashKeyDelete) may then
 *     be allowed - accompanied by appending of datarecords to the datafile.
 *     Nb: any restructuring of the datafile or of the hash index (cf.
 *     eHashIdxResize) should not take place while users are "on line".
 *
 *
 * ============================== MODULE STRUCTURE ===============================
 *
 *                     Data model
 * To define a hashtable on disk and in core we need some datastructures for
 * size- & statusinfo (one HEADER-record) and for index-values (multiple
 * KEY-records). These data must be maintained both on disk and in RAM.
 * The appropriate data-structures are defined in the include-file HASH.H.
 *                             
 * (1) Header-record layout
 *
 *     DISK (external)                     RAM (incore)
 *     struct stHdisk                      struct stHcore (*HASH)
 *     +--------------------+              +----------------------+
 *     | long     integrity | closed OK?   | enum     indexstatus | [ICLOSED,IOPEN]
 *     | int      checksum  | of SIZEINFO  | enum     indexmode   | [RW,RO]
 *     | SIZEINFO indexsize |              | FILE     *fd         | 
 *     +------------  |  ---+              | char     *filename   |
 *                    |                    | SIZEINFO indexsize   |
 *                    |                    +------------  |  -----+
 *                    |                                   |
 *                    +---> struct stHsize (SIZEINFO) <---+
 *                            +---------+
 *                            | wKsize  | size (#byte) of key
 *                            | dwIsize | size (#records) of index file
 *                            | dwIused | usage (#USED records) in index file
 *                            +---------+
 *
 * (2) Key-record layout
 *
 *     DISK (external)                 RAM (incore)
 *     key record                      struct stHkey (KEY)
 *     +-------------------+           +-------------------+
 *     | int               |           | enum  status      | [VACANT,USED,DELETED]
 *     | char[wKsize]      |           | char  *key      --->char[wKsize]
 *     | DWORD             |           | DWORD dwDatOffset |
 *     +-------------------+           +-------------------+
 *
 *
 *
 *                     Function decomposition
 * The index functions to generate and access these data structures may be
 * grouped into 3 broad categories : Header operations, Key-record operations
 * and low level index-file I/O. The main calling hierachy is outlined in the
 * following diagram :
 * 
 * (1) Index header operations     (2) Index key operations
 *
 *  +->eHashIdxCreate ---------+       eHashKeyInsert --------------------+
 *  |   | eHashIdxOpen ------+ |        | eHashKeyDelete ---------+       |
 *  |   | eHashIdxClose----+ | |        | eHashKeyFind -------+   |       |
 *  |   |                  | | |        |                     |   |       |
 *  |  eHashIdxRestore     | | |        |                  eLocateKey     |
 *  +--eHashIdxResize ---+ | | |        |                     |   |       |
 *                       | | | |        |                     |   |       |
 *     eHashIdxGetSize   | | | |        +----> dwHashFunc <---+   |       |
 *     eHashIdxGetLoad   | | | |        +----> dwRehashFunc <-+   |       |
 *     eHashIdxProcess <-+ | | |                                  |       |   
 *                         | | |                                  |       |  
 *                         | | |                                  |       |  
 *                         | | |   (3)                            |       |  
 *                         | | |   Index low-level I/O            |       |
 *                         | | |                                  |       |
 *                         | +-|-> eReadIdxHdr                    |       |
 *                         +-->+-> eWriteIdxHdr                   |       |
 *                             |                                  |       |
 *                             |   eReadIdxKey <------------------+<------+
 *                             +-> eWriteIdxKey <-------------------------+
 *
 *
 *-1========================================================================*/


/*==========================================================================*/
/*                                Includes                                  */
/*==========================================================================*/

/* Stacdard C (ANSI/ISO) headerfiles */
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
                                                                          
/* #define S/H-DEBUG: runtime check of stack- and heap on DOS  */
/* Relies on PC/MS DOS V.3.3 system files which are deprecated */
//#include "../check/stck/stck.h"
   # define STCK(x)
   # define REAL_MAIN	main
//#include "../check/hpck/hpck.h"

/* Project headerfile */
#define _HASH_ALLOC
#include "SS.H"


/*==========================================================================*/
/*                           Defines & Macros                               */
/*==========================================================================*/

/* FILE_OFFSET calculates the offset into the file according to  */
/* the element# (nelem) and the keysize (wKsize)                 */
#define FILE_OFFSET(nelem,wKsize)               \
        ( sizeof(struct stHdisk)                \
          + nelem * ( sizeof( struct stHkey )   \
          - sizeof( char * ) + wKsize )         \
        )


/*==========================================================================*/
/*                           Function prototypes                            */
/*==========================================================================*/

/*-------------------------- Main Testdriver -------------------------------*/
PRIVATE   void
          vSigCatch P((int iSigNum));

PRIVATE   void
          vKeyGet   P((WORD wKeySize, char *pzKeyBuf));

PRIVATE   WORD
          wKeyPrint P((char *pzKeyBuf, DWORD dwDatOff));

PRIVATE   eRetType
          eIdxStatPrint P((HASH *pH));


/*-------------------------- Hashing functions -----------------------------*/
PRIVATE   DWORD
          dwHashFunc P((char *pcKeystr, WORD wKsize, DWORD dwMaxSlot));

PRIVATE   DWORD
          dwRehashFunc P((char *pcKeystr, WORD wKsize, DWORD dwCurSlot, DWORD dwMaxSlot));

#ifdef DEBUG
PRIVATE   void
          vHashTrace P((char *pcKeystr, WORD wKsize, DWORD dwCurSlot));
#endif	/* DEBUG */


/*-------------------------- Hashindex Header Record -----------------------*/
PRIVATE   eRetType
          eHashIdxRestore P((HASH * pH));

PRIVATE   void
          vUpdate_stHdisk P((HASH H, struct stHdisk * pstIdxHdr, eAnsType eIntegr));


/*-------------------------- Hashindex Keyrecords --------------------------*/
PRIVATE   eRetType
          eLocateKey P((HASH H, char *key, DWORD * pdwKeySlot, DWORD * dwDatOffset));


/*-------------------------- Hashindex Reorganization ----------------------*/
PRIVATE   WORD
          wCopy2New P((char *key, DWORD dwDatOffset));


/*-------------------------- Hashindex Low Level I/O  ----------------------*/
PRIVATE   eRetType
          eWriteIdxHdr P((HASH H, struct stHdisk * pstIdxHdr));

PRIVATE   eRetType
          eReadIdxHdr P((HASH H, struct stHdisk * pstIdxHdr));

PRIVATE   eRetType
          eWriteIdxKey P((HASH H, DWORD dwSlot, struct stHkey * f));

PRIVATE   eRetType
          eReadIdxKey P((HASH H, DWORD dwSlot, struct stHkey * f));


/*-------------------------- Utility Functions -----------------------------*/
PRIVATE   void
          vGetPrime P((DWORD *pdwPrime, int iUpward));

PRIVATE   FLAG
          fIsPrime P((DWORD n));

PRIVATE   double
          rRandom   P((VOID));

PRIVATE   WORD
          wCCITTcrc P((char *pzData, WORD wLength));

PRIVATE   char *
          pzStrcpyAlloc P((char **ppzDest, char *pzSrc));




#ifdef MAIN
/****************************************************************************/
/******************************** MAIN **************************************/
/****************************************************************************/

/* Define "sigon message" for module hash.c */
PRIVATE char SIGNON[] =
	"\nHashIndex Functions (Testdriver), Version 0.1.0\n"
	"MOD[ss.c] VER[0.1.0 Pre] DAT[92/07/10] DEV[ad dec]\n"
	"Copyright (c) Allan Dystrup 1992\n\n";


/* Define initial values for hashindex */
#define   Ksize   10           /* Keysize */
#define   HIsize  10L          /* Initial size of hash index (# key-records) */
HASH      HI   =  NULL;        /* Handle to "In core" index descr.struct. */
char      pzKBuf[Ksize + 1];   /* Buffer for entering key value */


/*+1 MODULE HASH.C =========================================================*/
/*   NAME   01               REAL_MAIN()                                    */
/*== SYNOPSIS ==============================================================*/
int
REAL_MAIN(argc, argv, envp)
    int       argc;            /* Argument count */
    char     *argv[];          /* Argument vector */
    char     *envp[];          /* Environment pointer */
{
/* DESCRIPTION
 *    Testdriver for module hash.c; - exercises the functions in the module
 *    and validates the functionality through trace-statements when compiled
 *    with flag "DEBUG".
 *
 *   1: Print signon message & setup to catch "break" signals.
 *
 *   2: MAKE and fill a new hash index ... :
 *      2.1: Create & open the new hash index (Read/Write mode), -
 *           actions 2.x are skipped, if the indexfile already exists.
 *      2.2: Print initial statistics for the index (size & loadfactor).
 *      2.3: Fill the hashindex file with: (key,offset) records,
 *           build by scanning the datafile.
 *      2.4: Print final statistics for the index (size & loadfactor).
 *      2.5: Print all (defined) keyrecord entries in the indexfile.
 *      2.6: Close the hash index gracefully, - NB: the indexfile MUST be
 *           closed properly to preserve the index integrity in RW mode!
 *
 *   3: TEST the hash index by looking up entries in the index file ... :
 *      3.1: Open the existing hash index (ReadOnly mode).
 *     [3.2: Optionally print all (defined) keyrecord entries,
 *           - this function may take some time.]
 *      3.3: LOOP : Enter keyvalue, lookup in hashindex file, print keyrecord
 *           UNTILL empty keyvalue entered
 *      3.4: Close the hash index gracefully
 *
 * RETURN
 *   REAL_MAIN() is a testdriver and not intented to interface with any
 *   calling program. The return value from REAL_MAIN() is thus insignificant
 *   in this context.
 *   You should however notice the checking of error-conditions on return
 *   from each call to a PUBLIC function defined in hash.c; - This practice
 *   should also be followed in your application to "catch" and diagnose any
 *   malfunction in the system or in the services provided by this module.
 *   (You will probably want to write your own error-handling, though).
 *
 * EXAMPLE
 *   The contents of function REAL_MAIN() demonstrates the basic aspects of
 *   building and accessing a hashindex using the *LOW LEVEL* public data-
 *   structures and interface-functions in HASH.C/H.
 *   The modules INDEX.C/H and KEY.C/H provides more *HIGL LEVEL* interfaces
 *   for generating and using indexfiles from application programs.
 *
 * SEE ALSO
 *   hash.h for a detailed description of symbolic constants, macro's,
 *   data structures, return codes and error codes.
 *-1*/

    char      cIdxFile[] = "FILE.HSH"; 	/* Name of indexfile to create & test */
    WORD      wLoad      =  0;         	/* Indexfile load factor */
    DWORD     dwDatOff   = 0L;         	/* Datafile offset */
    DWORD     dwCount    = 0L;         	/* Scratch count variable */
    int       iMaxTry    = 100;		/* Max key lookups */ 


    /*----------------- 1: Signon and setup signal catcher -----------------*/

    /* Signon */
    fputs(SIGNON, stdout);


    /* Setup to redirect interrupt signals to our own handler */
    signal(SIGINT, vSigCatch);
    signal(SIGTERM, vSigCatch);


    /*----------------- 2: Make & fill a new hash index --------------------*/

    /* 2.1: Create & open the hash index (Read/Write mode) */
    if (eHashIdxCreate(&HI, cIdxFile, Ksize, HIsize) != ERROR) {

	/* 2.2: Print initial statistics */
	ACHK_ERR(eIdxStatPrint(&HI), A_CONT);

	/* 2.3: Fill the hashindex file w. (key,offset) pairs from datafile */
	for (dwCount = 1L; dwCount <= (HIsize * 10); dwCount++) {
	    /* Enter new keyrecord in hashindex file */
	    vKeyGet(Ksize, pzKBuf);
	    ACHK_ERR(eHashKeyInsert(&HI, pzKBuf, dwCount), A_CONT);

	    /* Expand hashindex file (&	print new statistics) if nessecary */
	    ACHK_ERR(eHashIdxGetLoad(&HI, &wLoad), A_CONT);
	    if (Astat == A_XPAND) {
		   eHashIdxResize(&HI, 200);
		   ACHK_ERR(eIdxStatPrint(&HI), A_CONT);
	    }
	}

	/* 2.4: Print final statistics */
	ACHK_ERR(eIdxStatPrint(&HI), A_CONT);

	/* 2.5: Print all valid (USED) keyrecord entries */
	ACHK_ERR(eHashIdxProcess(&HI, wKeyPrint), A_CONT);

	/* 2.6: Close hashindex gracefully, -NB: file integrity in RW mode! */
	ACHK_ERR(eHashIdxClose(&HI), A_CONT);
    }
    else
	ACHK_ERR(;, A_CONT);


    /*----------------- 3: Find entries in the hash index ------------------*/

    /* 3.1: Open the existing hash index (ReadOnly mode) */
    ACHK_ERR(eHashIdxOpen(&HI, "FILE.HSH", "rb"), A_CONT);

    /* 3.2: Uncomment to print all valid keyrecord entries, - optional */
    /* ACHK_ERR(eIdxStatPrint(&HI), A_CONT); */

    /* 3.3: LOOP enter keyvalue, lookup in hashindex file, print keyrecord */
    #ifdef RANDOM
    	iMaxTry = 10;	
    #endif
    do {
		vKeyGet(Ksize, pzKBuf);
		ACHK_ERR(eHashKeyFind(&HI, pzKBuf, &dwDatOff), A_CONT);
		wKeyPrint(pzKBuf, dwDatOff);
    } while (*pzKBuf != '\0' && iMaxTry-- >0); /* until empty keyvalue string */

    /* 3.4: Close the hash index gracefully */
    ACHK_ERR(eHashIdxClose(&HI), A_CONT);


    /* Return status to caller , - not used for this driver program! */
    return (Astat = A_OK ? 0 : 1);

}   /* END function REAL_MAIN() */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   01.01                 vSigCatch                                 */
/*-- SYNOPSIS --------------------------------------------------------------*/
#define   MAXLINE   81

PRIVATE void
vSigCatch(iSigNum)
    int       iSigNum;
{
/* DESCRIPTION
 *    Support function for REAL_MAIN() test driver.
 *    Signal handler set up to catch the "break" signals : SIGINT (asynch.
 *    interactive attention) & SIGTERM (asynch. interactive termination).
 *    1: Prompt user for break confirmation; We have two possible situations:
 *       a: break during index generation (INTEGRITY="UNKNOWN"), - will leave
 *          the index in a not fully generated state, possibly unusable.
 *          Issue warning if this situation !
 *       b: break during index test (INTEGRITY="OK"), - no problem!
 *    2: Depending on user confirmation : [Y]->terminate or [N]->continue
 *
 * RETURN
 *    If break confirmed: program terminated with exit code 'EXIT_FAILURE'
 *    else: signal 'iSigNum' reset and program execution resumed.
 *
 * BUGS
 *    Asynch. signals don't guarantee access to volatile data at sequence pts;
 *    Since we restrict our access to READ operations, this shouldn't pose any
 *    problem, - though not strictly ANSI (cf. type sig_atomic_t).
 *-4*/
    char      pzLine[MAXLINE];         /* Line buffer */

    /* 1: Prompt user for break confirmation; - issue warning if situation 1 ! */
    fprintf(stdout, "\nINTERRUPT:\tSignal [%d] received\n", iSigNum);
    fprintf(stdout, "\tCurrent state of indexfile [%s] : INTEGRITY [%s]\n",
	    HI->filename, (HI->indexmode == RW ? "UNKNOWN" : "OK"));
    fprintf(stdout, (HI->indexmode == RW ? "\tIndex not complete!, -" : "\t"));
    fprintf(stdout, "Sig %d, Abort program? [Y|N] => ", iSigNum);


    /* 2: Depending on user answer : terminate [Y] or continue [N] */
    (void) fgets(pzLine, MAXLINE, stdin);
    if (strchr(pzLine,'y') != NULL || strchr(pzLine, 'Y') != NULL) {
	/* 2.1: Terminate program "gracefully" */
	eHashIdxClose(&HI);
	exit(EXIT_FAILURE);
    }
    else
	/* 2.2: Continue : reset signal, and continue */
	signal(iSigNum, vSigCatch);

}   /* END function vSigCatch() */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   02                  vKeyGet                                     */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vKeyGet(wKeySize, pzKeyBuf)
    WORD      wKeySize;	    /* Size of key (#chars) to generate/read */
    char     *pzKeyBuf;	    /* Buffer for placing key value */
{
/* DESCRIPTION
 *    Support function for REAL_MAIN() test driver.
 *    Get a key value of "wKeySize" characters into the buffer "pzKeyBuf"; -
 *    pzKeyBuf must be at least wKeySize+1 byte wide.
 *    Depending on the compilation flag "RANDOM", the function will :
 *    1: [RANDOM defined].. - call rRandom() to create a pseudorandom keyvalue,
 *    2: [RANDOM undefined] - prompt the user for a keyvalue.
 *       A keysize < wKeySize is padded with spaces, and a keysize > wKeySize
 *       is "chopp'd off", so the size of the returned key is always wKeySize.
 *
 * RETURN
 *    Call-by-reference: A keystring of wKeySize chars placed in pzKeyBuf
 *-4*/

#ifdef RANDOM
    /* 1: Create a pseudorandom key value */
    WORD      wRan  = 0;           /* One random number */
    WORD      wLet  = 0;               /* One random generated letter in keystr. */
    WORD      wKsz  = wKeySize;        /* Size of key (# byte) */
    char     *pzKp  = pzKeyBuf;        /* Pointer to step through keystring */
    float     fShft = (float) 10000.0; /* Factor for mult. random fraction */

    while (wKsz-- > 0) {
	wRan = (WORD) ((float)rRandom() * fShft);
	wLet = (wRan % 26) + 'a';
	*pzKp++ = (char) ((wRan % 10) ? wLet : toupper(wLet));
    }
#else
    /* 2: Get a key value from stdin */
    fprintf(stdout, "\nEnter a key value %d chars -> ", wKeySize);	/* prompt */
    strncpy(pzKeyBuf, "                              ", wKeySize);	/* clear */
    fgets(pzKeyBuf, wKeySize, stdin);
    while (getchar() != '\n')
       /* eat rest of the line */ ;
#endif

    /* Zero-terminate (& "clamp"/chop off) the key value at size "wKeySize" */
    *(pzKeyBuf + wKeySize) = '\0';

}   /* END function vKeyGet() */



/*+4 MODULE HASH.C----------------------------------------------------------*/
/*   NAME   03                    wKeyPrint                                 */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   WORD
wKeyPrint(pzKeyBuf, dwDatOff)
    char     *pzKeyBuf;	    /* Keystring, - not zero terminated */
    DWORD     dwDatOff;	    /* Corresponding datafile offset */
{
/* DESCRIPTION
 *    Support function for REAL_MAIN() test driver.
 *    Print one hashindex key-record: (keyvalue,offset) on stdout; -
 *    The function may be passed as parameter to function : eHashIdxProcess()
 *    for looping through the indexfile and printing the contents of all valid
 *    (ie. USED) indexrecords.
 * RETURN
 *    Side effects ........: Hashindex records (key & offset) printed on stdout
 *    Function return value: '1' indicating to eHashIdxProcess() that printing
 *                           should continue for ALL all key-records.
 *-4*/

    /* Make sure key-string is zero-terminated */
    strncpy(pzKBuf, pzKeyBuf, Ksize);
    *(pzKBuf + Ksize) = '\0';

    /* Print corresponding key-value and datafile offset */
    fprintf(stdout, "Key[%s]-(lookup)->FlatfileOffset[%lu]\n", pzKBuf, dwDatOff);

    /* "Again" code : [1] continue, [0] stop - cf. eHashIdxProcess() */
    return (1);

}   /* END function wKeyPrint() */



/*+4 MODULE HASH.C----------------------------------------------------------*/
/*   NAME   04                    eIdxStatPrint                             */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   eRetType
eIdxStatPrint(pH)
    HASH     *pH;	       /* Handle for hashindex descr.struct */
{
/* DESCRIPTION
 *    Support function for REAL_MAIN() test driver.
 *    Print size and load-statistics for hashindex file.
 * RETURN
 *    Side effects ........: Hashindex status (size & load) printed on stdout
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 *-4*/

    DWORD     dwSize = 0L;     /* Var. parameter for eHashIdxGetSize */
    DWORD     dwUsed = 0L;     /* Ditto */
    WORD      wLoad  = 0;      /* Indexfile load factor */

    ACHK_ERR(eHashIdxGetSize(pH, &dwSize, &dwUsed), A_STOP)
    fprintf(stdout, "Hashindex keyrecords : Size=[%lu], Used=[%lu]\n", dwSize, dwUsed);

    ACHK_ERR(eHashIdxGetLoad(pH, &wLoad), A_STOP)
    fprintf(stdout, "Loadfactor=%d\n", wLoad);

    ARET_OK

}   /* END function eIdxStatPrint() */

#endif /* MAIN */




/****************************************************************************/
/**************************** HASH ALGORITHM ********************************/
/****************************************************************************/

/* Define machine-independent percentages of DWORD datatype (cf. general.h) */
/* obs: LINT may quabble about "Unusual use of Boolean" & "Suspicious cast" */
PRIVATE DWORD dw12P = ((DWORD)( (NBITS(DWORD)) * 0.125));  /* 12% of dwbits */
PRIVATE DWORD dw75P = ((DWORD)( (NBITS(DWORD)) * 0.75 ));  /* 75% of dwbits */
PRIVATE DWORD dwHB  = ((DWORD)(~((DWORD)(~0) >> ((DWORD)( (NBITS(DWORD)) * 0.125)))));

PRIVATE FLAG  fFirstHash = FALSE;  /* Flag for RF_DBL double hashing method */
PRIVATE DWORD dwPrime2   = 0L;     /* Calculated prime for Index-size */
PRIVATE DWORD dwPrime1   = 0L;     /* Twin-prime (dwPrime2-2L) for RF_DBL */



/*+3 MODULE HASH.C =========================================================*/
/*   NAME   05               dwHashFunc                                     */
/*== SYNOPSIS ==============================================================*/
PRIVATE   DWORD
dwHashFunc(pcKeystr, wKsize, dwMaxSlot)
    char     *pcKeystr;        /* Keystring to hash */
    WORD      wKsize;          /* Size of keystring (# byte) */
    DWORD     dwMaxSlot;       /* Number of "slots" in target hash-table */
{
/* DESCRIPTION
 *    Hash algorithms. The following transformations are offered :
 *       1: Cichelli's First+Last+Length - Not usefull for keys of fixed length
 *       2: Simple Sum-of-chars - Can't handle permutations & large tabels
 *       3: "P.J.Weinburger" hash - slower but solves the problems [DEFAULT]
 * RETURN
 *    Function return value: Hashvalue of pcKeystr, cf function spec: HF_TYPE.
 *-3*/

    D(char   *pcKst  = pcKeystr;)   /* Variable for DEBUG trace */
    D(WORD   wKsz    = wKsize;)     /* Variable for DEBUG trace */
    int      iKeylen       = 0;     /* Length of keystring */
    register DWORD dwHval1 = 0L;    /* Scratch hash calc. variables */
    register DWORD dwHval2 = 0L;    /* Scratch hash calc. variables */

    assert(HF_TYPE == HF_FLL || HF_TYPE == HF_ADD || HF_TYPE == HF_PJW);


    switch (HF_TYPE) {

	case HF_FLL:
	    /* Cichelli's "minimal perfect hash function" candidate :  */
	    /* First+Last+Length; Not usefull for keys of fixed length */
	    iKeylen = strlen(pcKeystr);
	    dwHval1 = (DWORD) ((*pcKeystr << 8) + *(pcKeystr + iKeylen - 1) \
			       +iKeylen) % dwMaxSlot;
	    break;

	case HF_ADD:
	    /* "Sum-of-chars" hash function */
	    /* Simple and fast, not good at handling permutations & large tables */
	    for (dwHval1 = (DWORD) *pcKeystr; wKsize-- > (WORD) 0 && *pcKeystr != '\0'; pcKeystr++)
		   dwHval1 = ((dwHval1 << 8) + (DWORD) *pcKeystr) % dwMaxSlot;
	    /* Terminates at keysize or EOS */
	    break;

        default :
            /*FALLTHRU*/
            /* Fall through to default hash method: HF_PJW */
	case HF_PJW:
	    /* "P.J.Weinburger" compiler hash func. for C-pgm. identifiers */
	    /* Somewhat slower than HF_ADD, - but solves the problems      */
	    for (dwHval1 = (DWORD)0; wKsize-- > (WORD)0 && *pcKeystr != '\0'; pcKeystr++) {
               dwHval1 = (dwHval1 << dw12P) + (DWORD) *pcKeystr;
	       if ( (dwHval2 = (dwHval1 & dwHB)) > (DWORD)0L )
	          dwHval1 = ((dwHval1 ^ (dwHval2 >> dw75P)) & ~dwHB);
	    }
	    dwHval1 = (DWORD) (dwHval1 % dwMaxSlot);
	    break;
    }

    fFirstHash = TRUE;     /* Raise flag for 1. hashing */

    D(vHashTrace(pcKst, wKsz, dwHval1));
    STCK("dwHashFunc");
    return (dwHval1);

}   /* END function dwHashFunc() */



/*+3 MODULE HASH.C =========================================================*/
/*   NAME   06               dwRehashFunc                                   */
/*== SYNOPSIS ==============================================================*/
PRIVATE   DWORD
dwRehashFunc(pcKeystr, wKsize, dwCurSlot, dwMaxSlot)
    char     *pcKeystr;        /* Keystring for Re-hash */
    WORD      wKsize;          /* Size of keystring (# byte) */
    DWORD     dwCurSlot;       /* Current "slot" in hash-table for Re-hash */
    DWORD     dwMaxSlot;       /* Number of "slots" in target hash-table */
{
/* DESCRIPTION
 *    Collision-resolution algorithms. The following transformations are offered:
 *    1: Linear rehashing with constant increment = 1
 *   [2: Quadratic rehashing - Not implemented (reserved for future use)]
 *    3: Double hashing with twin primes [DEFAULT]
 * RETURN
 *    Function return value: ReHashvalue of pcKeystr, cf function spec: RF_TYPE.
 *-3*/

    register DWORD dwRval  = 0L;     /* Rehash value */
    static   DWORD dwRincr = 0L;     /* Rehash increment, calc. on 1.st RF_DBL */

    assert(RF_TYPE == RF_LIN || RF_TYPE == RF_DBL);


    switch (RF_TYPE) {

	case RF_LIN:
           /* Linear rehashing with constant increment = 1 */
           dwRval = (dwCurSlot + 1) % dwMaxSlot;
           break;

	case RF_QAD:
           /* Quadratic rehashing - reserved for future use */
           /* ---Not implemented, bail out if used!--- */
           assert(TRUE == FALSE);            /* Never! */
           break;


	default:
           /*FALLTHRU*/
           /* Fall through to default hash method: RF_DBL */
	case RF_DBL:
           /* Double hashing with twin primes */
           if (fFirstHash == TRUE) {
              dwRincr = dwHashFunc(pcKeystr, wKsize, dwPrime1) + 1L;
              fFirstHash = FALSE;
           }
           dwRval = (dwCurSlot + dwRincr) % dwMaxSlot;
           break;
    }

    STCK("dwRehashFunc");
    return (dwRval);

}   /* END function dwRehashFunc() */



#ifdef DEBUG
static char rgcKbf[80];

/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   07               vHashTrace                                     */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vHashTrace(pcKeystr, wKsize, dwCurSlot)
    char     *pcKeystr;        /* key value (string) */
    WORD      wKsize;          /* key size (#byte) */
    DWORD     dwCurSlot;       /* Current "slot" in target hash-table */
{
/* DESCRIPTION
 *    Trace a hashing lookup : print (key-string, hashtable-slot)
 *-4*/

    char     *pcKst = pcKeystr;

    strncpy(rgcKbf, pcKst, wKsize);
    rgcKbf[wKsize] = '\0';

    fprintf(stdout, "Key[%s]-(hash)->HashfileKeyRecord[%lu]\n", rgcKbf, dwCurSlot);

}   /* END function vHashTrace() */

#endif   /* ifdef DEBUG */




/****************************************************************************/
/**************************** HASHINDEX HEADER ******************************/
/****************************************************************************/
#define INTEGRITY_UNKNOWN   0L	    /* set when opened for WRITE */
#define INTEGRITY_OK       -1L	    /* set when closed properly */


/*+2 MODULE HASH.C =========================================================*/
/*   NAME   08               eHashIdxCreate                                 */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eHashIdxCreate(pH, pzIdxFile, wKsize, dwIsize)
    HASH     *pH;          /* handle for hashindex descr.struct to create */
    char     *pzIdxFile;   /* name of the physical hashindex (disk file) */
    WORD      wKsize;      /* size of the keys to use (# byte per key) */
    DWORD     dwIsize;     /* size of the hashindex file (# key records) */
{
/* DESCRIPTION
 *    Creates (and opens) a hash-index file in the current directory on disk
 *    with the name of "pzIdxFile". Up to "dwIsize" key records with a key
 *    size of "wKsize" may be inserted into the index (using eHashKeyInsert)
 *    before expansion becomes nessecary.
 *
 *    1.1: Allocates an "incore" hashindex descriptor (struct stHcore), and
 *         "points" the handle (HASH *pH) to this structure.
 *    1.2: Creates a new hashindex file with the name "pzIdxFile", and inserts
 *         the hashindex filedescriptor and filename into the HASH descriptor.
 *    1.3: Adjusts the size of the hashindex to next higher prime (performance).
 *    1.4: Initializes the hashindex size-information (struct stHsize, incor-
 *         porated in the hashindex descriptor).
 *    2.1. Copies the hashindex size-information to a hashindex fileheader
 *         (struct stHdisk), and writes the fileheader to the hashindex.
 *    2.2: Writes "dwIsize" number of empty key records (struct stHkey) to
 *         the hashindex. The key records are allocated "wKsize" bytes for a
 *         keyvalue and a LONG for use as index into a normal flat datafile.
 * RETURN
 *    Call-by-reference ...: Handle pH pointed to incore HASH index descriptor.
 *    Side effects ........: Hashindex "pzIdxFile" created & initialized (open).
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 * EXAMPLE
 *     #include "hash.h"
 *     HASH H;
 *     if (eHashIdxCreate(&H, "HASH.IDX", sizeof(datakey), 100) == ERROR)
 *        fprintf(stderr, "ERROR[%d] : %s\n", Astat, ERRMSG[Astat]);
 * SEE ALSO
 *    hash.h, for a detailed description of data structures and return codes.
 *-2*/

    struct   stHdisk stIdxHdr;     /* Hash index header record structure */
    struct   stHkey stKeyRec;      /* Hash index key record structure */
    FILE     *fd      = NULL;      /* Hash index file descriptor */
    eRetType eRetCode = ERROR;     /* Hash function return code */
    register DWORD dwCount = 0L;   /* Scracth count variable */


    /* 0: Check that indexfile not already exists */
    if ((fd = fopen(pzIdxFile, "r")) != (FILE *) NULL) {
	(void) fclose(fd);
	ARET_ERR(TRUE, A_FILEEXIST, 801);
    }
    assert(wKsize >= 1 && dwIsize >= 3);


    /* ------------------- Hashindex in core ------------------------------- */

    /* 1.1: Build "in core" hashindex descriptor struct. */
    *pH = (HASH) malloc(sizeof(struct stHcore));
    ARET_ERR(*pH == NULL, A_BADALLOC, 802);

    /* 1.2: Create (and open) the hashindex file, mode read/write */
    (*pH)->fd = fopen(pzIdxFile, "w+b");
    ARET_ERR((*pH)->fd == NULL, A_NOTCREATE, 803);
    ARET_ERR(pzStrcpyAlloc(&(*pH)->filename, pzIdxFile) == NULL, A_BADALLOC, 804);

    /* 1.3: Compute prime size of hashindex file (# key records) */
    dwPrime2 = dwIsize + 1L;    /* 1 empty slot required for search termination */
    vGetPrime(&dwPrime2, TRUE);
    if (RF_TYPE == RF_DBL) {    /* Find twin primes if double hashing */
	while (!fIsPrime(dwPrime2 - 2L)) {
	    D(fprintf(stdout, "MAXKEY2[%ld]\n", dwPrime2));
	    dwPrime2++;
	    vGetPrime(&dwPrime2, TRUE);
	}
	dwPrime1 = dwPrime2 - 2L;
	D(fprintf(stdout, "MAXKEY2[%ld]\n", dwPrime2));
	D(fprintf(stdout, "MAXKEY1[%ld]\n", dwPrime1));
    }

    /* 1.4: Initialize the hashindex size struct */
    (*pH)->indexsize.wKsize = wKsize;
    (*pH)->indexsize.dwIsize = dwPrime2;
    (*pH)->indexsize.dwIused = 0;
    (*pH)->indexstatus = IOPEN;
    (*pH)->indexmode = RW;


    /* ------------------- Hashindex on disk ------------------------------- */

    /* 2.1: Allocate & init a hashindex fileheader, and write it to file */
    vUpdate_stHdisk(*pH, &stIdxHdr, NO);
    eRetCode = eWriteIdxHdr(*pH, &stIdxHdr);
    ARET_ERR(eRetCode == ERROR, Astat, 805);


    /* 2.2: Write all key-records (ie. create hashindex file in FULL SIZE) */
    stKeyRec.status = VACANT;      /* All records initially empty and free */
    stKeyRec.key = (char *) malloc(wKsize);
    (void) memset(stKeyRec.key, '#', wKsize);
    stKeyRec.dwDatOffset = 0L;

    for (dwCount = 0L; dwCount < (*pH)->indexsize.dwIsize; dwCount++) {
	eRetCode = eWriteIdxKey(*pH, dwCount, &stKeyRec);
	ARET_ERR(eRetCode == ERROR, Astat, 806);
    }
    free(stKeyRec.key);

    STCK("eHashIdxCreate");
    ARET_OK

}   /* END function eHashIdxCreate() */



/*+2 MODULE HASH.C =========================================================*/
/*   NAME   09               eHashIdxOpen                                   */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eHashIdxOpen(pH, pzIdxFile, pzAccess)
    HASH     *pH;          /* handle for hashindex descr.struct. to create */
    char     *pzIdxFile;   /* name of the physical hashindex (disk file) */
    char     *pzAccess;    /* "rb" (for ReadOnly) or "w+b" (for ReadWrite) */
{
/* DESCRIPTION
 *    Opens an already existing hash-index file in the current directory on
 *    disk with the name of "pzIdxFile" and the open-mode of "pzAccess".
 *    Valid modes are : "rb" (ReadOnly access) and "w+b" (ReadWrite access).
 *
 *    1: Opens an existing hash index "pzIdxFile" in mode "pzAccess" ("rb"/"w+b")
 *    2: Allocates an "in core" hashindex descriptor structure (struct stHcore)
 *       and points the handle passed as parameter (HASH* pH) to this structure.
 *       (The initial parameter-value of the handle is assumed to be NULL).
 *    3: Verifies the hash index header record by calculating a CCITT-CRC of
 *       the embedded struct stHsize, and compares this value with a previ-
 *       ously calculated checksum (also stored in the header).
 *    4: Initializes the "in core" hash index descriptor with basic file
 *       information : open-status, filename, filesize.
 *    5: Verifies the hash index file "integrity" field, which indicates a
 *       succesful close-operation after a Read/Write acces to the hash index.
 *       If the hash index has not been closed properly (bad CRC), the
 *       function restores the essential size-information in the file header.
 *       NB: The integrity-verification is designed for SingleUser creation,
 *       and maintenamce of hashindex'es in acces-mode ReadWrite; - it does
 *       NOT apply (and is not relevant) for multiuser acces in ReadOnly mode.
 *    6: Patch, for "double hashing". This hashing method requires a set of
 *       "twin primes": the table size (T) for hashing, and T-2 for rehashing.
 * RETURN
 *    Call-by-reference ...: Handle pH pointed to incore HASH index descriptor.
 *    Side effects ........: Hashindex "filename" opened in mode "access".
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 * EXAMPLE
 *     #include "hash.h"
 *     HASH H;
 *     if (eHashIdxOpen(&H, "HASH.IDX", "rb") == ERROR)
 *        fprintf(stderr, "ERROR[%d] : %s\n", Astat, ERRMSG[Astat]);
 * SEE ALSO
 *    hash.h, for a detailed description of data structures and return codes.
 *-2*/

    struct    stHdisk stIdxHdr;    /* Hash index header record structure */
    FILE      *fd       = NULL;    /* Hash index file descriptor */
    eRetType  eRetCode  = ERROR;   /* Hash function return code  */
    int       iCheckSum = 0;       /* Hash index checksum (CRC)  */

    
    D(fprintf(stdout, "Opening SS-file[%s], mode[%s] :\n",
	     pzIdxFile, (strcmp(pzAccess, "rb") == 0 ? "RO" : "RW"));)

    /* 1: Open an existing hash index file */
    ARET_ERR((*pH) != NULL && (*pH)->indexstatus == IOPEN, A_ISOPEN, 901);
    fd = fopen(pzIdxFile, pzAccess);
    ARET_ERR(!fd, A_NOFILE, 902);


    /* 2: Allocate an "in core" hashindex descriptor structure */
    *pH = (HASH) malloc(sizeof(struct stHcore));
    ARET_ERR(!*pH, A_BADALLOC, 903);


    /* 3: Verify the hash index file header record (is CRC OK ?) */
    (*pH)->fd = fd;
    eRetCode = eReadIdxHdr(*pH, &stIdxHdr);	/* read the header record */
    ARET_ERR(eRetCode == ERROR, Astat, 904);

    iCheckSum = wCCITTcrc((char *) (&stIdxHdr.indexsize), (WORD) sizeof(struct stHsize));
    ARET_ERR(iCheckSum != stIdxHdr.checksum, A_WRONGFILE, 905);


    /* 4: Initialize the "in core" hash index descriptor with hashfile info */
    (*pH)->indexstatus = IOPEN;
    (*pH)->indexmode = (strcmp(pzAccess, "rb") == 0 ? RO : RW);
    ARET_ERR(pzStrcpyAlloc(&(*pH)->filename, pzIdxFile) == NULL, A_BADALLOC, 906);
    (*pH)->indexsize = stIdxHdr.indexsize;

    D(fprintf(stdout, "\tRead SS-header: dwIsize[%ld], dwIused[%ld], wKsize[%d]\n",
       stIdxHdr.indexsize.dwIsize, stIdxHdr.indexsize.dwIused, stIdxHdr.indexsize.wKsize); )

    /* 5:  Verify the hash index file integrity (has file been closed OK?) */
    /* NB: Applies only for "SingleUser" mode (ie. file opened READWRITE)  */
    if ((*pH)->indexmode == RW)    /* Multiuser RO : ignore! */
	if (stIdxHdr.integrity == INTEGRITY_OK) {
	    /* Indexfile has been closed ok, - now mark it open */
	    stIdxHdr.integrity = INTEGRITY_UNKNOWN;
	    eRetCode = eWriteIdxHdr(*pH, &stIdxHdr);
	    ARET_ERR(eRetCode == ERROR, Astat, 907);
	}
	else {
	    /* Indexfile NOT closed properly, - restore index stIdxHdr */
	    eRetCode = eHashIdxRestore(pH);
	    ARET_ERR(eRetCode == ERROR, Astat, 908);
	}

    /* 6: Patch, for double hashing : set global variable for rehash */
    dwPrime1 = (*pH)->indexsize.dwIsize - 2L;

    STCK("eHashIdxOpen");
    ARET_OK

}   /* END function eHashIdxOpen() */



/*+2 MODULE HASH.C =========================================================*/
/*   NAME   10               eHashIdxClose                                  */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eHashIdxClose(pH)
    HASH     *pH;	  /* handle for hashidx descr. w/filename to close */
{
/* DESCRIPTION
 *    Closes an open hashindex file, and releases the resources allocated
 *    for the attached "in core" hashindex descriptor structure.
 *    NB: This function MUST be called before "touching the big red button"
 *    to guarantee validity of the indexfile (if opened in ReadWrite mode).
 *
 *    1: Checks that the hashindex to be closed is in fact open !
 *    2: Updates the hashindex file header record w/ size, CRC and validity.
 *       NB: The indexheader-update is designed for SingleUser creation,
 *       and maintenamce of hashindex'es in acces-mode ReadWrite; - it does
 *       NOT apply (and is not relevant) for multiuser acces in ReadOnly mode.
 *       2.1: Fills in a hashindex header structure (struct stHdisk) with the
 *            current values from the "in core" hashindex descriptor structure
 *            (struct stHcore) pointed to by the handle HASH *pH (parameter).
 *            The actual updating of the header structure is done by calling
 *            the support function vUpdate_stHdisk() to define the header-
 *            values for the hashindex size, header-checksum and file-validity.
 *       2.2: Writes the updated hashindex header structure (struct stHdisk)
 *            as the first record to the hashindex file.
 *    4: Closes the hashindex file.
 *    5: Frees the "in core" hashindex descriptor structure (struct stHcore)
 *       pointed to by the handle HASH *pH (passed as parameter).
 * RETURN
 *    Side effects ........: Hashindex file (*pH)->filename closed.
 *                           Incore HASH index descriptor "free'd" (pH=NULL)
 *    Function return value: OK if operation succeeded, ERROR otherwise.
 *                           If ERROR, "Astat" holds the precise error code.
 * EXAMPLE
 *     #include "hash.h"
 *     HASH H;
 *     if (eHashIdxClose(&H) == ERROR)
 *        fprintf(stderr, "ERROR[%d] : %s\n", Astat, ERRMSG[Astat]);
 * SEE ALSO
 *    hash.h, for a detailed description of data structures and return codes.
 *-2*/

    struct stHdisk stIdxHdr;   	/* Hash index header record structure */
    eRetType  eRetCode = ERROR;	/* Hash function return code */


    /* 1: Check that the hashindex to be closed is actually open by now ! */
    ARET_ERR((*pH)->indexstatus != IOPEN, A_NOTOPEN, 1001);


    /* 2: Reset the hashindex header : size, checksum and integrity fields */
    /* NB: Applies only for "SingleUser" mode (ie. file opened READWRITE)  */
    if ((*pH)->indexmode == RW) {  /* Multiuser RO : ignore */

	/* 2.1: Fill in a hashindex header from the hashindex descriptor */
	vUpdate_stHdisk(*pH, &stIdxHdr, YES);

	/* 2.2: Write the updated hashindex header as 1.rec. in indexfile */
	eRetCode = eWriteIdxHdr(*pH, &stIdxHdr);
	ARET_ERR(eRetCode == ERROR, Astat, 1002);
    }

    /* 3: Close the hashindex file */
    eRetCode = (fclose((*pH)->fd) != 0 ? ERROR : OK);
    ARET_ERR(eRetCode == ERROR, A_FILECLOSE, 1003);
    (*pH)->indexstatus = ICLOSED;


    /* 4: Free the "in core" hashindex descriptor structure */
    free((*pH)->filename);
    free((char *) (*pH));
    pH = NULL;


    STCK("eHashIdxClose");
    ARET_OK

}   /* END function eHashIdxClose() */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   11               vUpdate_stHdisk                                */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vUpdate_stHdisk(H, pstIdxHdr, eIntegr)
    HASH     H;                    /* Addr. of hashindex descr. w/ size info */
    struct   stHdisk *pstIdxHdr;   /* Ptr. to hashindex hdr record structure */
    eAnsType eIntegr;              /* Integrity statys : [YES | NO] */
{
/* DESCRIPTION
 *    Support function (common "Workhorse")
 *    Fills in a hashindex header structure (struct stHdisk) passed as param.
 *    Called from eHashIdxCreate() and eHashIdxClose().
 *
 *    1: Copies INDEXSIZE from the hashindex "in core" descriptor (param H).
 *    2: Calculates CHECKSUM from the fileheader's : struct stHsize.
 *    3: Sets the header integrity depending on the boolean parameter "eIntegr"
 *       -  NO : integrity is NOT guaranteed (for create and open in mode RW)
 *       - YES : integrity is guarenteed     (for open in mode RO)
 *-4*/

    /* 1: Copy current hashindex size from index descriptor to header rec. */
    pstIdxHdr->indexsize = H->indexsize;

    /* 2: Set header checksum to calculated CRC for file's "struct stHsize" */
    pstIdxHdr->checksum = \
	wCCITTcrc((char *) (&pstIdxHdr->indexsize), (WORD) sizeof(struct stHsize));

    /* 3: set header integrity depending on boolean parameter "eIntegr" */
    pstIdxHdr->integrity = (eIntegr == YES) ? INTEGRITY_OK : INTEGRITY_UNKNOWN;

}   /* END function vUpdate_stHdisk() */




/****************************************************************************/
/**************************** HASHINDEX KEYRECORDS **************************/
/****************************************************************************/


/*+2 MODULE HASH.C =========================================================*/
/*   NAME   12            eHashKeyInsert                                    */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eHashKeyInsert(pH, key, dwDatOffset)
    HASH     *pH;          /* Handle for hashindex descr. w/ indexfile */
    char     *key;         /* Key string, inserted in hashindex record */
    DWORD     dwDatOffset; /* DataFile offset ins. in hashindex record */
{
/* DESCRIPTION
 *    Insert a key and its corresponding datafile offset into a hashindex.
 *
 *    1: CHECK hashindex-file is open, and there is room for new key-record.
 *    2: FIND FREE SLOT in hashindex-file :
 *       2.1 Hash new key to initial position in hashindex-file.
 *       2.2: LOOP: compare new key to keyvalue of current slot in indexfile,-
 *               If collision (and not duplicate): rehash
 *            UNTIL free slot (ie. [VACANT | EMPTY]) located
 *    3: INSERT NEW KEY-RECORD in the free slot.
 *
 * RETURN
 *    OK if operation succeded, else ERROR (Astat holds precise error-code).
 *
 * BUGS
 *    Duplicate keys are NOT allowed (this is reasonable)
 *-2*/

    DWORD    *pdwIsize  = NULL;    /* Ptr. to size of index (# available records) */
    DWORD    *pdwIused  = NULL;    /* Ptr. to # indexrecords used right now */
    WORD     *pwKsize   = NULL;    /* Ptr. to size of key (# chars) */
    DWORD     dwCurSlot = 0L;      /* (Re)hashed slot in indexfile */
    struct stHkey stKeyRec;        /* File entry (key-record) */
    FLAG      fDupkey   = TRUE;    /* Boolean: Duplicate key? [Y|N] */
    FLAG      fAgain    = TRUE;    /* Boolean: Test & Rehash? [Y|N] */
    eRetType  eRetCode  = ERROR;   /* ReturnCode */


    /*------ 1: CHECK open hashindex-file, and room for new key-record -----*/
    ARET_ERR((*pH)->indexstatus != IOPEN, A_NOTOPEN, 1201);

    pdwIsize = &(*pH)->indexsize.dwIsize;
    pdwIused = &(*pH)->indexsize.dwIused;
    pwKsize = &(*pH)->indexsize.wKsize;

    /* Check room for new record; - 1 slot reserved for search termination! */
    ARET_ERR(*pdwIused + 2 > *pdwIsize, A_FULL, 1202);


    /*----------------- 2: FIND FREE SLOT in hashindex-file ----------------*/

    /* 2.1 Hash new key to initial position in hashindex-file */
    dwCurSlot = dwHashFunc(key, *pwKsize, *pdwIsize);

    /* Allocate room for key read from hashindex-file (for comparing) */
    stKeyRec.key = (char *) malloc(*pwKsize);
    ARET_ERR(stKeyRec.key == NULL, A_BADALLOC, 1203);

    /* 2.2: LOOP: comp. key to current slot in indexfile UNTIL empty slot */
    while (fAgain) {

	/* Read current key-record ("slot") in hashindex */
	eRetCode = eReadIdxKey(*pH, dwCurSlot, &stKeyRec);
	ARET_ERR(eRetCode == ERROR, A_READ, 1204);

	/* Test if collision (and not duplicate);- if yes: rehash */
	fDupkey = (strncmp(key, stKeyRec.key, *pwKsize) == 0);
	if (stKeyRec.status == USED && !fDupkey)
	    dwCurSlot = dwRehashFunc(key, *pwKsize, dwCurSlot, *pdwIsize);
	else
	    fAgain = FALSE;    /* Status VACANT or DELETED */
    }

    /* Free room for key read from hashindex-file (for comparing) */
    free(stKeyRec.key);


    /*---- 3: Free slot (VACANT|DELETED) located, INSERT NEW KEY-RECORD ----*/
    if (!fDupkey) {
	stKeyRec.key = key;
	stKeyRec.status = USED;
	stKeyRec.dwDatOffset = dwDatOffset;

	eRetCode = eWriteIdxKey(*pH, dwCurSlot, &stKeyRec);
	ARET_ERR(eRetCode == ERROR, A_WRITE, 1205);
	(*pdwIused)++;
    }
    else
	ARET_ERR(TRUE, A_DUPLICATE, 1206);


    STCK("eHashKeyInsert");
    ARET_OK

}   /* END function eHashKeyInsert() */



/*+2 MODULE HASH.C =========================================================*/
/*   NAME   13            eHashKeyDelete                                    */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eHashKeyDelete(pH, key)
    HASH     *pH;             /* Handle for hashindex descriptor */
    char     *key;            /* Keystring defining rec. in hashindex to delete */
{
/* DESCRIPTION
 *    Deletes a hashindex entry by marking it "DELETED"; -
 *
 *    NB: we can't simply mark the entry "VACANT" (for reuse), since  there
 *    may be items that have previously collided with the item to be deleted,
 *    and these items would then become unreachable via. rehash-chains!
 *
 * RETURN
 *    OK when item found and marked "DELETED",
 *    else ERROR (Astat holds precise error-code).
 *-2*/

    DWORD    dwDatOffset = 0L;     /* Var param, - NOT actually used here */
    DWORD    dwCurSlot   = 0L;     /* Slot in indexfile for current key-record */
    struct   stHkey stKeyRec;      /* Structure holding current key record */
    eRetType eRetCode    = ERROR;  /* ReturnCode */


    /* 1: Locate key-record in index-file */
    eRetCode = eLocateKey(*pH, key, &dwCurSlot, &dwDatOffset);

    /* 2: Mark key-record "DELETED" */
    if (eRetCode == OK) {
	stKeyRec.status = DELETED;
	eRetCode = eWriteIdxKey(*pH, dwCurSlot, &stKeyRec);
	ARET_ERR(eRetCode == ERROR, Astat, 1301);
	(*pH)->indexsize.dwIused--;
    }
    else
	ARET_ERR(TRUE, A_NOTFOUND, 1302);

    STCK("eHashKeyDelete");
    ARET_OK

}   /* END function eHashKeyDelete() */



/*+2 MODULE HASH.C =========================================================*/
/*    NAME   14           eHashKeyFind                                      */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eHashKeyFind(pH, key, pdwDatOffset)
    HASH     *pH;              /* Handle for hashindex descriptor */
    char     *key;             /* Keystring defining rec. in hashindex to find */
    DWORD    *pdwDatOffset;    /* Ptr to var. for returning datafile offset */
{
/* DESCRIPTION
 *    Find the datafile-offset associated with a given key-value
 *    by looking up the key-record in the indexfile
 *
 * RETURN
 *    OK when key is found, else ERROR (Astat holds precise error-code)
 *-2*/

    DWORD     dwKeySlot = 0L;       /* Var param, -NOT actually used here */

    return (eLocateKey(*pH, key, &dwKeySlot, pdwDatOffset));

}   /* END function eHashKeyFind() */



/*+3 MODULE HASH.C =========================================================*/
/*    NAME   15              eLocateKey                                     */
/*=== SYNOPSIS =============================================================*/
PRIVATE   eRetType
eLocateKey(H, key, pdwKeySlot, pdwDatOffset)
    HASH    H;             /* Hashindex descriptor */
    char    *key;          /* Keystring defining rec. in hashindex to locate */
    DWORD   *pdwKeySlot;   /* Ptr to var. for returning keyrecord slot */
    DWORD   *pdwDatOffset; /* Ptr to var. for returning datafile offset */
{
/* DESCRIPTION
 *    Locate a key-record in index H
 *
 *   [0: If DEBUG mode, define statistic variables for tuning and debugging]
 *    1: Get index- and key-size from "in core" hash descriptor
 *    2: Hash key to initial position in hashindex-file
 *    3: LOOP: compare key to keyvalue of current slot in indexfile, -
 *          If (DELETED) or (USED and different key), then rehash key
 *       UNTIL key found or VACANT entry (ie. key not in index!)
 *    4: Set up key-slot and datafile-offset in var-params.
 *   [5: If DEBUG mode, print statistics]
 *
 * RETURN
 *    Key found : OK w/ side-effects dwCurSlot & pdwDatOffset set,
 *    else ERROR (Astat holds precise error-code)
 *-3*/

    DWORD     dwCurSlot = 0L;      /* (Re)hashed slot in indexfile */
    DWORD     dwIsize   = 0L;      /* Size of index (total # key-records) */
    WORD      wKsize    = 0;       /* Size of keystring (# byte) */
    struct stHkey stKeyRec;        /* Structure for hashindex key-record */
    FLAG      fAgain    = TRUE;    /* Boolean: Test & Rehash? [Y|N] */
    int       iCompare  = 0;       /* Result of compare lookup-key w. index-key */
    eRetType  eRetCode  = ERROR;   /* Return code for HASH.C function */
    eRetType  eRetFound = ERROR;   /* Return code for eLocateKey: key found? */


    /* 0: If debug-mode, define statistic variables for tuning & dbg */
    D(static int sum_lookup_all = 1;
    static int sum_probe_all = 1;
    static int sum_lookup_ok = 1;
    static int sum_probe_ok  = 1;
    int    probes = 0;
    );

    /* 1: Get index- and key-size from "in core" hash descriptor */
    ARET_ERR(H->indexstatus != IOPEN, A_NOTOPEN, 1501);
    dwIsize = H->indexsize.dwIsize;
    wKsize = H->indexsize.wKsize;

    /* 2: Hash key to initial position in hashindex-file */
    dwCurSlot = dwHashFunc(key, wKsize, dwIsize);

    /* allocate room for key read from hashindex-file */
    stKeyRec.key = (char *) malloc(wKsize);
    ARET_ERR(stKeyRec.key == NULL, A_BADALLOC, 1502);


    /* 3: LOOP Lookup key, - rehash if not found */
    while (fAgain) {

	eRetCode = eReadIdxKey(H, dwCurSlot, &stKeyRec);
	ARET_ERR(eRetCode == ERROR, Astat, 1503);
	D(probes++);

	switch (stKeyRec.status) {
	    case VACANT:
		fAgain = FALSE;
		eRetFound = ERROR;
		break;

	    case USED:
		iCompare = strncmp(key, stKeyRec.key, wKsize);
		if (iCompare == 0) {	/* found */
		    fAgain = FALSE;
		    eRetFound = OK;
		}
		else
		    dwCurSlot = dwRehashFunc(key, wKsize, dwCurSlot, dwIsize);
		break;

	    case DELETED:
		dwCurSlot = dwRehashFunc(key, wKsize, dwCurSlot, dwIsize);
		break;

	    default:
		ARET_ERR(TRUE, A_OTHER, 1504);
	}
    }

    /* 4: Set up key-slot and datafile-offset in var-params. */
    *pdwKeySlot = dwCurSlot;   /* only valid when found is true */
    *pdwDatOffset = stKeyRec.dwDatOffset;	/* ditto */

    free(stKeyRec.key);


    /* 5: If DEBUG mode: print statistics */
	D(if (eRetFound == OK) {
	sum_lookup_ok++;
	sum_probe_ok += probes;
	}
    sum_lookup_all++;
    sum_probe_all += probes;
    );

    D(fprintf(stdout, "SUM OK : lookup=[%d], probe=[%d], probe/lookup=[%.2f]\n", \
	      sum_lookup_ok, sum_probe_ok, \
	      ((float) sum_probe_ok) / ((float) sum_lookup_ok)));

    D(fprintf(stdout, "SUM ALL: lookup=[%d], probe=[%d], probe/lookup=[%.2f]\n", \
	      sum_lookup_all, sum_probe_all, \
	      ((float) sum_probe_all) / ((float) sum_lookup_all)));


    STCK("eLocateKey");
    Aid = 1505;
    Astat = (eRetFound == OK ? A_OK : A_NOTFOUND);
    return (eRetFound);

}   /* END function eLocateKey() */




/****************************************************************************/
/**************************** HASHINDEX REORGANIZE **************************/
/****************************************************************************/


/*+3 MODULE HASH.C =========================================================*/
/*   NAME   16               eHashIdxRestore                                */
/*== SYNOPSIS ==============================================================*/
PRIVATE   eRetType
eHashIdxRestore(pH)
    HASH     *pH;	       /* Handle for "in core" hashindex descr. */
{
/* DESCRIPTION
 *    Restore index-information of an "in core" hashindex descriptor
 *    by recalculating the total number of keyrecordes in the indexfile
 *    and counting the number of used records in the file.
 *
 *    1: Find the current size (#byte) of the hashindex file
 *    2: Compute the total # hashindex keyrecords from the filesize
 *    3: Scan the hashindex file and count all USED key-entries
 *    4: Update the hashindex "in core" structure with info from 1 & 3
 *
 * RETURN
 *    OK if operation succeded, else ERROR (Astat holds precise error-code).
 *-3*/

    WORD      wKsize   = (*pH)->indexsize.wKsize; /* Keysize, - assumed OK! */
    DWORD     dwIsize  = 0L;       /* Hashindex size (# keyrecords) */
    DWORD     dwIused  = 0L;       /* Hashindex usage (# USED keyrecords) */
    register  DWORD dwCount = 0L;  /* Scratch count variable */
    FILE      *F = (*pH)->fd;      /* Handle of hashindex file */
    struct    stHkey stKeyRec;     /* Structure for hashindex key-record */
    long      filesize = 0L;       /* Size of hashindex file (# byte) */
    eRetType  eRetCode = ERROR;    /* Return code for HASH.C function */


    /* 1: Find hashindex filesize (#byte) */
    eRetCode = (fseek(F, 0L, SEEK_END) != 0 ? ERROR : OK);
    ARET_ERR(eRetCode == ERROR, A_SEEK, 1601);

    filesize = ftell(F);
    ARET_ERR(filesize == -1L, A_READ, 1602);


    /* 2: Compute hashindex keyrecords (TOTAL number); - wKsize assumed OK. */
    /* NB: the index is always created in full size by eHashIdxCreate() */
    dwIsize = (filesize - sizeof(struct stHdisk)) /
	(sizeof(struct stHkey) - sizeof(char *) + wKsize);


    /* 3: Scan hashindex file and count USED key-entries */
    stKeyRec.key = (char *) malloc(wKsize);
    ARET_ERR(stKeyRec.key == NULL, A_BADALLOC, 1603);

    for (dwCount = 0L; dwCount < dwIsize; dwCount++) {
	eRetCode = eReadIdxKey(*pH, dwCount, &stKeyRec);
	ARET_ERR(eRetCode == ERROR, Astat, 1604);

	if (stKeyRec.status == USED)
	    dwIused++;
    }

    free(stKeyRec.key);


    /* 4: Restore (update) hashindex index-info structure */
    (*pH)->indexsize.dwIsize = dwIsize;
    (*pH)->indexsize.dwIused = dwIused;

    STCK("eHashIdxRestore");
    ARET_OK

}   /* END function eHashIdxRestore() */



/* Temporary (scratch) HASH handle used by eHashIdxResize() & wCopy2New() */
static HASH Temp_hash;


/*+2 MODULE HASH.C =========================================================*/
/*   NAME   17            eHashIdxResize                                    */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eHashIdxResize(pH, iResize)
    HASH     *pH;          /* Handle for "in core" hashindex descr. */
    int       iResize;     /* Factor for resizing the hashindex */
{
/* DESCRIPTION
 *    Resize a hashindex file, - Must be called when index is nearly full.
 *    iRezize is the percentage the index will be resized by, ie :
 *    iResize>100 : expansion of index, and iResize<100 : shrinking of index.
 *
 *    When eHashIdxGetLoad sets Astat to A_XPAND, is it recommended to call
 *    this function with a resize factor ("iResize") of 200, thus doubling
 *    the index file to keep performance high.
 *    The index can NOT be shrinked to less than it's actual size of used
 *    keyrecords, - when trying to do so, you will get an unchanged index.
 *
 *    PREPARE OLD
 *       1.1: Check old hashindex file is open and RW
 *       1.2: Save name of old hashindex file (from index descriptor)
 *    PREPARE NEW
 *       2.1: Set temporary name for new hashindex file
 *       2.2: Calculate new (prime) size of hashindex file
 *       2.3: Create & open new hashindex file (as temporary file)
 *    COPY OLD -> NEW
 *       3: Copy all key-records from old to new hashindex file
 *    CLEAN UP
 *       4.1: Close old and new hashindex descriptors & files
 *       4.2: Remove old file, rename new to old, and open new file as old
 *
 * RETURN
 *    OK if operation succeded, else ERROR (Astat holds precise error-code).
 *-2*/

    DWORD     dwIsize = (*pH)->indexsize.dwIsize;    /* # keyrecords) */
    DWORD     dwIused = (*pH)->indexsize.dwIused;    /* # USED keyrecords) */
    WORD      wKsize  = (*pH)->indexsize.wKsize;     /* # byte in keystring */
    DWORD     dwNewsize = 0L;      /* New size of index - after resizing */
    char     *pzOldFile = NULL;    /* Name of current indexfile */
    char     *pzNewFile = NULL;    /* Name of new (resized) indexfile */
    eRetType  eRetCode = ERROR;    /* Return code for HASH.C function */
    int       iRes = 0;	           /* Resultcode for file operation */


    /* 1.1: Check old hashindex file is open and RW */
    ARET_ERR((*pH)->indexstatus != IOPEN, A_NOTOPEN, 1701);
    ARET_ERR((*pH)->indexmode != RW, A_OTHER, 1702);

    /* 1.2: Save name of old hashindex file (from index descriptor) */
    pzOldFile = malloc(strlen((*pH)->filename) + 1);
    ARET_ERR(!pzOldFile, A_BADALLOC, 1703);
    (void) strcpy(pzOldFile, (*pH)->filename);


    /* 2.1: Set temporary name for new hashindex file */
    pzNewFile = "HASH_TMP";
    (void) remove(pzNewFile);  /* just in case ... */

    /* 2.2: Calculate new (prime) size of hashindex file */
    dwNewsize = dwIsize * iResize / 100;
    if (dwNewsize < dwIused)
	dwNewsize = dwIused;
    vGetPrime(&dwNewsize, TRUE);

    /* 2.3: Create & open new hashindex file (as temporary file) */
    eRetCode = eHashIdxCreate(&Temp_hash, pzNewFile, wKsize, dwNewsize);
    ARET_ERR(eRetCode == ERROR, Astat, 1704);


    /* 3: Copy all key-records from old to new hashindex file */
    eRetCode = eHashIdxProcess(pH, wCopy2New);
    ARET_ERR(eRetCode == ERROR, Astat, 1705);


    /* 4.1: Close old and new hashindex descriptors & files */
    eRetCode = eHashIdxClose(pH);
    ARET_ERR(eRetCode == ERROR, Astat, 1706);

    eRetCode = eHashIdxClose(&Temp_hash);
    ARET_ERR(eRetCode == ERROR, Astat, 1707);


    /* 4.2: Remove old file, rename new to old, and open new file as old */
    iRes = remove(pzOldFile);
    ARET_ERR(iRes != 0, A_NOTCREATE, 1708);

    iRes = rename(pzNewFile, pzOldFile);
    ARET_ERR(iRes != 0, A_NOTCREATE, 1709);

    eRetCode = eHashIdxOpen(pH, pzOldFile, "r+b");
    ARET_ERR(eRetCode == ERROR, Astat, 1710);

    free(pzOldFile);


    STCK("eHashIdxResize");
    ARET_OK

}   /* END function eHashIdxResize() */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   18               wCopy2New                                   */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   WORD
wCopy2New(key, dwDatOffset)
    char     *key;
    DWORD     dwDatOffset;
{
/* DESCRIPTION
 *    This function is passed as parameter to function "eHashIdxProcess"
 *    called by eHashIdxResize to regenerate a hash indexfile.
 *-4*/
    eRetType  eRetCode = ERROR;

    eRetCode = eHashKeyInsert(&Temp_hash, key, dwDatOffset);

    return (eRetCode != ERROR ? 1 : 0);	/* 1 means continue */

}   /* END function wCopy2New() */




/****************************************************************************/
/**************************** STATISTICS & TRAVERSAL ************************/
/****************************************************************************/


/*+2 MODULE HASH.C =========================================================*/
/*   NAME   19               eHashIdxGetSize                                    */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eHashIdxGetSize(pH, pdwSize, pdwUsed)
    HASH     *pH;          /* Handle for "in core" hashindex descr. */
    DWORD    *pdwSize;     /* Ptr. to var for returning total # records */
    DWORD    *pdwUsed;     /* Ptr. to var for returning used # records */
{
/* DESCRIPTION
 *    Extracts the size of the hashindex (# records, total & used)
 *    from the hashindex descriptor to the var. parameters.
 * RETURN
 *    OK if operation succeded, else ERROR (Astat holds precise error-code).
 *-2*/

    ARET_ERR((*pH)->indexstatus != IOPEN, A_NOTOPEN, 1901);

    *pdwSize = (*pH)->indexsize.dwIsize;
    *pdwUsed = (*pH)->indexsize.dwIused;

    STCK("eHashIdxGetSize");
    ARET_OK

}   /* END function eHashIdxGetSize() */



/*+2 MODULE HASH.C =========================================================*/
/*   NAME   20               eHashIdxGetLoad                                */
/*== SYNOPSIS ==============================================================*/
PUBLIC eRetType
eHashIdxGetLoad(pH, pwLoad)
    HASH     *pH;          /* Handle for "in core" hashindex descr. */
    WORD     *pwLoad;      /* Ptr. to var for returning index load factor */
{
/* DESCRIPTION
 *    Calculates the load percentage (ie: 100 * used/total) for index H.
 *    When the load factor exceeds 80%, Astat is set to A_XPAND to indicate
 *    that expansion of the index is recommened.
 *
 * RETURN
 *    returns OK  when loadfactor < 80%,
 *    else ERROR (Astat holds precise error-code).
 *-2*/

    /* Check index is open */
    ARET_ERR( (*pH)->indexstatus != IOPEN,	A_NOTOPEN, 2001);


    /* Calculate loadfactor, and return A_XPAND if >= 80% */
    *pwLoad = (WORD) (100 * (*pH)->indexsize.dwIused / ((*pH)->indexsize.dwIsize - 1));
    ARET_ERR( *pwLoad >= 80, A_XPAND, 2002);


    STCK("eHashIdxGetLoad");
    ARET_OK

}   /* END function eHashIdxGetLoad() */



/*+2 MODULE HASH.C =========================================================*/
/*   NAME   21               eHashIdxProcess                               */
/*== SYNOPSIS ==============================================================*/
PUBLIC    eRetType
eHashIdxProcess(pH, ufunc)
    HASH   *pH;                            /* Handle for "in core" H.descr. */ 
    WORD   (*ufunc) P((char *, DWORD));    /* Ptr to function */
{
/* DESCRIPTION
 *    This function is called to process all the key-records in a hashindex.
 *    The function passed as parameter ('ufunc') is called once for each record,
 *    with the keystring and dwDatOffset extracted from the record as parameters.
 *    eHashIdxProcess expects the user-function 'ufunc' to return a value:
 *       0.   if processing must stop
 *       1.   to continue processsing
 *    Normally (1) is returned every time, which means that the processing
 *    will proceed until all records have been processed.
 *
 *    1: Check hashindex is open
 *    2: LOOP: Read next keyrecord from indexfile
 *             call ufunc(keystring, datafileoffset)
 *       UNTIL all records (dwIsize) processed OR break (again = 0)
 *
 * RETURN
 *    OK if all records processed, else ERROR with Astat=A_MOREDATA
 *-2*/

    WORD      wKsize  = (*pH)->indexsize.wKsize;    /* Size of keystring */
    struct    stHkey stKeyRec;     /* Structure for hashindex key-record */
    register  DWORD dwCount = 0L;  /* Scratch count variable */
    WORD      wAgain        = 0;   /* Returncode for user-function 'ufunc' */
    eRetType  eRetCode  = ERROR;   /* Return code for HASH.C function */


    /* 1: Check hashindex is open */
    ARET_ERR((*pH)->indexstatus != IOPEN, A_NOTOPEN, 2101);

    /* 2: LOOP: Read next keyrecord, and call ufunc(keystring, datafileoffset) */
    stKeyRec.key = (char *) malloc(wKsize);
    ARET_ERR(stKeyRec.key == NULL, A_BADALLOC, 2102);

    for (dwCount = 0L; dwCount < (*pH)->indexsize.dwIsize; dwCount++) {

	eRetCode = eReadIdxKey(*pH, dwCount, &stKeyRec);
	ARET_ERR(eRetCode == ERROR, Astat, 2103);

	if (stKeyRec.status == USED) {
	    wAgain = (*ufunc) (stKeyRec.key, stKeyRec.dwDatOffset);
	    if (!wAgain)
		break;
	}
    }

    free(stKeyRec.key);

    /* 3: Ret OK if all records processed, else ERROR with Astat=A_MOREDATA */
    if (!wAgain) {
	Astat = A_MOREDATA;
	return (ERROR);
    }

    STCK("eHashIdxProcess");
    ARET_OK

}   /* END function eHashIdxProcess() */



/****************************************************************************/
/**************************** LOW LEVEL HASHFILE I/O ************************/
/****************************************************************************/


/*+3 MODULE HASH.C =========================================================*/
/*   NAME   22               eWriteIdxHdr                                   */
/*== SYNOPSIS ==============================================================*/
PRIVATE   eRetType
eWriteIdxHdr(H, pstHdrRec)
    HASH      H;	       /* Hash index "in core" descriptor structure */
    struct stHdisk *pstHdrRec; /* Structure holding headerrecord to write */
{
/* DESCRIPTION
 *    Write (immediate) index header-record (stHdisk) to hash index H :
 *    0: Check index open-mode = ReadOnly
 *    1: Set filepointer on header-record (offset 0)
 *    2: Write header-record from struct stHdisk to indexfile H->fd
 * RETURN
 *    OK if operation succeded, else ERROR (Astat holds precise error-code).
 *-3*/
    int       iRetCode = 0;     /* Return code for file operation */

    /* 0: Check index open-mode = ReadOnly */
    ARET_ERR(H->indexmode == RO, A_READONLY, 2201);

    /* 1: Set filepointer on header-record (offset 0) */
    (void) fseek(H->fd, 0, SEEK_SET);

    /* 2: Write header-record from struct stHdisk to indexfile H->fd */
    iRetCode = fwrite(pstHdrRec, sizeof(struct stHdisk), 1, H->fd);
    ARET_ERR(iRetCode == 0, A_WRITE, 2202);

    iRetCode = fflush(H->fd);
    ARET_ERR(iRetCode == EOF, A_WRITE, 2203);


    STCK("eWriteIdxHdr");
    ARET_OK

}   /* END function eWriteIdxHdr() */



/*+3 MODULE HASH.C =========================================================*/
/*   NAME   23               eReadIdxHdr                                    */
/*== SYNOPSIS ==============================================================*/
PRIVATE   eRetType
eReadIdxHdr(H, pstHdrRec)
    HASH      H;               /* Hash index "in core" descriptor structure */
    struct stHdisk *pstHdrRec; /* Structure holding headerrecord to write */
{
/* DESCRIPTION
 *    Read index header-record (stHdisk) from hash index H
 *    1: Set filepointer on header-record (offset 0)
 *    2: Read header-record from indexfile H->fd to struct stHdisk
 * RETURN
 *    OK if operation succeded, else ERROR (Astat holds precise error-code).
 *-3*/
    int       iRetCode = 0;    /* Return code for file operation */

    /* 1: Set filepointer on header-record (offset 0) */
    (void) fseek(H->fd, 0, SEEK_SET);

    /* 2: Read header-record from indexfile H->fd to struct stHdisk */
    iRetCode = fread(pstHdrRec, sizeof(struct stHdisk), 1, H->fd);
    ARET_ERR(iRetCode == 0, A_READ, 2301);


    STCK("eReadIdxHdr");
    ARET_OK

}   /* END function eReadIdxHdr() */



/*+3 MODULE HASH.C =========================================================*/
/*   NAME   24               eWriteIdxKey                                   */
/*== SYNOPSIS ==============================================================*/
PRIVATE   eRetType
eWriteIdxKey(H, dwSlot, pstKeyRec)
    HASH      H;               /* Hash index "in core" descriptor structure */
    DWORD     dwSlot;          /* Slot in hash indexfile for keyrecord write */
    struct stHkey *pstKeyRec;  /* Structure holding keyrecord to write */
{
/* DESCRIPTION
 *    Write a key-record (stHkey) to hash index H, slot dwSlot :
 *    0: Check index open-mode = ReadOnly
 *    1: Set filepointer on key-record according to "dwOffset"
 *    2: Write record-status, key-string & datafile-offset
 *       from struct stHkey to indexfile H->fd (keyrecord dwSlot)
 * RETURN
 *    OK if operation succeded, else ERROR (Astat holds precise error-code).
 *-3*/
    WORD      wKsiz    = H->indexsize.wKsize;          /* Size of keystring */
    DWORD     dwOffset = FILE_OFFSET(dwSlot, wKsiz);   /* Datafile offset   */
    int       iRetCode = 0;               /* Return code for file operation */


    /* 0: Check index open-mode = ReadOnly */
    ARET_ERR((H->indexmode == RO), A_READONLY, 2401);


    /* 1: Set filepointer on key-record according to "dwOffset" */
    iRetCode = fseek(H->fd, dwOffset, SEEK_SET);
    ARET_ERR(iRetCode, A_SEEK, 2402);


    /* 2: Write record-status, key-string & datafile-offset */
    /* from struct stHkey to indexfile H->fd (keyrecord dwSlot) */
    iRetCode = fwrite(&pstKeyRec->status, sizeof(pstKeyRec->status), 1, H->fd);
    ARET_ERR(iRetCode == 0, A_WRITE, 2403);

    iRetCode = fwrite(pstKeyRec->key, wKsiz, 1, H->fd);
    ARET_ERR(iRetCode == 0, A_WRITE, 2404);

    iRetCode = fwrite(&pstKeyRec->dwDatOffset, sizeof(pstKeyRec->dwDatOffset), 1, H->fd);
    ARET_ERR(iRetCode == 0, A_WRITE, 2405);


    STCK("eWriteIdxKey");
    ARET_OK

}   /* END function eWriteIdxKey() */



/*+3 MODULE HASH.C =========================================================*/
/*   NAME   25               eReadIdxKey                                    */
/*== SYNOPSIS ==============================================================*/
PRIVATE   eRetType
eReadIdxKey(H, dwSlot, pstKeyRec)
    HASH      H;               /* Hash index "in core" descriptor structure */
    DWORD     dwSlot;          /* Slot in hash indexfile for keyrecord read */
    struct stHkey *pstKeyRec;  /* Structure for reading keyrecord into */
{
/* DESCRIPTION
 *    Read a key-record (stHkey) from hash index H, slot dwSlot.
 *    1: Set filepointer on key-record according to "dwOffset"
 *    2: Read record-status, key-string & datafile-offset
 *       from indexfile H->fd (keyrecord dwSlot) to struct stHkey
 * RETURN
 *    OK if operation succeded, else ERROR (Astat holds precise error-code).
 *-3*/

    WORD      wKsiz    = H->indexsize.wKsize;          /* Size of keystring */
    DWORD     dwOffset = FILE_OFFSET(dwSlot, wKsiz);   /* Datafile offset   */
    int       iRetCode = 0;               /* Return code for file operation */

    D(fprintf(stdout, "\t\t\tHashfileKeyOffset[%lo]\n", dwOffset));

    /* 1: Set filepointer on key-record according to "dwOffset" */
    iRetCode = fseek(H->fd, dwOffset, SEEK_SET);
    ARET_ERR(iRetCode, A_SEEK, 2501);


    /* 2: Read record-status, key-string & datafile-offset */
    /* from indexfile H->fd (keyrecord dwSlot) to struct stHkey */
    iRetCode = fread(&pstKeyRec->status, sizeof(pstKeyRec->status), 1, H->fd);
    ARET_ERR(iRetCode == 0, A_READ, 2502);

    iRetCode = fread(pstKeyRec->key, (int) wKsiz, 1, H->fd);
    ARET_ERR(iRetCode == 0, A_READ, 2503)
	iRetCode = fread(&pstKeyRec->dwDatOffset, sizeof(pstKeyRec->dwDatOffset), 1, H->fd);
    ARET_ERR(iRetCode == 0, A_READ, 2504);


    STCK("eReadIdxKey");
    ARET_OK

}   /* END function eReadIdxKey() */



/****************************************************************************/
/********************************** UTILITY *********************************/
/****************************************************************************/

/* Global "dwSeed" value for random number calculator */
static DWORD dwSeed = 53600520L;  /* Default initialization, any number will do */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   26               vGetPrime                                      */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vGetPrime(pdwPrime, iUpward)
    DWORD    *pdwPrime;    /* Initial & final value for prime search */
    int       iUpward;     /* Search direction: TRUE=up, FALSE=down */
{
/* DESCRIPTION
 *    Search for nearest prime number; Parameter "*pdwPrime" is the initial value,
 *    and parameter "iUpward" is used to specify the search direction :
 *       if iUpward==TRUE we scan "up" (higher numbers), else we scan "down".
 *    The function calls fIsPrime() to perform a probabilistic primality test.
 * RETURN
 *    The first prime number found (starting from *pdwPrime) is returned in *pdwPrime.
 *-4*/

    D(static char m0[] =   "%10ld\b\b\b\b\b\b\b\b\b\b";)
    D(static char m1[] =   "%10ld prime!\n";)
    FLAG fGotPrime     =   FALSE;


    /* 1: Get initial (ODD) number to start downward search for prime */
    /* & initialize random seed, used by rRandom() called from fIsPrime() */
    if (ISEVEN(*pdwPrime))
	*pdwPrime += (iUpward ? 1 : -1);
    dwSeed = 571925350L + *pdwPrime;    /* any random number will do */

    /* 2: Test initial number, - and successively smaller/greater (odd) numbers */
    do {
	/* Test *pdwPrime for primality */
	fGotPrime = fIsPrime(*pdwPrime);

	/* If not prime, proceed with next odd number > *pdwPrime or < *pdwPrime */
	if (!fGotPrime)
	    *pdwPrime += (iUpward ? 2 : -2);

	/* Report progress on stdout */
	D(fprintf(stdout, (!fGotPrime ? m0 : m1), *pdwPrime));

    } while (!fGotPrime);

}   /* END function vGetPrime() */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   27               fIsPrime                                       */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE   FLAG
fIsPrime(n)
    DWORD n;
{
/* DESCRIPTION
 *    Probabilistic test of "primality" : is number n prime ? (Y/N)
 *    References : Knuth [1980]
 *
 *                      Pierre de Fermat's theorem (1643) :
 *    Given an odd number : n = 1 + 2**k * q,   where q is odd
 *    If n is NOT prime, it is always possible to find a value : 1 < x < n
 *    such that :     x ** (n-1) mod n != 1
 *    If n IS prime, then the following expression :
 *                    x ** (n-1) mod n,   where	1 < x < n
 *    will yield 1 with probability p >= 3/4.
 *
 *                      Knuth's probabilistic primality test.
 *    The expression: x ** (n-1) mod n,   where	1 < x < n
 *                =   x ** ((2**k) * q) mod n
 *    can be calculated in  O(log n)  steps :
 *                    x ** ((2**j) * q) mod n,    where   j = 0,1, ... n-1
 *    If this procedure is repeated t times, and every time yields 1 as result,
 *    then n can be concluded to be prime with probability p = (1/4) ** t.
 *    For t=25 we have p = (1/4) ** 25 < 1E-15 ( which is less probable than a
 *    hardware failure ! ).
 *
 * RETURN
 *    TRUE if n is prime, FALSE otherwise.
 *-4*/
    DWORD     p = 0L;
    double    x = 0L;
    double    y = 0L;
    WORD      wCount = 0;      /* scratch count variable */
    WORD      wTests = 25;     /* # times to perform primality test */


    for (wCount = 1; wCount < wTests; wCount++) {
	x = 2 + (unsigned long) ((float) (n - 2) * (float) rRandom());
	y = 1;
	p = n - 1;

	do {
	    if (!ISEVEN(p))    /* if p is even : skip 	 */
		y = fmod((y * x), (double) n);

	    x = fmod((x * x), (double) n);	/* x =  (x ** 2) % n */

	    p = p / 2;

	} while (p > (DWORD)0L);

	if (y != 1)	       /* OOPS : definitely NOT prime! */
	    break;
    }

    return (y == 1 ? TRUE : FALSE);

}   /* END function fIsPrime() */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   28               rRandom                                        */
/*-- SYNOPSIS --------------------------------------------------------------*/
/*+0 Define the "magic numbers" for PMMLCG random number generator */
PRIVATE DWORD a = 16807;       /* multiplier : a long [2...m-1] */
PRIVATE DWORD m = 2147483647;  /* modulus .. : a large prime */
PRIVATE DWORD q = 127773;      /* quotient . : (m div a) */
PRIVATE DWORD r = 2836;        /* rest ..... : (m mod a) */

/*-0*/
PRIVATE double
rRandom(VOID)
{			
/* DESCRIPTION
 *    Prime Modulus Multiplicative Linear Congruential Generator (PMMLCG),
 *    using Lehmers algorithm implemented by Schrage's method.
 *    References : Knuth [1980], Park et.al. [1988]
 *
 *                      D. H. Lehmer's algorithm.
 *    The generating function :   f(X) = a*X mod m
 *    gives a repeated sequence (cycle, period) of statistically independent
 *    random numbers, with appropriate choice of :
 *    modulus  m : should be large, to permit a LONG period
 *                 should be prime, to permit a FULL period (cf multiplier a)
 *    multipl. a : should be a "primitive root" of prime m to permit a FULL
 *                 period, ie. :   a**(m-1) mod m = 1 (Fermats theorem).
 *    The following choice of parameters passes tests of random distribution
 *    and permits implementation with 32-bit arithmetic (C "long" data type) :
 *                    m   =   (2**31 - 1) =   2147483647
 *                    a   =   (7**5)      =   16807
 *                    X0  =   initial seed in the range [ 1 ... (m-1) ]
 *    The PMMLCG with these parameters is recommended as a minimal standard
 *    for a reliable and portable random number generator (see references).
 *
 *                      Schrage's Method.
 *    An algorithm, that evaluates expressions in the PMMLCG in such a way,
 *    that all intermediate results are bounded by (m-1).
 *    Given :     f(X)    =   a*X mod m
 *                        =   a*X - m ( a*X div m )
 *    we can use an "approximate" factoring of m : 	m = aq + r
 *    with the quotient q = (m div a) and the rest r = (m mod a)
 *    to transcribe f(X) to :
 *                f(X)    =   a(X mod q) -  r(X div q) +
 *                            (X div q) - (a*X div m)
 *                        =   y(X) + z(X)
 *    where :     y(X) > 0  =>  z(X) = 0
 *                y(X) = 0  =>  z(X) = m
 *
 * RETURN
 *    A (pseudo-)random float value, normalized to the range [0 - 0.999...]
 *-4*/

    /* Assume a "globally defined X-value : static DWORD dwSeed */
    long      hi = 0L;         /* Scratch variables */
    long      lo = 0L;
    long      yx = 0L;

    /* 1: Calculate Lehmer's expression : f(X) = a*X mod m */
    /* (using Schrage's method to eliminate potential overflow by a*X) */
    lo = dwSeed % q;           /* (X mod q) */
    hi = dwSeed / q;           /* (X div q) */
    yx = (a * lo) - (r * hi);  /* a * (X mod q) - r * (X div q) */
    /* always in the range [0...m-1] */

    /* 2: Reset seed for next call, and return (pseudo-)random value */
    dwSeed = (yx > 0 ? yx : yx + m);               /* always in range [0...2m-1] */
    return (double)((float) dwSeed / (float) m);   /* normalize to range [1/m...1-1/m] */

}   /* END function rRandom() */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   29               wCCITTcrc                                      */
/*-- SYNOPSIS --------------------------------------------------------------*/
#define POLY   0x8408
#define CRCOK  0x470F
#define PRESET 0xFFFF

PRIVATE   WORD
wCCITTcrc(pzData, wLength)
    char     *pzData;      /* Address of start of data block  */
    WORD      wLength;     /* Length of data block (in bytes) */
{
/* DESCRIPTION
 *    This routine generates the 16-bit remainder of a block of data, using
 *    the 16-bit CCITT polynomial generator. The basic idea is to treat the
 *    entire message as a (rather long) binary number; the crc checksum is
 *    then obtained by taking the one's complement of the remainder after the
 *    modulo 2 division by a generator polynomial.
 *                           16   12   5
 *    The CCITT-CRC uses: ( X  + X  + X + 1 ) for the generator polynomial.
 *    This may also be expressed as :  position   16   12        5     0
 *     - in binary :                               1 0001 0000 0010 0001
 *     - in hex    :                               1        10        21
 *    In computing the crc, a 17-bit dataregister is simulated by testing the
 *    MSB before shifting the data. This affords us the luxury of specifying
 *    the polynomial as a 16-bit value : 0x1021.
 *    The crc is generated in "LSB->MSB" order, so the bits of the polynomial
 *    are also stored in reverse order : 0x8408.
 * RETURN
 *    16-bit CCITT-reverse of block pointed to by pzData, length wLength byte.
 *-4*/
    WORD      wData = 0;   /* One byte of data */
    WORD      wCrc = 0;    /* Accumulator for 16-bit CRC-value */
    BYTE      bBit = 0;    /* Counter for bits in a byte */


    /* 1: "Preset" crc to -1 (=binary all 1's), and check wLength > 0 */
    wCrc = PRESET;	       /* Avoid problem of leading zero'es in data */
    if (wLength <= (WORD)0)
	return (~wCrc);


    /* 2: For each BYTE (8 bit) in the data block ... */
    do {

	/* For each bit in the data BYTE ... */
	for (bBit = 0, wData = (WORD) 0xFF & *pzData++;
	     bBit < 8; bBit++, wData >>= 1) {

	    /* (LSBit of crc) XOR (LSBit of data */
	    if ((wCrc & 0x0001) ^ (wData & 0x0001))
		wCrc = (wCrc >> 1) ^ POLY;	/* (crc/2) XOR polynomial */
	    else
		wCrc >>= 1;     /* (crc/2) */
	}

    } while (--wLength);


    /* 3: Do 1's complement and swap bytes in final crc */
    /* If a crc is itself included in the calculation, the valid values are: */
    /* Final crc: 0xF0B8, After complement: 0x0F47, After byte swap: 0x470F */
    wCrc = ~wCrc;
    wData = wCrc;
    wCrc = (wCrc << 8) | (wData >> 8 & 0xFF);

    return (wCrc);

}   /* END function wCCITTcrc() */



/*+4 MODULE HASH.C ---------------------------------------------------------*/
/*   NAME   30               pzStrcpyAlloc                                  */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE char *
pzStrcpyAlloc(ppzDest, pzSrc)
    char    **ppzDest;     /* Pointer to destination-string */
    char     *pzSrc;       /* Source-string */
{
/* DESCRIPTION
 *    Transfer a string to dynamic (heap-)space.
 *    1.  Allocate a new destination-string of size strlen(source-string),
 *    2.  Copy source-string to destination-string (on the heap).
 * RETURN
 *    Address of destination-string.
 *    NB: Memory allocated by pzStrcpyAlloc must be free'd after use.
 *-4 */

    /* 1: Allocate destination string */
    *ppzDest = (char *) malloc(strlen(pzSrc) + 1);


    /* 2: Copy source-string to destination-string */
    if (*ppzDest != NULL)
	(void) strcpy(*ppzDest, pzSrc);

    /* 3: Return address of destination-string (NULL if bad alloc) */
    return (*ppzDest);

}   /* END function pzStrcpyAlloc() */



/* END module hash.c                                                        */
/*==========================================================================*/
