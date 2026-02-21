/*+1========================================================================*/
/*  MODULE                        LCK.C                                     */
/*==========================================================================*/
/*  FUNCTION       This module implements a UNIX/MSDOS portable interface for
 *                 file sharing and record locking, based on the X/Open XPG3
 *                 (POSIX.1) definition of file control functions.
 *
 *                 In a UNIX environment the module's portable functions map
 *                 directly to the X/Open equivalents (open and fcntl), while
 *                 in a MSDOS environment the portable functions call on com-
 *                 piler specific procedures (MSC sopen, locking) to "imitate"
 *                 the X/Open functionality.
 *
 *                 The module offers a "HighLevel" API for portable WRITE-
 *                 locks : iPLock(), and a "VeryHighLevel" API for portable
 *                 semaphore operations : iPSem().
 *
 *  SYSTEM         !NB: NOT Standard C, - uses compiler specific routines NB!
 *                 Portable across UN*X (POSIX) and MSDOS (MS C compiler) :
 *                 - UNIX SVR3&4 (DG CC using standard POSIX system interface)
 *                 - PC/MSDOS V33&50 (MSC60A using MS specific ANSI C extensions)
 *
 *  SEE ALSO       Modules: LCK.H, MAKEFILE
 *
 *  PROGRAMMER     Allan Dystrup, DIVDEC/AD
 *
 *  COPYRIGHT      (c) Allan Dystrup, Kommunedata I/S November 1991.
 *
 *  VERSION        $Header: d:/cwork/lck/RCS/lck.c 0.1 92/11/10 16:37:29
 *                 Allan_Dystrup PREREL Locker: Allan_Dystrup $
 *                 -----------------------------------------------------------
 *                 $Log:	lck.c $
 *                 Revision 0.1  92/11/10  16:37:29  Allan_Dystrup
 *                 PREREL (ALFA1)
 *
 *  REFERENCES     X/Open Company Ltd. [1989] : X/Open Portability Guide
 *                    Issue 3 (XPG3), XSI System Interface and Headers
 *                    Prentice Hall, Englewood Cliffs, New Jersey 07632
 *                 Microsoft Corp. [1990] : Microsoft C Reference
 *                    for MS OS/2 and MS-DOS Operating Systems.
 *                    Microsoft Corporation.
 *                 IBM Corp. [1984] : PC Hardware Reference Library
 *                    Technical Reference, 6361459
 *                    International Business Machines Corp.
 *
 *  USAGE          Module LCK.C features the following public routines for
 *                 file sharing and record locking under MSDOS and UNIX.
 *                 See the headerfile LCK.H for a detailed description of
 *                 the user accessible datastructures and interface functions.
 *                     POPEN()         // Open a file for sharing & locking
 *                     PFCNTL          // LowLevel locking operation on file
 *                     iPLock()        // HighLevel locking operation on file
 *                     iPSem()         // VeryHiglLevel semaphore operations
 *
 *  DOC            Documentation is incorporated into the module and may be
 *                 extracted (using a utility such as ex.awk) :
 *                    Level 0 : Module doc. (detailed technical research)
 *                    Level 1 : Module doc. (history, design, testdriver)
 *                    Level 2 : PUBLIC functions (module program interface, "API")
 *                    Level 3 : major PRIVATE functions (design)
 *                    Level 4 : minor PRIVATE functions (support)
 *
 *  BUGS           The purpose of this module is to provide a DOS/UNIX system
 *                 independent interface to file sharing and record locking.
 *                 This can only be achieved using SYSTEM SPECIFIC FUNCTIONS
 *                 so the PORTABILITY of the module is LIMITED to :
 *                    MSDOS >= Version 3, and MS C 6.0 Compiler
 *                    UNIX SV >= R3, and X/Open XPG3 compliant C Compiler Lib.
 *                 The module will not compile in a strict standard C (ANSI)
 *                 environment!!!
 *
 *+0============================= BACKGROUND =================================
 *
 *                      THE PROBLEM : MUTUAL EXCLUSION
 * There are occations when multiple concurrent processes want to share a
 * resource.  It is essential, that some form of mutual exclusion is enforced,
 * so that only one process at a time can access the resource.
 * Consider the classical Writers-problem of more users updating a record-
 * structured sequential file; - Each user processes must be able to :
 *    1 read any record
 *    2 update the record
 *    3 write back the record
 * The problem stems from the following "race condition" : in the time it
 * takes for a single process to execute the three steps, another process may
 * start performing the same three steps on the same record, - chaos will
 * result!
 *
 *
 *                      THE SOLUTION : LOCKING
 * What we need is a synchronization mechanism to prevent the processes from
 * interfering with one another : a process should be able to set a LOCK on
 * an object, so that no other process can access this same object until the
 * first process is done and releases its LOCK. A program section accessing
 * a shared object is called a "critical section", and the locking mechanism
 * must ensure that conflicting critical sections do not overlap, thereby
 * guaranteing mutual exclusive access to the shared object.
 *
 *
 *                      LOCKING : RANGE
 * The "resolution" of the locking function may be coarse (file) or fine grained
 * (byte range).
 *    FILE   LOCKING (F) : locks an entire file (eg UNIX rwx--x--x permission)
 *    RECORD LOCKING (R) : locks only a specified part ("record") of the file.
 *
 *
 *                      LOCKING : STRICTNESS
 * Operating systems may impose loose (advisory) or strong (mandatory) locking
 * control.
 *    ADVISORY LOCKING (A): the operating system maintains full knowledge of
 *    which files have been locked by which process, but it does not prevent
 *    a process from writing to a file locked by another process. This type
 *    of locking may be used by a group of cooperating processes.
 *
 *    MANDATORY LOCKING (M): The operating system checks every read and write
 *    request, to verify that the operation does not interfere with a lock
 *    held by any process. This strict kind of locking must be imposed, if
 *    the processes accessing the shared ressource can not be controlled by
 *    the system designer in advance.
 *
 * ------------------------------ DETAILS ------------------------------------
 *
 *                      LOCKING : SYSTEM IMPLEMENTATION
 * The LOCK-function is composed of a test- and set-operation ("IF record locked
 * THEN not-available ELSE lock") and these two atomic parts must be executed
 * as ONE consecutive operation. Only the operating system has complete control
 * over process scheduling, so only a system call can enforce an indivisible
 * test-&-set-LOCK function. Modern operating system normally offer a DEDICATED
 * system call for the locking operation :
 *
 *    UNIX : Berkley 4.3BSD : A  , F  : int flock(int fd, int c);
 *           System  VR3... : A  , F&R: int fcntl(int fd, int cmd, ...)
 *
 *    MSDOS: DOS >=  v 3.1  : M  , F&R: The MSDOS system interface is simply
 *           a series of (at times undocumented!) software interrupts, and the
 *           handling of exclusive file access is a rather complex "cludge"
 *           evolved by combining some of these interrupt procedures :
 *
 *           1 SYSTEM CALL API (INT 20H-27H)
 *             The SYSTEM CALL interface to MSDOS is through SW INT 20H-27H
 *             (with INT 30H-3FH reserved for "future use").
 *
 *           2 FUNCTION REQUEST CALL (INT 21H)
 *             INT 21H is the general MSDOS FUNCTION REQUEST interrupt, with
 *             subfunctions 00H-62H (in register AH) providing a series of OS
 *             services for device I/O, management of files, memory & processes.
 *             Of special interest for file access operations are the sub-func.:
 *             2.1 DOS INT 3DH Open_a_file(char *fname, int faccess)
 *                 The low level file-open system function specifying file
 *                 access(A) & sharing(S) mode. Using S=100 ("deny-none")
 *                 will defer control of concurrent access to the locking
 *                 operation (INT 5CH)
 *             2.2 DOS INT 5CH Lock/Unlock_File_Access(cmd,fd,offset,range)
 *                 The dedicated system function for (un-)locking a range of
 *                 bytes in a file
 *
 *           3 GENERAL MULTIPLEXING & REDIRECTING (INT 2FH)
 *             INT 2FH (originally a vector for communication between DOS and
 *             PRINT.COM) has evolved to a general MSDOS MULTIPLEX interrupt
 *             providing a place to "hook in" any TSR IPC service; Multiplex
 *             subfunctions 00H-7FH (in reg. AH) are reserved for DOS, while
 *             80H-FFH are available for application use.
 *             Starting with MSDOS 3.0, subfunctions 11xx were introduced as
 *             a general REDIRECTOR INTERFACE : many MSDOS FUNCTION-REQUESTS
 *             (INT 21H) will be redirected through INT 2FH,11xx for possible
 *             interception & modification by application programs :
 *             Ex : DOS OPEN   (21H-3D<MD>H) -> MULTIPLEX OPEN   (2FH-1116H)
 *                  DOS LOCK   (21H-5C00H)   -> MULTIPLEX LOCK   (2FH-110AH)
 *                  DOS UNLOCK (21H-5C01H)   -> MULTIPLEX UNLOCK (2FH-110BH)
 *             This design provides a stream-level (ie. FCB vs. File-handle
 *             independent) access to device I/O for application programs,
 *             and a clean interface for implementing "virtual devices" (com-
 *             pared to hooking into INT 21H or writing a new device driver).
 *
 *           4 SHARING (SHARE.EXE)
 *             MSDOS >= 3.1 provides the program SHARE.EXE for basic mutual
 *             exclusion extensions to DOS. The program loads as a TSR, and
 *             installs itself in the chain of INT 2FH handlers (if the call
 *             is not for a subfunction 10xx, control is passed to the next
 *             handler in the chain). In this way SHARE can monitor all file
 *             access and arbitrate filesharing & -locking in a multitasking
 *             or/and networked environment. Together with a network redirector
 *             (see below) SHARE can provide mutual exclusive access to remote
 *             files on a LAN.
 *
 *             DOS INT 21(44H) is the general MSDOS low level device-IOCTL;
 *             Subfunction 0BH (reg. AL) was introduced with v. 3.0 as a
 *             means for resetting ("tuning") the retry count (default 3) and
 *             delay loop count (default 1) in case of share/lock conflicts.
 *
 *           5 VIRTUAL DEVICES (IFS)
 *             The INT 2FH (11xxH) redirector interface of the multiplex INT
 *             has been used to implement a number of INSTALLABLE FILE SYSTEMS
 *             (IFS's) as for instance :
 *              - MSCDEX.EXE (Microsoft CD-ROM Extension program
 *              - IFSFUNC.EXE (MSDOS 4.0 for support of disks > 32MB)
 *              - REDIRECTOR (network redirector for NetBIOS based LANs)
 *                The redirector hooks into INT 2FH and traps 11xxH-calls for
 *                mapping logical drives to remote physical disks; The mapping
 *                is effected by one or more TSR/Device Driver modules, by :
 *                 - packing OS system calls (device/file requests) into Server-
 *                   Message-Blocks (SMB), which are passed by INT 2AH to a
 *                 - Session Level module, where they each are packed into a
 *                   Network-Control-Block (NCB) and send by INT 5CH to the
 *                 - Transport Level module (NetBIOS) for further Data Link
 *                   expedition by "virtual circuit" to a remote server.
 *                   Common DOS redirectors ("INT 2FH,11xxH hook" TSR's) are :
 *                    - REDIR.EXE (IBM PC LAN)
 *                    - MSREDIR.EXE (MS NET)
 *                    - WRKSTA.EXE (MSLM >=2.0 IBMLS >=1.2)
 *                   The newest DOS/OS2 redirectors (WRKSTA.EXE) include support
 *                   for named pipes & mail slots.
 *
 *           6 PEER-TO-PEER SESSION (NetBIOS)
 *             The actual SESSION-LEVEL communication between the two OS'es
 *             is done by the peer-to-peer NetBIOS protocol stack (TSR's):
 *                Application -INT21H->             MSDOS FUNCTION REQUEST
 *                DOS         -INT2FH(11xxH)->      MSDOS MULTIPLEX CHAIN
 *                SHARE       -INT2FH(10xxH)->      (1) MUTUAL EXCLUSION
 *                REDIRECTOR  -INT2AH-(SMB)->       (2) NET REDIRECTOR
 *                NET         -INT5CH-(NCB)->       SESSION:REC/SRV/MES
 *                NetBIOS     -INT5CH-(CCB)->       TRANSPORT:DATAGR/VIR.CIRC.
 *                DataLink    ... etc               LLC/MAC/PHYSICAL LAYERS
 *
 *    OS/2 : USHORT DosFileLocks(HFILE hf, PFILELOCK pfUnLock, PFILELOCK pfLock);
 *           Will unlock and/or lock region(s) in a file specified by the
 *           pointer(s) to structure(s) : pfUnLock and pfLock (optionally NULL).
 *           Unlock is performed before lock, and OS/2 supports overlapping
 *           locks, BUT ONLY EXCLUSIVE (NOT SHARED) LOCKS.
 *
 * An alternative to using a dedicated system call for implementing the mutual
 * exclusion mechanism is to rely on a FILE SYSTEM function for providing
 * the exclusive access; This practice of using a dummy file as a "semaphore"
 * has been a common practice in (early) UNIX versions. The locking may be
 * based on one of the following file system calls :
 *  - link() : fails if the link name already exists
 *  - creat(): fails if the file exists & the caller lacks Write-permission
 *  - open() : fails if the file exists (flag O_CREAT & O_EXCL, 4.3BSD & SysV)
 *
 *
 *                      LOCKING : C IMPLEMENTATION
 * The C language library normally offers a high level API to the "raw" opera-
 * ting system call interface. For sharing and locking operations we may use :
 *
 *    UNIX : SysVR3 C : int lockf(int fd, int cmd, long size);
 *
 *    MSDOS: The sopen() function of MS/BORLAND C provides a high-level API to
 *           the DOS INT 21H-3DxxH Open_a_file, while the lock(ing)/unlock
 *           functions is the C API to DOS INT 21H-5CxxH Lock/Unlock file acc.
 *           MSC 6.00 : int sopen(char *file, int oflag, int shflag, int pmode);
 *                      int locking(int fd, int cmd, long size);
 *           BORL.TC++: int sopen(char *path, int access, int shflag, int mode);
 *                      int lock(int handle, long offset, long length);
 *                      int unlock(int handle, long offset, long length);
 *           LOW-LEVEL
 *           Most DOS C compilers provide a more direct interface to the
 *           operating system calls, to ensure availability of the full OS
 *           functionality; for MSC60 and BTC++ we may thus use :
 *              int intdos(union REGS *inregs, union REGS *outregs);
 *              with variants : intdosx, int86, int86x
 *           For the commonly used system calls, MSC60 furthermore offers a
 *           series of predefined "INT macro's", among others :
 *              _dos_open(char*path, unsigned mode, int *handle);
 *
 *           OBS exclusive file access in MSDOS :
 *           File locking under DOS requires loading of the SHARE.EXE
 *           monitor/arbiter TSR module, and locking on remote files in a
 *           LAN requires a net redirector TSR/device-driver in place
 *           (for instance WRKSTA.EXE in MS LAN MANAGER).
 *           Before trying to execute the first share/lock-request, an
 *           application program should ensure the precense (in RAM) of
 *           these OS extensions!
 *
 *
 *                      LOCKING : PORTABILITY
 * The problem with both the operating system interface (of course) and the
 * C-library API is, that no standardization has been undertaken across the
 * different platforms; Even the (several!) international standardization
 * efforts have not (yet, as of 1992) resulted in a common API for locking :
 *    POSIX.1 ISO/IEEE 1003.1 and X/Open XPG3 System Interface (XSI):
 *       int fcntl(int fd, int cmd, ...);
 *       Adapted from UNIX System VR3 (SVID) advisory record locking
 *    ISO/IEC 9890 STD C :
 *       The standarc C I/O library does NOT include an API for file/record
 *       locking, - not even at the low level.
 *
 * ============================== DESIGN =====================================
 *
 *                      PORTABLE LOCKING : SOLUTIONS
 * To implement a portable locking mechanism we may choose one of two methods:
 *
 *   1  Implement our own "semaphore" datatype based on the file system-call's
 *      mutual exclusion property :
 *      - for LOCK ("semaphore raise", ie. create semaphore file)
 *           UNIX : fd = open(path, O_WRONLY | O_CREAT, 0);
 *           MSDOS: fd = open(path, O_WRONLY | O_CREAT | S_IREAD | S_IWRITE);
 *      - for UNLOCK ("semaphore lower", ie. remove semaphore file)
 *           UNIX & MSDOS : unlink(path);
 *
 *   2  Base our implementation on the POSIX-X/Open System interface for low
 *      level file control, including LOCK/UNLOCK operations. This system call
 *      is available in all POSIX-X/Open conforming UNIX implementations (which
 *      is a requirement for UNIX OS'es supported by KMD) :
 *         UNIX : int fcntl(int fd, int cmd, ...);
 *      and the same interface can be simulated on MSDOS by "wrapping" any
 *      existing compiler-specific C-function for locking into a simple macro
 *      mimicrying the POSIX API :
 *         MSDOS/MSC6.0 : int locking(int fd, int cmd, long size);
 *
 *
 *                      PORTABLE LOCKING : CHOICE
 * Solution 2 is less general (for instance it will not port to BSD), but it
 * is easier and more safe to implement (detection of potential deadlock,
 * robustness in a distributed environment).
 * Furthermore solution 2 is fully adequate for the KMD system arcitecture
 * (we don't need to support ALL UNIX versions!), so we choose this option
 * for our implementation.
 *
 *
 * ========================== IMPLEMENTATION =================================
 *
 *                      POSIX-X/Open XSI C INTERFACE
 *
 * int fcntl(int fildescriptor, int command, struct flock argument);
 *
 * command (relevant for locking) :
 *    F_SETLK      Set a record lock, failing if the lock is not available :
 *                 Set or clear a file segment lock according to the lock
 *                 description pointed to by argument.
 *                 The argument's l_type may be : F_RDLCK, F_WRLCK or F_UNLCK.
 *                 If a shared or exclusive lock is blocked by other locks
 *                 fcntl will return -1
 *    F_SETLKW     Set a record lock, pending (blocking) if necessary :
 *                 Same as F_SETLK except that if a shared or exclusive
 *                 lock is blocked by other locks, fcntl will wait until
 *                 the request can be satisfied (or a SW INT is received,
 *                 causing return -1 with errno=EINTR).
 *    F_GETLK      Test whether existing record locks would cause an attempt
 *                 to set a particular lock to fail : Get first lock which
 *                 would blocks the lock description pointed to by argument.
 *                 If no blocking lock, leave argument unchanged, else set
 *                 l_type to F_UNLCK and l_pid to process ID of the lock-owner.
 *
 * argument (for locking operations) :
 *    struct flock {
 *       short l_type;     - Type of lock.
 *                           F_RDLCK   set a read (shared) lock
 *                           F_WRLCK   set an write (exclusive) lock
 *                           F_UNLCK   remove either type of lock
 *       short l_whence;   - From which direction to measure l_start :
 *                           SEEK_SET  from start of file
 *                           SEEK_CUR  from current position
 *                           SEEK_END  from end of file
 *       off_t l_start;    - Offset in file for start of lock
 *                           counted relative to l_whence
 *       off_t l_len;      - Length of lock region, possibly 0
 *                           number of consecutive bytes to lock
 *                           a Zero (0) value will lock to end-of-file
 *       pid_t l_pid;      - Process ID for owner of lock, set by F_GETLK
 *                           identifying process with a blocking lock
 *       short  l_sysid;   - DG System identifier
 *       short  l_pad1;    - DG Unused padding;
 *                           set to zero when returned by the kernel.
 *    };
 *
 *
 *                      MICROSOFT C 6.0 INTERFACE
 *
 * int locking(int fildescriptor, int command, long nbytes);
 *
 * command :
 *     Failing (non-blocking) locks : If the file range cannot be locked,
 *     locking returns -1 immediately.
 *     LK_NBLCK       set a non-blocking read (shared) lock
 *     LK_NBRLCK      set a non-blocking write (exclusive) lock
 *
 *     Pending (blocking) locks : If the file range connot be locked,
 *     locking tries again after 1 second; If after 10 attempts the bytes
 *     cannot be locked, locking returns -1.
 *     LK_LOCK        set a pending read (shared) lock
 *     LK_RLCK        set a pending write (exclusive) lock
 *
 *     LK_UNLCK       remove either type of lock
 *                    the bytes must have been locked in advance
 *
 * nbytes  :          Length of lock region
 *                    number of consecutive bytes to lock
 *
 * FORWARD REFERENCE: !NB In spite of what the MSC6.0 documentation postulates
 *                    THE MS C COMPILER DOES NOT SUPPORT READ/SHARED LOCKS -
 *                    cf. the following section : "TEST".
 *
 *-0
 *--------------------- PORTABLE POSIX/MSC FCNTL SUBSET ----------------------
 *
 * UNIX/POSIX-C                           MSDOS/MS-C
 *
 * struct flock {
 *    short l_type;                       cf. equivalences listed below
 *    short l_whence;                     > Requires lseek(fd,l_start,l_whence)
 *    off_t l_start;                      > before call of locking(...)
 *    off_t l_len;                        Equivalent to nbytes
 *    pid_t l_pid;                        Ignored
 *    short l_sysid;                      Totally ignored (DG UX extension)
 *    short l_pad1;                       Totally ignored (DG UX extension)
 *  };
 *
 * l_type equivalences :
 *    fcntl(fd, F_SETLK,  [F_RDLCK]);     locking(fd, LK_NBLCK,  l_len);
 *    fcntl(fd, F_SETLK,  [F_WRLCK]);     locking(fd, LK_NBRLCK, l_len);
 *    fcntl(fd, F_SETLKW, [F_RDLCK]);     locking(fd, LK_LOCK,   l_len);
 *    fcntl(fd, F_SETLKW, [F_WRLCK]);     locking(fd, LK_RLCK,   l_len);
 *    fcntl(fd, F_SETLK,  [F_UNLCK]);     locking(fd, LK_UNLCK,  l_len);
 *
 * The fcntl() functionality for locking operations can be provided under
 * MSDOS by setting up an interface function (eg. PFCNTL, Portable FCNTL)
 * translating the fcntl() call syntax to the locking() equivalents.
 * This will be called the "LowLevel API".
 *
 * For convenience I will furthermore provide a "HighLevel API" in the form
 * of a function iPLock(), that simply packs it's arguments into a flock-
 * structure and pases this structure as an argument to PFCNTL for execution
 * of the locking operation.
 *
 * -------------------- PORTABLE POSIX/MSC FCNTL API -------------------------
 *
 *     UNIX/POSIX-C              PORTABLE            MSDOS/MS-C
 *      (-DUNIX)                                     (-DMSDOS)
 *                                POPEN
 *                                  |
 *             +--------------------+--------------------+
 *             |                                         |
 *           open                                      DOPEN
 *                                                       |
 *                                                     vChkEnv
 *                                                     sopen
 *
 *  HighLevel API:                iPLock
 *                                  |
 *  LowLevel  API:                PFCNTL
 *                                  |
 *             +--------------------+--------------------+
 *             |                                         |
 *          fcntl                                      DFCNTL
 *                                                       |
 *                                                     lseek
 *                                                     locking
 *
 *+0============================= TEST =======================================
 *
 * The following table lists the results of a test-run on DOS and UNIX using
 * the defined "portable" interface for file sharing and record locking;
 *
 *   +----------------------------------------------+---------------------+
 *   | Lin Lock     Proc#  LockOperation  LockRange |   DOS      UNIX     |
 *   +----------------------------------------------+---------------------+
 *   | 01  READ       1    NREAD  lock     0 - 10   |   OK       OK       |(*)
 *   | 02             2    read   file       do     |   EBADF    OK       |
 *   | 03             2    write  file       do     |   EBADF    OK       |
 *   | 04  READ(ux)   2    NREAD  lock       do     |   EACCES   OK       |
 *   | 05             2    NWRITE lock       do     |   EACCES   EACCES   |(*)
 *   | 06             1    read   file       do     |   OK       OK       |(*)
 *   | 07             1    write  file       do     |   OK       OK       |(*)
 *   | 08  READ(ux)   1    NREAD  lock       do     |   EACCES   OK       |
 *   | 09             1    NWRITE lock       do     |   EACCES   EACCES   |(*)
 *   +----------------------------------------------+---------------------+---
 *   | 10  WRITE      1    NWRITE lock     0 - 10   |   OK       OK       | *
 *   | 11             2    read   file       do     |   EBADF    OK       |
 *   | 12             2    write  file       do     |   EBADF    OK       |
 *   | 13             2    NREAD  lock       do     |   EACCES   EACCES   |(*)
 *   | 14             2    NWRITE lock       do     |   EACCES   EACCES   | *
 *   | 15             1    read   file       do     |   OK       OK       | *
 *   | 16             1    write  file       do     |   OK       OK       | *
 *   | 17  READ(ux)   1    NREAD  lock       do     |   EACCES   OK       |
 *   | 18  WRITE(ux)  1    NWRITE lock       do     |   EACCES   OK       |
 *   +----------------------------------------------+---------------------+
 *
 * Observe the following characteristics and differences :
 *
 * 1. DOS record locking is MANDATORY, as opposed to UNIX ADVISORY locking :
 *     - DOS will BLOCK any read/write attempt on a locked file region
 *       by any other process than the user holding the lock.
 *     - UNIX does NOT VALIDATE read/write-requests to locked regions.
 *
 * 2. MSC60 record locking does not support shared/read-locks but only
 *    exclusive/write-locks whereas UNIX offers both types of locking :
 *     - MSC60 locking() function will not allow more than one lock
 *       on any file region, even by the same process (lines: 04,08,09).
 *       THIS DOES NOT AGREE WITH THE DOCUMENTATION OF THE MS C 6.0 LOCKING
 *       FUNCTION AND THE HEADERFILE \SYS\LOCKING.H, STATING THE PRECENCE OF
 *       BOTH READ AND WRITE LOCKS! - A closer inspection of DOS 21H-5CxxH
 *       "Lock/Unlock_File_Access" function reveals, that DOS itself in fact
 *       does NOT support read-locks at the byte level (only the share acces
 *       modes: deny read/write/both at the file level specified in open()).
 *     - UNIX does support read locks, including multiple read locks on the
 *       same file range, as well as substitution of a read with a write lock
 *       by the owner process, if no other process holds a lock on the region
 *       in question (lines: 04,08,17,18).
 *
 * 3. The subset of locking operations with a consistent behaviour across the
 *    DOS and UNIX platforms has been marked with asterix: * in the table;
 *    For a portable implementation, the READ-lock (ie. SHARED locking) option
 *    should not be used, since it is obviously not supported by the MSC60 
 *    compiler for DOS. I have put the asterix for these operations inside (*)
 *    in the table.
 *    What remains are the following WRITE-lock (EXCLUSIVE locking) operations,
 *    which we may safely use to implement mutual exclusive access to file
 *    regions in a multitasking multiuser environment :
 *
 *   +----------------------------------------------+---------------------+
 *   | Lin Lock     Proc#  LockOperation  LockRange |   DOS      UNIX     |
 *   +----------------------------------------------+---------------------+
 *   | 10  WRITE      1    NWRITE lock     0 - 10   |   OK       OK       | *
 *   | 14             2    NWRITE lock       do     |   EACCES   EACCES   | *
 *   | 15             1    read   file       do     |   OK       OK       | *
 *   | 16             1    write  file       do     |   OK       OK       | *
 *   +----------------------------------------------+---------------------+
 *
 *-0
 *=========================== FINAL WRAPPING =================================
 *
 * As clarified in the previous section ("TEST") a portable READ/SHARED-lock 
 * mechanism is not available across the DOS/UNIX platforms; This restriction 
 * introduces a new problem for all those applications that may tolerate a 
 * short-term write lock on records, but need multiple read access most of the
 *  time (in a UNIX environment locking is only ADVISORY, so any process may 
 * freely read a locked region, whereas DOS imposes a MANDATORY locking with 
 * strict I/O-checking and rejection of all access attempts on a locked record).
 *
 * To solve this problem, we will use our function iPLock and the "consistent
 * function subset" (cf. the TESTING section above) to implement a simple
 * semaphore mechanism for mutual exclusive access to record-structured files.
 * This access scheme will be based on the precence of a "lock-byte" in each
 * record;  the lock-byte may be set ("SEMUP"), cleared ("SEMDOWN") and tested
 * by multiple processes using the "VeryHighLevel" function : iPSem().
 *
 *     UNIX/POSIX-C              PORTABLE            MSDOS/MS-C
 *      (-DUNIX)                                     (-DMSDOS)
 *
 *  VeryHighLevel API:            iPSem
 *                                  |
 *  HighLevel API:                iPLock
 *                                  |
 *  LowLevel  API:                PFCNTL
 *                                  |
 *             +--------------------+--------------------+
 *             |                                         |
 *          fcntl                                      DFCNTL
 *                                                       |
 *                                                     lseek
 *                                                     locking
 *
 * The new VHLAPI function iPSem() calls on iPLock() to set up a "critical
 * section" around access to the lock-byte, thus in effect implementing the
 * basic indivisible sem.operations (SEMTEST, SEMUP, SEMDOWN) on the byte.
 * By including a lock-byte in each record and using iPSem() to change the
 * state of the lock-byte (SEMUP or SEMDOWN), the lock-byte will act as a mutual
 * exclusive flag (= a semaphore) telling whether the record-resource is
 * in current use for updating (semaphore:SEMUP) or free (semaphore:SEMDOWN).
 * NOTE that even if the state of the semaphore for a record is "SEMUP", the
 * record may still be safely read by other processes. A record guarded by a
 * locked semaphore (state "SEMUP") may also for that matter be directly updated
 * by other processes, if they fail to test or chose to ignore the semaprore
 * flag - THIS PRACTICE SHOULD BE AVOIDED HOWEVER, AS IT WILL LEAD TO UNPRE-
 * DICTABLE RESULTS!
 *
 *
 *+0==========================================================================
 *
 * Outstanding questions :
 *  -  test NFS functionality (UNIX remote drive!)
 *
 *-0
 *
 *-1========================================================================*/



/*==========================================================================*/
/*                           INCLUDE FILES                                  */
/*==========================================================================*/

/* Standard C headers */
#include <signal.h>
#include <string.h>


/* Module header */
#define _LCK_ALLOC
#include "lck.h"



/*==========================================================================*/
/*                      DEFINE'S & FUNCTION PROTOTYPES                      */
/*==========================================================================*/
#define    STOP    FALSE       /* Possible actions after error check */
#define    CONT    TRUE

#define    ERROR   -1          /* Symbolic const for DOS func return value. */
#define    OK       0
#define    ICLAMP(lvar) (lvar >= 0L ? OK : ERROR)  /* Conv lseek.pos to int */

#define    SEMRD    0          /* Semaphore I/O operations */
#define    SEMWR    1


PRIVATE int
          iSemIO   P((int iFd, long lPos, int iOp, char *pcSem));


#ifdef MSDOS
PRIVATE int
          iChkEnv  P((void));
#endif



#ifdef MAIN
/****************************************************************************/
/**************************** MAIN TESTDRIVER *******************************/
/****************************************************************************/


/*--------------------------------------------------------------------------*/
/*                      main() error# & messages                            */
/*--------------------------------------------------------------------------*/

/* DOS file operations -------------------------------------------- */
#define    AOPEN     0                 /* 00: File open */
#define    ASEEK     1                 /* 01: File seek */
#define    AREAD     2                 /* 02: File read */
#define    ALOCK     3                 /* 03: File lock */
#define    AWRIT     4                 /* 04: File read */
#define    AFILE     5                 /* 05: File operation (generic) */
/* iChkEnv error codes -------------------------------------------- */
#define    ABCHK   (11)                /* Base offset */
#define    ANBLD   (-5)                /* 06: Net-BIOS Loa-Ded */
#define    ANWLD   (-4)                /* 07: Net-Work Loa-Ded */
#define    ASHLD   (-3)                /* 08: SH-are Loa-Ded */
#define    ASHEX   (-2)                /* 09: SH-are EX-ists */
/* iPSem error codes ---------------------------------------------- */
#define    ABSEM   (15)                /* Base offset */
#define    ASMIV   (-5)                /* 10: Se-Maphore InValid */
#define    ASMUS   (-4)                /* 11: Se-Maphore in USe (SEMUP) */
#define    ASMFR   (-3)                /* 12: Se-Maphore FRee (SEMDOUN) */
#define    ASMIN   (-2)                /* 13: Se-Maphore err in INpot */


/* Define error messages, cf. associated error-ID's above */
PRIVATE char *rgpchErrmsg[] = {
    /* DOS file operations --------------------------------- BASE 00 */
    "Fejl[%d] ved †bning af fil[%s] - Findes filen?\n",        /* 00: AOPEN, errno, filename */
    "Fejl[%d] ved positionering ('seek') i fil\n",             /* 01: ASEEK, errno */
    "Fejl[%d] ved l‘sning fra fil\n",                          /* 02: AREAD, errno */
    "Fejl ved l†se-operation p† fil\n",                        /* 03: ALOCK */
    "Fejl[%d] ved skrivning til fil\n",                        /* 04: AWRIT, errno */
    "Fejl[%d] ved fil operation\n",                            /* 05: AFILE, errno */
    /* iChkEnv --------------------------------------------- BASE 11: ABCHK */
    "Ingen NetBIOS interface - udf›r netinstallation!\n",      /* 06: ANBLD */
    "Ingen netdrev synlige - udf›r netops‘tning!\n",           /* 07: ANWLD */
    "Ingen support for fildeling/l†sning - udf›r SHARE.EXE!\n",/* 08: ASHLD */
    "DOS version < 3.0 , - ikke support for flerbruger!\n",    /* 09: ASHEX */
    /* iPSem ----------------------------------------------- BASE 14: ABSEM */
    "Fejl: SEMUP/SEMDOWN p† ikke-semaphor (invalid byte)\n",   /* 10: ASMIV */
    "Fejl: SEMUP p† hejst semaphor - Record allerede l†st!\n", /* 11: ASMUS */
    "Fejl: SEMDOWN p† fri semaphor - Record allerede fri!\n",  /* 12: ASMFR */
    "Fejl i inddata til semaphor funktion: iPSem()\n"          /* 13: ASMIN */
};


/* Define logon-message */
PRIVATE char SIGNON[] =
    "\nKMD Portable Locking Functions (Testdriver), Version 0.1\n"
    "MOD[LCK.C] VER[0.1.0 Exp] DAT[92/08/31] DEV[ad divdec]\n"
    "Copyright (c) Allan Dystrup 1992\n\n";


/* Declare a global-scope file descriptor (visible for signal catcher) */
PRIVATE int  iFd;          /* File descriptor for testfile */



/*--------------------------------------------------------------------------*/
/*                      main support function prototypes                    */
/*--------------------------------------------------------------------------*/
PRIVATE void
          vSigCatch    P((int iSigNum));

PRIVATE void
          vChkErr      P((int cond, int cont, int iLine, int err, ...));

PRIVATE void
          vChkEno      P((int iRetCode));


#define   BUFLEN  1024L    /* Length of record buffer */



/*+1 MODULE LCK.C ==========================================================*/
/*   NAME                         main                                      */
/*== SYNOPSIS ==============================================================*/
PRIVATE void
main(argc, argv, envp)
    int       argc;        /* Argument count */
    char     *argv[];      /* Argument vector */
    char     *envp[];      /* Environment pointer */

{
/* DESCRIPTION
 *    Demoprogram and testdriver for module "lck.c".
 *    The function demonstrates the proper use of datastructures and
 *    public functions declared in lck.h and defined in lck.c.
 *
 *    0: Signon & setup signal catcher.
 *    1: Open testfile (for update/sharing).
 *    2: LOOP  //Test file locking, - execute lck from multiple terminals!
 *          2.1: Prompt for input
 *          2.2: Get input string from user/stdin, format: <CODE START LEN>
 *          2.3: Call iPLock() or iPSem(), with func. depending on entered CODE
 *       UNTIL ( CODE=Quit )
 *
 * RETURN
 *    main() returns :
 *     - EXIT_SUCCESS on normal termination,
 *     - EXIT_FAILURE on fatal error.
 *    (However main() is a testdriver and not intended to interface with any
 *    calling program, so the return value is insignificant in this context).
 *
 * EXAMPLE
 *    The function main() demonstrates the proper use of the portable "high
 *    level" interface iPLock() for file sharing & record locking, and the
 *    portable "very high level" interface iPSem() for semaphore operations.
 *
 * SEE ALSO
 *    Study the implementation of function iPLock() for an example of direct
 *    call to the portable "low level" interface PFCNTL.
 *-1*/

    char      pzPath[] = "./LCK.tst1";  /* Name of testfile for locking */
    char      pzDum[]  = "./LCK.tst2";  /* Name of dummy file/string */
    char      pzBuf[BUFLEN];          	/* Read buffer for testfile */
    char      *pStr    = NULL;         	/* Pointer to a string */    
    long      lStr     = 0L;           	/* Length of a string */

     char      cAnswer  = 'E';          /* INPUT: type of lock operation */
    off_t     lStart   = (off_t) -1;   /* INPUT: startpos in file for lock */
    off_t     lLen     = (off_t) -1;   /* INPUT: # bytes to lock (from lStart) */

    int       iFd1     = -1;           /* File descriptor, for WhiteBox-test */
    int       iRetCode = -1;           /* Int return code */
    long      lRetCode = 0L;           /* Long return code */


    /*----------------------------------------------------------------------*/
    /* 0: Signon & setup signal catcher                                     */

    fputs(SIGNON, stdout);
    signal(SIGINT, vSigCatch);
    signal(SIGTERM, vSigCatch);


    /*----------------------------------------------------------------------*/
    /* 1: Open testfile for update/sharing; - DOS: check version & share    */

    iFd = POPEN(pzPath);
    vChkErr(iFd == ASHEX, STOP, __LINE__, ABCHK + iFd);
    vChkErr(iFd == ASHLD && system("share") != 0, STOP, __LINE__, ABCHK + iFd);
    if (iFd == ASHLD) {
       iFd = POPEN(pzPath);
       printf("SHARE:\tSupport for fildeling/recordl†s load'ed\n");
    }
    vChkErr(iFd <= ERROR, STOP, __LINE__, AOPEN, errno, pzPath);

    iRetCode = iChkEnv();      /* DOS: check net */
    vChkErr(iRetCode == ANWLD || iRetCode == ANBLD, CONT, __LINE__, ABCHK + iRetCode);


    /*----------------------------------------------------------------------*/
    /* 2: LOOP : Test file locking, - execute from multiple terminals! */
    do {

       /* 2.1: Prompt for input */
       fputs("\nEnter code (H:HELP) [r|w|R|W|uU|sS|tT|mM|fF|gG|bB|hH|qQ] -> ", stdout);


       /* 2.2: Get input string from user/stdin, format: <CODE [START] [LEN]> */

       /* First get function CODE: <cAnswer> */
       scanf("%c", &cAnswer);

       /* Then get function param's [START] [LEN], if any:  [<lStart>] [<lLen>] */
       lStart = lLen = (off_t) -1;
       switch (toupper(cAnswer)) {
                               /* These CODEs require 2 param's: [START] & [LEN] */
           case 'R':           /* Read (shared) lock    : <lStart> <lLen> */
           case 'W':           /* Read (exclusive) lock : <lStart> <lLen> */
           case 'U':           /* Unlock (any) lock     : <lStart> <lLen> */
           case 'S':           /* Show (byte) range     : <lStart> <lLen> */
           case 'T':           /* Transaction on range  : <lStart> <lLen> */
               scanf("%ld %ld", &lStart, &lLen);
               if (lStart < 0 || lLen <= 0)
                   cAnswer = 'E';
               break;
                               /* These CODEs require 1 parameter: [START] */
           case 'M':           /* Mark semaphore in position : <lStart> */
           case 'F':           /* Free semaphore in position : <lStart> */
           case 'G':           /* Get value of sem. in pos.  : <lStart> */
               scanf("%ld", &lStart);
               if (lStart < 0)
                   cAnswer = 'E';
               break;

                               /* These CODEs take NO parameters */
           case 'B':           /* Perform "white-Box" test of module */
           case 'H':           /* Print Helpscreen for testdriver */
           case 'Q':           /* Quit testdriver for module */
               break;

       } /* END get function param's */


       while (getchar() != '\n')
           /* eat rest of line */;

       D(printf("Answer[%c] Start[%ld], Length[%ld]\n",
           cAnswer, lStart, lLen));


       /* 2.3: Call iPLock() or iPSem() with func. depending on entered CODE */
       switch (cAnswer) {

           /*---------------------------------------------------------------*/
           /*                      HLAPI : iPLock()                         */

           case 'r':          /* Set a non-blocking read (shared) lock */
               vChkEno(iPLock(iFd, lStart, lLen, NREAD));
               break;

           case 'w':          /* Set a non-blocking write (exclusive) lock */
               vChkEno(iPLock(iFd, lStart, lLen, NWRITE));
               break;

           case 'R':          /* Set a pending read (shared) lock */
               vChkEno(iPLock(iFd, lStart, lLen, BREAD));
               break;

           case 'W':          /* Set a pending write (exclusive) lock */
               vChkEno(iPLock(iFd, lStart, lLen, BWRITE));
               break;

           case 'u':          /* Remove either type of lock */
           case 'U':
               vChkEno(iPLock(iFd, lStart, lLen, UNLOCK));
               break;

           case 's':          /* Show contents of "record" */
           case 'S':
               lLen = (lLen > BUFLEN ? BUFLEN - 1 : lLen); /* Catch overflow */
               iRetCode = ICLAMP(lseek(iFd, lStart, SEEK_SET));
               vChkErr(iRetCode == ERROR, STOP, __LINE__, ASEEK, errno);
               iRetCode = read(iFd, pzBuf, (unsigned) lLen);
               vChkErr(iRetCode == ERROR, CONT, __LINE__, AREAD, errno);
               fputs(iRetCode != ERROR ? pzBuf[iRetCode] = '\0', pzBuf
                                       : "READ ERROR\n", stdout);
               break;

           case 't':          /* Perform NICE transaction on "record" */
               /* Nice trans: first set a non-blocking write (exclusive) lock */
               vChkEno(iPLock(iFd, lStart, lLen, NWRITE));
               /*FALLTHROUGH*/ 

           case 'T':          /* Perform CRUDE transaction on "record" */
               /* Crude transaction: process the record without locking */
               iRetCode = ICLAMP(lseek(iFd, lStart, SEEK_SET));
               vChkErr(iRetCode == ERROR, CONT, __LINE__, ASEEK, errno);
               iRetCode = read(iFd, pzBuf, (unsigned) lLen);
               vChkErr(iRetCode == ERROR, CONT, __LINE__, AREAD, errno);

               /* Stamp the record with mark : argv[1] or DUMMY */
               pStr = (argc > 1 ? argv[1] : pzDum);
               lStr = (lLen < (long) strlen(pStr) ? lLen : (long) strlen(pStr));
               strncpy(pzBuf, pStr, (size_t) lStr);

               /* Write back the record, - KEEPING IT LOCKED! */
               iRetCode = ICLAMP(lseek(iFd, lStart, SEEK_SET));
               vChkErr(iRetCode == ERROR, CONT, __LINE__, ASEEK, errno);
               iRetCode = write(iFd, pzBuf, (unsigned) lLen);
               vChkErr(iRetCode == ERROR, CONT, __LINE__, AWRIT, errno);
               break;

           /*---------------------------------------------------------------*/
           /*                      VHLAPI : iPSem()                         */

           case 'm':          /* Mark record in use (SEMUP on semaphore) */
           case 'M':
               iRetCode = iPSem(iFd, lStart, SEMUP);
               vChkErr(iRetCode < ERROR, CONT, __LINE__, ABSEM + iRetCode);
               vChkErr(iRetCode == ERROR, CONT, __LINE__, AFILE, errno);
               printf("iPSem-RETURN:[%c]\n", iRetCode <= ERROR ? 'E' : iRetCode);
               break;

           case 'f':          /* Free record for use (SEMDOWN on semaphore) */
           case 'F':
               iRetCode = iPSem(iFd, lStart, SEMDOWN);
               vChkErr(iRetCode < ERROR, CONT, __LINE__, ABSEM + iRetCode);
               vChkErr(iRetCode == ERROR, CONT, __LINE__, AFILE, errno);
               printf("iPSem-RETURN:[%c]\n", iRetCode <= ERROR ? 'E' : iRetCode);
               break;

           case 'g':          /* Get record usage (semaphore: SEMUP or SEMDOWN) */
           case 'G':
               iRetCode = iPSem(iFd, lStart, SEMTEST);
               vChkErr(iRetCode < ERROR, CONT, __LINE__, ABSEM + iRetCode);
               vChkErr(iRetCode == ERROR, CONT, __LINE__, AFILE, errno);
               printf("iPSem-RETURN:[%c]\n", iRetCode <= ERROR ? 'E' : iRetCode);
               break;

           /*---------------------------------------------------------------*/
           /*                      Miscellaneous                            */

           case 'b':           /* WhiteBox module test */
           case 'B':
               printf("\n\nW-BOX :\tfile operations on non existant file\n");
               iFd1 = POPEN(pzDum);
               vChkErr(iFd1 == ERROR, CONT, __LINE__, AOPEN, errno, pzDum);             
               iRetCode = ICLAMP(lseek(iFd1, lStart, SEEK_SET));
               vChkErr(lRetCode == ERROR, CONT, __LINE__, ASEEK, errno);
               iRetCode = read(iFd1, pzBuf, 1);
               vChkErr(iRetCode == ERROR, CONT, __LINE__, AREAD, errno);
               iRetCode = write(iFd1, pzBuf, 1);
               vChkErr(iRetCode == ERROR, CONT, __LINE__, AWRIT, errno);

               printf("\n\nW-BOX :\tCatch of wraparound by long->int conv.\n");
               lRetCode = lseek(iFd, 32768, SEEK_SET);
               vChkErr(lRetCode == ERROR, CONT, __LINE__, ASEEK, errno);
               iRetCode = lseek(iFd, 32768, SEEK_SET);
               vChkErr(iRetCode == ERROR, CONT, __LINE__, ASEEK, errno);
               printf("\nW-BOX :\tlRetCode=[%ld] <-vs-> iRetCode=[%d]\n", lRetCode, iRetCode);
               iRetCode = ICLAMP(lseek(iFd, 32768, SEEK_SET));
               vChkErr(iRetCode == ERROR, CONT, __LINE__, ASEEK, errno);
               printf("\nW-BOX :\tlRetCode=[%ld] <-vs-> ICLAMP(lRetCode)=[%d]\n", lRetCode, iRetCode);

               printf("\n\nW-BOX:\tCheck environment\n");
               iRetCode = iChkEnv();
               vChkErr(iRetCode != OK, CONT, __LINE__, ABCHK + iRetCode);
               break;

           default:           /* Undefined */
               cAnswer = 'E';
               /*FALLTHROUGH*/

           case 'E':          /* Error */
               fputs("\nERROR in input, - try again ...\n\a", stderr);
               /*FALLTHROUGH*/

           case 'h':          /* Help */
           case 'H':
               fputs("\nLCK.C function codes :\n", stdout);
               fprintf(stdout, "\t+======================== HLAPI ============================+\n");
               fprintf(stdout, "\t: For all :      starting lock at <s>, length <l> byte      :\n");
               fprintf(stdout, "\t:   r   <s> <l>  Set a non-blocking read (shared) lock      :\n");
               fprintf(stdout, "\t:   w   <s> <l>  Set a non-blocking write (exclusive) lock  :\n");
               fprintf(stdout, "\t:   R   <s> <l>  Set a pending read (shared) lock           :\n");
               fprintf(stdout, "\t:   W   <s> <l>  Set a pending write (exclusive) lock       :\n");
               fprintf(stdout, "\t: [u|U] <s> <l>  Unlock either type of lock                 :\n");
               fprintf(stdout, "\t+-----------------------------------------------------------+\n");
               fprintf(stdout, "\t: [s|S] <s> <l>  Show file content ('dump' to screen)       :\n");
               fprintf(stdout, "\t:   t   <s> <l>  Nice  Transaction (lock & 'stamp' argv[1]) :\n");
               fprintf(stdout, "\t:   T   <s> <l>  Crude Transaction ('stamp' argv[1])        :\n");
               fprintf(stdout, "\t+======================== VHLAPI ===========================+\n");
               fprintf(stdout, "\t: For all :      semaphore at pos <s>                       :\n");
               fprintf(stdout, "\t: [m|M] <s>      Mark a record 'in use' (SEMUP on sem. s)   :\n");
               fprintf(stdout, "\t: [f|F] <s>      Free a record for use  (SEMDOWN on sem. s) :\n");
               fprintf(stdout, "\t: [g|G] <s>      Get record usage (state of semaphore s)    :\n");
               fprintf(stdout, "\t+-----------------------------------------------------------+\n");
               fprintf(stdout, "\t: [b|B]          Perform whiteBox test of module            :\n");
               fprintf(stdout, "\t: [h|H]          Print this help screen for reference       :\n");
               fprintf(stdout, "\t: [q|Q]          Quit (exit) the lck testprogram            :\n");
               fprintf(stdout, "\t+===========================================================+\n");
               break;

           case 'q':          /* Quit */
           case 'Q':
               break;

       } /* END switch cAnswer */


    } while (toupper(cAnswer) != 'Q');
    /* LOOP (retry) on Error / EXIT on Quit */


    /* Return to environment */
    exit(EXIT_SUCCESS);

}
/* END main() */



/*+4 MODULE LCK.C ----------------------------------------------------------*/
/*   NAME                         vSigCatch                                 */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vSigCatch(iSigNum)
    int       iSigNum;
{
/* DESCRIPTION
 *    Support function for main() test driver.
 *    Signal handler set up to catch "break" signals : SIGINT (asynch.
 *    interactive attention) & SIGTERM (asynch. interactive termination).
 *
 *    1: Prompt user for break confirmation
 *    2: Depending on user answer: [Y]->terminate, [N]->continue
 *
 * RETURN
 *    If break confirmed: program terminated with exit code EXIT_SUCCESS
 *    else: signal 'iSigNum' reset and program execution resumed.
 *-4*/

    /* 1: Prompt for break confirmation */
    fprintf(stdout, "\nINTERRUPT:\tSignal [%d] received\n", iSigNum);
    fprintf(stdout, "\tAbort %d testdriver [Y|N] => ", iSigNum);


    /* 2: Depending on user answer: [Y]->terminate, [N]->continue */
    if (toupper(getchar()) == 'Y') {
       /* 2.1: Remove outstanding locks owned by this process, and exit */
       close(iFd);
       exit(EXIT_SUCCESS);
    }
    else
       /* 2.2: Reset signal for catch by signalhandler & continue */
       signal(iSigNum, vSigCatch);

} /* END function vSigCatch() */



/*+4 MODULE LCK.C ----------------------------------------------------------*/
/*   NAME                         vChkEno                                    */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vChkEno(iRetCode)
    int       iRetCode;        /* return code from fcntl */
{
/* DESCRIPTION
 *    Support function for main() test driver.
 *    Check return code from PFCNTL (passed as iRetCode) :
 *      0 : success
 *     -1 : standard error from PFCNTL (with errortype specified by "errno")
 *     -2 : DOS-specific error : bad seek in file
 *
 * RETURN
 *    Side effects : Error-message printed on stderr
 *    Func. Return : none (void)
 *-4*/

    if (iRetCode == OK)        /* iRetCode = 0 */
       fprintf(stderr, "Function completed!\n");
    else
       switch (iRetCode) {

           case ERROR:       /* iRetCode = -1 */
               switch (errno) {

                   case EACCES:
                   case EAGAIN:
                       fprintf(stderr, "E[%d,%d] already locked\n", iRetCode, errno);
                       break;

                   case EBADF:
                       fprintf(stderr, "E[%d,%d] bad file descriptor\n", iRetCode, errno);
                       break;

                   case EINTR:
                       fprintf(stderr, "E[%d,%d] fcntl aborted\n", iRetCode, errno);
                       break;

                   case EINVAL:
                       fprintf(stderr, "E[%d,%d] invalid argument\n", iRetCode, errno);
                       break;

                   case EMFILE:
                       fprintf(stderr, "E[%d,%d] no available filedesc.\n", iRetCode, errno);
                       break;

                   case ENOLCK:
                       fprintf(stderr, "E[%d,%d] no available locks\n", iRetCode, errno);
                       break;

                   case EDEADLK:
                       fprintf(stderr, "E[%d,%d] potential deadlock\n", iRetCode, errno);
                       break;

                   default:
                       fprintf(stderr, "E[%d,%d] impossible fcntl err!\n", iRetCode, errno);
                       break;
               } /* END switch (errno) */
               /* NOTREACHED*/
               break;

           case -2:           /* iRetCode = -2 */
               fprintf(stderr, "E[%d] bad seek in file\n", iRetCode);
               break;

           default:           /* iRetCode [ <-2 or >0 ] */
               fprintf(stderr, "E[%d] Undefined errorcode from PFCNTL!\n", iRetCode);
               break;

       } /* END switch (iRetCode) */

} /* END function vChkEno() */



/*+4 MODULE ----------------------------------------------------------------*/
/*   NAME                          vChkErr                                  */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE void
vChkErr(int cond, int cont, int iLine, int err,  ...)
//    int cond;      	/* Error-condition to check (TRUE=error) */
//    int cont;      	/* Continue in spite of error (TRUE=yes) */
//    int iLine;     	/* Source code line pinpointing error */
//    int err;       	/* Errornumber if error-condition TRUE */
			/* Var. # args for error-msg. - cf. <general.h> */
{
/* DESCRIPTION
 *    Support function for main() test driver.
 *    Simple error handler: All errors are fatal (ie cause program term.).
 *    Error messages are written to stderr, and may be redirected to a file.
 *    Error-messages are "hard coded" into array rgpchErrmsg[].
 *
 *    1: Print header identifying error location (file, line, date, time)
 *       [STDC:Point arg.ptr. to 1. var arg, Print error-message & Clean up]
 *
 * RETURN
 *    2: Side effect, depending on flag "cont" :
 *       - FALSE: Exit module with "core-dump" (DEBUG) or "fail-code" (normal).
 *       - TRUE : Continue execution (return none/void).
 *-4*/
#ifdef __STDC__
       /* Var. Argument pointer */
       va_list   ap;
#endif /*__STDC__*/

    assert(cont == STOP || cont == CONT);

    if (cond) {

       /* 1.1: Print error header identifying module */
       fprintf(stderr, "\nMODULE: File[%s] - Line[%d]" \
           "\n\tVersion: Date[%s] - Time[%s]" \
           "\n\tError..: Number[%02d] - ", \
           __FILE__, iLine, __DATE__, __TIME__, err);

#ifdef __STDC__
       /* 1.2: Point ap to 1.vararg, Print error-message & Clean up */
	va_start(ap, err);
       (void) vfprintf(stderr, rgpchErrmsg[err], ap);
       fflush(stderr);
       va_end(ap);
#endif /*__STDC__*/

       /* 2: Exit module with "core-dump" (DEBUG-mode) or "fail-code" (normal) */
       if (!cont) {
           D(abort());
           exit(EXIT_FAILURE);
       }

    }  /* END if(cond) */
    

} /* END function vChkErr() */

#endif /* #ifdef MAIN */



#ifdef MSDOS
/****************************************************************************/
/*************************** MSDOS LOW-LEVEL API ****************************/
/***************************   DOPEN & DFCNTL    ****************************/
/****************************************************************************/


/*+2 MODULE LCK.C===========================================================*/
/*   NAME                         DOPEN                                     */
/*== SYNOPSIS ==============================================================*/
PUBLIC int
DOPEN(pzFile)
    char     *pzFile;          /* Path of file to open */
{
/* DESCRIPTION
 *    Portable file open for locking operations as defined in MSDOS(MSC60).
 *
 *    macro POPEN -> [ DOS:DOPEN() | POSIX:open() ];
 *  - If MSDOS is defined at compile time (-DMSDOS), the macro name POPEN
 *    maps to this LowLevel DOS-specific interface for "open()".
 *  - If UNIX is defined at compile time (-DUNIX), the macro name POPEN
 *    maps directly to the open() function (and this code is not compiled!)
 *
 *    The user interface on MSDOS and UNIX is invariant : POPEN. On both ope-
 *    rating systems, POPEN will enforce the correct open-flags (cf. setup of
 *    OFLAG in LCK.H) to ensure opening for file sharing and record locking.
 *
 *    1: Check environment for DOS Version & SHARE.EXE.
 *       If Dos Ver < 3 or share not loaded, return (without opening file).
 *    2: Open file for sharing/locking
 *
 * RETURN
 *     -3 : ERROR:   SHARE.EXE not loaded (required for file shr & rec. lock)
 *     -2 : ERROR:   DOS Version < 3 (no file sharing support!)
 *     -1 : ERROR:   Bad open operation - precise error code in global "errno"
 *                   Common DOS(MSC60)-UNIX(X/Open) subset for "errno" :
 *                      EACCES - access permissions denied
 *                     [EEXIST - create of existing file - should not occur]
 *                     [EINVAL - invalid value of OFLAG  - should not occur]
 *                      EMFILE - max open files exceeded
 *                      ENOENT - file for open does not exist
 *    >=0 : OK       Return value = File handle for opened file.
 *-2*/
    int       iRetCode;

    /* 1: Check environment for DOS Version &  SHARE.EXE */
    iRetCode = iChkEnv();
    if (iRetCode == -2 || iRetCode == -3)
       return (iRetCode);

    /* 2: Open file in share/lock mode */
    return (sopen(pzFile, OFLAG));

} /* END function DOPEN() */



/*+4 MODULE LCK.C ----------------------------------------------------------*/
/*   NAME                         iChkEnv                                   */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE int
iChkEnv(VOID)
{
/* DESCRIPTION
 *    Support function for DOPEN; - check DOS environment for multiuser support.
 *
 * 1: Check major DOS OS version ("_osmajor" in <stdlib.h> of MSC & BTC++)
 *    NB: DOS versions < 3.0 should be upgraded for multiuser support.
 *
 * 2: Check load of SHARE.EXE using DOS MULTIPLEX INT 2FH, 1000H.
 *    SHARE.EXE must be present for file sharing and locking in DOS.
 *    The program is loaded by the following command (issued from the cmd.line
 *    or from autoexec.bat/config.sys(DOS 5.0):  [c:\dos\]share /f:1024 /l:50
 *    NB Dont perform this test on pre-DOS 3.0, or you'll crash the machine!
 *
 * 3: Check precence of networked drives, using DOS INT 21H, IOCTL 4409H.
 *    NB This call can't discriminate between a network drive and a CD-ROM!
 *
 * 4: Check precence of NetBIOS entry INT 2AH.
 *    NB NetBIOS is not a required feature of a LAN, - the program might be
 *    installed on a NFS (UNIX "Network File System") drive mapped on top of
 *    TCP/IP, in which case the netdrive-test (3) would STILL work correctly.
 *
 * RETURN
 *     -5 : ERROR:   NetBIOS interface not present (load net TSR or driver)
 *     -4 : ERROR:   No redirected drives present (lacking net configuration)
 *     -3 : ERROR:   SHARE.EXE not loaded (required for file shr & rec lock)
 *     -2 : ERROR:   DOS Version < 3 (no file sharing support!)
 *     -1 : ERROR:   Bad open operation - precise error code in global "errno"
 *      0 : OK   :   Function complete.
 *-4*/

    union REGS ioregs;                 /* DOS cpu register structure */
    int        fNetDrive = FALSE;      /* FLAG: remote drives present? */
    int        iDrive    = 0;          /* drive number on machine */


    /* 1: Check major DOS OS version (must be > 3.00) */
    if (_osmajor < 3)
       return (-2);


    /* 2: Check load of SHARE.EXE */           /* AH=10 : MPLX SHARE.EXE hook */
    ioregs.x.ax = 0x1000;                      /* AL=00 : Get installed state */
    (void) int86(0x2F, &ioregs, &ioregs);      /* Query DOS Multiplex INT     */
    if (ioregs.h.al != 0xFF)                   /* 0xFF : ret code "I'm Here!" */
       return (-3);


    /* 3: Check precence of networked drives */
    for (iDrive = 3; iDrive <= 26; iDrive++) { /* AH=44 : DOS IOCTL interrupt    */
       ioregs.x.ax = 0x4409;                   /* AL=09 : Is device local/remote */
       ioregs.x.bx = iDrive;                   /* Drive# : 0=default, 1=A, etc   */
       (void) int86(0x21, &ioregs, &ioregs);   /* Query DOS Function Request INT */
       if ((ioregs.x.dx & 0x1000) == 0x1000)   /* Remote devices: Bit 12 set     */
           fNetDrive = TRUE;
    }
    if (!fNetDrive)
       return (-4);


    /* 4: Check precence of NetBIOS interface */
    ioregs.h.ah = 0;                           /* Redirector-(INT2AH)->NetBIOS */
    (void) int86(0x2A, &ioregs, &ioregs);      /* AH=0 : Installation check    */
    if (ioregs.h.ah == 0)                      /* AH=0:Not Installed; AH=1:OK  */
       return (-5);


    /* OK, environment in place */
    return (OK);

} /* END function iChkEnv() */




/*+2 MODULE LCK.C===========================================================*/
/*   NAME                         DFCNTL                                    */
/*== SYNOPSIS ==============================================================*/
PUBLIC int
DFCNTL(iFd, iFcntlCmd, pstFlock)
    int       iFd;         /* FileDescr. identifying file for locking op. */
    int       iFcntlCmd;   /* fcntl() command : [F_SETLK | F_SETLKW] */
    stFlock_t *pstFlock;   /* Pointer to flock-str. def. lock type & range */
{
/* DESCRIPTION
 *    Portable "LowLevel API" for locking operations as defined in MSDOS(MSC60).
 *
 *    macro:PFCNTL -> [ DOS:DFCNTL() | POSIX:fcntl() ];
 *  - If MSDOS is defined at compile time (-DMSDOS), the macro name PFCNTL
 *    maps to this LowLevel DOS-specific interface for "fcntl()".
 *    DFCNTL imitates the fcntl()-function as defined by X/Open & POSIX,
 *    using the MSC60 "high level" func. locking() for file sharing/locking.
 *  - If UNIX is defined at compile time (-DUNIX), the macro name PFCNTL
 *    maps directly to the fcntl() function (and this code is not compiled!)
 *
 *    The user interface on MSDOS and UNIX is invariant : PFCNTL, - hence
 *    the term "Portable (LowLevel) API". - A portable HighLevel API to
 *    fcntl() is provided by the function iPLock(), see below.
 *
 *    1: Seek to the file position for the locking operation.
 *    2: Perform the locking operation, using the MSC60-specific primitives.
 *
 * RETURN
 *    The return code of locking() are :
 *    -1 : ERROR: Bad locking operation, - precice error code in global "errno".
 *                The DOS(MSC60) "errno" codes for locking() are :
 *                   EACCESS     access to locked region denied
 *                   EBADF       bad (not valid) file descriptor
 *                   EDEADLOCK   desired lock is blocked by other process
 *                   EINVAL      invalid argument
 *     0 : "OK" : Function completed succesfully.
 *
 * SEE ALSO
 *    Function iPLock() for the common DOS/UNIX API to file sharing/locking.
 *-2*/

    /* 1: Seek to the file position for the locking opreation */
    D(printf("DEBUG:LSEEK(%d, %d, %d)\n",
       iFd, pstFlock->l_start, pstFlock->l_whence));
    if (lseek(iFd, pstFlock->l_start, pstFlock->l_whence) == -1L)
       return (ERROR);

    /* 2: Do the locking operation */
    D(printf("DEBUG:LOCKING(%d, %s, %d)\n",
       iFd, F_OP[F_MAP[iFcntlCmd][pstFlock->l_type]], pstFlock->l_len));
    if (locking(iFd, F_MAP[iFcntlCmd][pstFlock->l_type], pstFlock->l_len) == -1)
       return (ERROR);

    /* 3: Return "fcntl OK" */
    return (OK);

} /* END function DFCNTL() */

#endif /* #ifdef MSDOS */




/****************************************************************************/
/*************************** UNIX/MSDOS HIGH-LEVEL API **********************/
/***************************        iPLock             **********************/
/****************************************************************************/

/* Declare (allocate) a "flock" structure for the PFCNTL argument */
PRIVATE stFlock_t stFlock;


/*+2 MODULE LCK.C ==========================================================*/
/*   NAME                         iPLock                                    */
/*== SYNOPSIS ==============================================================*/
PUBLIC int
iPLock(iFd, lStart, lLen, iOp)
    int       iFd;         /* FileDescr. identifying file for locking op. */
    long      lStart;      /* Start of lock (offset from start-of-file) */
    long      lLen;        /* Length of lock (#bytes from lStart) */
    int       iOp;         /* Lock op: [NREAD|NWRITE|BREAD|BWRITE|UNLOCK] */
{
/* DESCRIPTION
 *    Portable "HighLevel API" for locking operations in MSDOS(MSC60) & UNIX.
 *
 *    1: Set the fcntl() record locking COMMAND : [F_SETLK | F_SETLKW]
 *    2: Define the TYPE the lock : [F_RDLCK | F_WRLCK | F_UNLCK]
 *       Define the REGION for the lock : offset (parameter lStart) and
 *       length (parameter lLen), - both taken from start-of-file (SEEK_SET).
 *    3: Call the portable fcntl()-function to perform the locking operation,
 *       and return the result of PFCNTL.
 *
 * RETURN
 *    The return code of iPLock() "imitates" the X/Open fcntl() return values:
 *    -1 : ERROR: Bad PFCNTL operation, - precice error code in global "errno".
 *                The common DOS(MSC60)-UNIX(X/Open) subset for "errno" is :
 *                   EACCESS     access to locked region denied
 *                   EBADF       bad (not valid) file descriptor
 *                   EDEADLK     desired lock is blocked by other process
 *                               (MSC60 EDEADLOCK is mapped to X/Open EDEADLK)
 *                   EINVAL      invalid argument
 *     0 : OK   : Function completed succesfully.
 *
 * EXAMPLE
 *    // NB: You must include error checking in your own programs.
 *    #include "lck.h"             // Include the lck header file
 *    int     iFd;                 // Declare a file descriptor
 *    char    pzFn="data.fil";     // Declare a file name
 *
 *    iFd = POPEN(pzFn);           // Open file for share/lock
 *    iPLock(iFd, 0, 100, NWRITE); // Set a write lock on byte 0-100
 *    ...                          // Work on byte 0-100
 *    iPLock(iFd, 0, 100, UNLOCK); // Release lock on byte 0-100
 *
 * SEE ALSO
 *    Function DFCNTL() for the DOS implementation of PFCNTL.
 *-2*/
    int       iFcntlCmd;           /* fcntl() command, extracted from table L_MAP */


    /* 0: Debugging trace */
    D(printf("DEBUG:iPLock(iFd=%d, lSt=%ld, lLn=%ld, iOp=%s)\n",
       iFd, lStart, lLen, L_OP[iOp]));

    /* 1: Validate arguments */
    if (iFd < 0 || lStart < 0 || lLen <= 0) {
       errno = EINVAL;
       return (-1);
    }

    /* 2: Set the record locking COMMAND (by mapping iOp through L_MAP[.][0]) */
    iFcntlCmd = L_MAP[iOp][0];

    /* 3: Define the TYPE of lock (by mapping iOp through L_MAP[.][1]) */
    stFlock.l_type   = L_MAP[iOp][1];      /* Set record locking TYPE           */
    stFlock.l_whence = SEEK_SET;           /* Measure REGION from start-of-file */
    stFlock.l_start  = (off_t) lStart;     /* - Offset for start of lock        */
    stFlock.l_len    = (off_t) lLen;       /* - Length (#byte) of lock          */
    stFlock.l_pid    = 0;                  /* Ignore this field (UNIX specific) */

    /* 4: Finally call the portable fcntl() func. to perform the operation */
    return (PFCNTL(iFd, iFcntlCmd, &stFlock));

} /* END function iPLock() */




/****************************************************************************/
/*********************** UNIX/MSDOS VERY-HIGH-LEVEL API *********************/
/***********************          iPSem                 *********************/
/****************************************************************************/

/* Retry count for grabbing semaphore (DOS default retries 3x with delay) */
#define   RETRIES      5


/*+2 MODULE LCK.C ==========================================================*/
/*   NAME                         iPSem                                     */
/*== SYNOPSIS ==============================================================*/
PUBLIC int
iPSem(iFd, lStart, iOp)
    int       iFd;         /* FileDescr. identifying file with semaphore(s) */
    long      lStart;      /* Position from start of file of semaphore-byte */
    int       iOp;         /* Semaph.operation: [SEMUP | SEMDOWN | SEMTEST] */
{
/* DESCRIPTION
 *    Portable "Very HighLevel API" for lock operations in MSDOS(MSC60) & UNIX.
 *    This function offers an interface for working with semaphores implemented
 *    as "lock bytes" directly in a datafile. By including a semaphore (an
 *    extra byte) in each data-record and calling on iPSem() to set/clear the
 *    sem.value, a user may ensure mutual exclusive access to the file records.
 *
 *    The semaphore controlled access strategy requires a disciplined behaviour
 *    on behalf of the user-programs :
 *     - The function iPSem() accesses the semaphores inside a "critical
 *       section" (implemented using the portable function iPLock to set at
 *       short-lived write-lock on the semaphore byte), and thereby it does
 *       guarantee "race-free" updating of the semaphore.
 *     - The function will however NOT lock the associated datarecord, so
 *       programs may still freely read and write the record; - it is the
 *       user's reponsibility to serialize write-access (updating) of the
 *       data-records, by always "grabbing" the semaphore before performing
 *       a write operation.
 *
 * RETURN
 *    -5  : Attempt to "SEMUP/SEMDOWN/SEMTEST" on a byte with invalid semaphore value
 *          NB: a semaphore byte must be initialized to SEMDOWN!
 *    -4  : Attempt to "SEMUP" on a semaphore that is already UP (Record in use)
 *    -3  : Attempt to "SEMDOWN" on a semaphore, that is already DOWN (Record free)
 *    -2  : Bad input argument value
 *    -1  : ERROR  : Bad OS file operation, - precice error code in global "errno".
 *    >=0 : SUCCESS: Function completed, - current semaphore value is returned.
 *
 * EXAMPLE
 *    #include "lck.h"            // Include the lck header file
 *    int     iFd;                // Declare a file descriptor
 *    char    pzFn="data.fil";    // Declare a file name
 *    long    lSemPos = 0;        // Declare a semaphore in file, position 0
 *
 *    iFd = POPEN(pzFn);          // Open file for share/lock
 *
 *    iRetCode = iPSem(iFd, lSemPos, SEMUP);    // Grab the record (start pos 0)
 *    ...                                       // Read, Update & Write record
 *    iRetCode = iPSem(iFd, lSemPos, SEMDOWN);  // Release the record
 *-2*/

    int       iRetCode, iRetCode2; /* Function return codes */
    int       iRetry =  0;         /* Counter for retry of (non pending) lock */
    char      cSem   = 'X';        /* Semaphore variable,- initially undefined */


    /* 0: Debugging trace */
    D(printf("DEBUG:iPSem(iFd=%d, lSt=%ld, iOp=%c)\n", iFd, lStart, (unsigned char) iOp));


    /* 1: Validate arguments */
    if (iFd < 0 || lStart < 0)
       return (-2);


    /* 2.1: CS---------- Start critical section ----------CS */
    for (iRetry = 1, iRetCode = ERROR;
         iRetry <= RETRIES && iRetCode != OK; iRetry++) {
       iRetCode = iPLock(iFd, lStart, 1L, NWRITE);
    }
    if (iRetCode != OK)
       return (iRetCode);


    /* 2.2: Perform operation on semaphore; NB: no return out of CS! */
    switch (iOp) {

        case SEMTEST:      /* TEST semaphore */
            iRetCode = iSemIO(iFd, lStart, SEMRD, &cSem);
            if (iRetCode < OK)
               break;
            if (cSem != SEMUP && cSem != SEMDOWN)
               iRetCode = -5;
            break;

        case SEMUP:        /* UP on semaphore */
            /* Read the semaphore; If SEMUP or invalid - back out! */
            iRetCode = iSemIO(iFd, lStart, SEMRD, &cSem);
            if (iRetCode < OK)
               break;
            if (cSem == SEMUP) {
               iRetCode = -4;
               break;
            }
            if (cSem != SEMDOWN) {
               iRetCode = -5;
               break;
            }

            /* Perform SEMUP, and write back the semaphore */
            cSem = SEMUP;
            iRetCode = iSemIO(iFd, lStart, SEMWR, &cSem);
               break;

        case SEMDOWN:      /* DOWN on semaphore */
            /* Read the semaphore; If SEMDOWN or invalid - back out! */
            iRetCode = iSemIO(iFd, lStart, SEMRD, &cSem);
            if (iRetCode < OK)
               break;
            if (cSem == SEMDOWN) {
               iRetCode = -3;
               break;
            }
            if (cSem != SEMUP) {
               iRetCode = -5;
               break;
            }

            /* Perform SEMDOWN, and write back the semaphore */
            cSem = SEMDOWN;
            iRetCode = iSemIO(iFd, lStart, SEMWR, &cSem);
            break;

        default:           /* INVALID operation on semaphore */
            iRetCode = -2;
            break;

    } /* END switch(iOp) */


    /* 2.3: CS---------- End critical section ----------CS */
    iRetCode2 = iPLock(iFd, lStart, 1L, UNLOCK);
    if (iRetCode2 != OK)
       iRetCode = iRetCode2;

    /* 4: Return error or current state of semaphore */
    return (iRetCode <= ERROR ? iRetCode : (int) cSem);

} /* END function iPSem() */



/*+4 MODULE LCK.C ----------------------------------------------------------*/
/*   NAME                    iSemIO                                         */
/*-- SYNOPSIS --------------------------------------------------------------*/
PRIVATE int
iSemIO(iFd, lStart, iOp, pcSem)
    int       iFd;         /* FileDescr. identifying file for locking op. */
    long      lStart;      /* Position from file start holding the LockByte */
    int       iOp;         /* File operation: [SEMRD | SEMWR], sem rd/wr */
    char     *pcSem;       /* Address of sem ("byte buffer") for file rd/wr */
{
/* DESCRIPTION
 *    Support function for the user interface to semaphore operations: iPSem().
 *    Performs Read/Write of a semaphore (a lock-byte) from/to a file.
 *    NB: Must be called from inside a critical region to guarantee validity.
 *
 * RETURN
 *    -2  : Bad input argument to iSemIO
 *    -1  : ERROR   : Bad DOS file operation, - error code in global "errno".
 *    >=0 : SUCCESS : OK  DOS file operation, - function completed.
 *-4*/
    int    iRetCode = ERROR;       /* Return code from DOS file operation */


    /* 1: Validate arguments */
    assert(iOp == SEMRD || iOp == SEMWR);          /* Hard/internal error */
    if (iFd < 0 || lStart < 0 || pcSem == NULL)    /* User error */
       return (-2);

    /* 2: Seek to the file position of the semaphore */
    iRetCode = ICLAMP(lseek(iFd, lStart, SEEK_SET));

    /* 3: Read/Write the semaphore value (*pcSem) from/to the file */
    if (iRetCode != ERROR)

       switch (iOp) {

           case SEMRD:       /* READ semaphore from file */
               iRetCode = read(iFd, pcSem, 1);
               break;

           case SEMWR:       /* WRITE semaphore to file */
               iRetCode = write(iFd, pcSem, 1);
               break;

           default:           /* INVALID semaphore I/O operation */
               iRetCode = -2;
               break;

       } /* END switch(iOp) */


    /* 4: Debugging trace */
    D(printf("DEBUG:iSemIO(iFd=%d, lSt=%ld, iOp=%d, cSem=%c )\n",
       iFd, lStart, iOp, *pcSem));

    /* 5: Return status */
    return (iRetCode);

} /* END function iSemIO() */



/* END module LCK.C                                                         */
/*==========================================================================*/
