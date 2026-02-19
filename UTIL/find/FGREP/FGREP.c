// ===========================================================================
// Program header files
// ===========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define DEBUG
// #undef DEBUG
#include "tbm.h"


// ===========================================================================
// Program usage message
// ===========================================================================
char caUsage[] =
    "Usage: fileutil.exe <basePath> <drill> <searchPattern>\n"                  \
    "Where: <basePath>  full path to root directory for search\n"               \
    "       <drill> is boolean  Y: for recurse over subdirectories\n"           \
    "                           N: for not recurse over subdirectory tree\n"    \
    "       <searchPattern> is the word to search for in all files\n";


// ===========================================================================
// Function prototypes
// ===========================================================================

// Parse and search file path and files
int iGetDirTree(const char *root, char drill);
int iGetFileList(const char *directory);
int iSearchText(const char *dirName, const char *fileName);
int iSearchPS(const char *psScript, const char *dirName, const char *cpPattern);

// Directory
int iIsDirType(const char *path);
int iIsDirLink(const struct dirent *entry);
void vDumpDirTree();

// File
int isImage(const char *path);
int isBinary(const char *path);
void vToUpper(char * string);


// ===========================================================================
// Global data structs (yeah, to be refactored..)
// ===========================================================================
#define maxIdx 1024

// Directories
int iDirIdx = 0;
char paDirTree[maxIdx][maxIdx];     // List holding directory tree

// Files
int iFileIdx = 0;
char paFileList[maxIdx][maxIdx];    // List of files


// #################################################################################
//                  Main program
// #################################################################################
int main(int argc, char *argv[]) {

    // 1:   Check Cmd line arg number
    //      Extract variables from argv[]: <directory-root> <drill> <search-pattern>
    if (argc != 4) {
        fprintf(stderr, "%s", caUsage);
        return 0;
    }
    // const char* cpProgram = argv[0];         // [0]: <fileutil.exe>
    const char* cpBasePath = argv[1];           // [1]: <basePath>
    const char* cpDrill = argv[2];              // [2]: <drill> : ["Y"|"N"]
    const char* cpSearchPattern = argv[3];      // [3]: <searchPattern>


    // 2:   Get arg directory root, and (optionally) recursively add subdirectories
    if ( !iIsDirType(cpBasePath)) { perror("iIsDirectory() error"); return 0; }
    chdir(cpBasePath);                          // cd to basePath for search (possibly link)
    char cwd[maxIdx];                           // Get FULL <basePath> path
    if (getcwd(cwd, sizeof(cwd)) == NULL) { perror("getcwd() error"); return 0; }
    cpBasePath = cwd;
    strcpy(paDirTree[iDirIdx++], cpBasePath);   // Set Directory root

    iGetDirTree(cpBasePath, cpDrill[0]);        // Get Sub-directories

    D( vDumpDirTree(); )

    // 3:   Prepare <search pattern> for TBM search
    vBuildTBM( (BYTE*) cpSearchPattern, strlen(cpSearchPattern));

    // -----------------------------------------------------------------------------------
    // 4:   Iterate through paDirTree[*][maxIdx]
    for (int i = 0; i<iDirIdx; i++)  {

        // 4.1: Foreach directory [maxIdx] in paDirTree[i],
        //      get list of files: paFileList [*][maxIdx],
        printf("\nDIR[%04d]: %s\n", i, paDirTree[i]);
        iFileIdx = 0;
        iGetFileList(paDirTree[i]);     // Global iFileIdx

        // 4.2: Foreach file in paFileList[*][1024],
        //      search lines for <search pattern> using iRunTBM
         for (int j =0; j<iFileIdx; j++) {

            D( printf("FILE:\t%s", paFileList[j]) );
            if ( isImage(paFileList[j])         // Skip graphics
            ||   isBinary(paFileList[j])        // Skip Word, PFD...
             ) continue;

            iSearchText( paDirTree[i], paFileList[j] );
        }

        // 4.3: Done, -- now reset current dir back to original root dir
        chdir( paDirTree[0] );
    }


    // -----------------------------------------------------------------------------------
    // Spawn PowerShell script to recursively find <searchPattern> in .doc and .pdf files

    iSearchPS("/findDoc.ps1", paDirTree[0], cpSearchPattern);

    iSearchPS("/findPdf.ps1", paDirTree[0], cpSearchPattern);

    return 1;
}


// ================================================================================
//  Function to recursively build directory tree into global: paDirTree[*][1024]
// ================================================================================
int iGetDirTree(const char *root, const char drill)
{

    // 1: Open <root> as base directory
    struct dirent *entry;
    DIR *dir = opendir(root);
    if (dir == NULL) { perror("Unable to open directory"); return 0; }

    // 2: If <drill>: recursively add all root subdirs to paDirTree[*][]
    if ( drill == 'N') { closedir(dir); return 1; }   // No drill -- quit
    while ((entry = readdir(dir)) != NULL && iDirIdx < maxIdx) {

        // Skip if non-regular file (".", "..")
        if ( iIsDirLink(entry) ) continue;

        // Construct full path for next dir entry
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", root, entry->d_name);

        // If path is a directory, add path to paDirTree[*] and get subdirectories
        if ( iIsDirType(path) ) {
            strcpy(paDirTree[iDirIdx++], path);     // Add path to dir list
            iGetDirTree(path, drill);               // Treat path as new root
        }
    }
    closedir(dir);
    return 1;
}

int iIsDirLink(const struct dirent *entry) {
    return (  ( (entry->d_namlen == 1 && strcmp(entry->d_name, ".")  == 0)
             || (entry->d_namlen == 2 && strcmp(entry->d_name, "..") == 0 ) )
        ? 1 : 0 );
}

int iIsDirType(const char *path){
    struct stat statbuf;
    if  ( stat(path, &statbuf) != 0 ) {
        //D( printf("isDir: stat error%d: %s\n", errno, path); )
        return 0;
    }
    return S_ISDIR(statbuf.st_mode);
}

void  vDumpDirTree() {
    printf("%s\n", "Dump of directory tree");
    for (int i =0; i<iDirIdx; i++) {
        printf("DIR[%04d]: %s\n", i, paDirTree[i]);
    }
    printf("\n------------------------------\n");
}


// ==========================================================================
// Function to recursively read files into global list: paFileList[*][1024]
// ==========================================================================
int iGetFileList(const char *directory) {

    DIR *d = opendir(directory);
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL && iFileIdx < maxIdx) {

            // Skip if non-regular file (".", "..")
            if ( ! iIsDirLink(dir)
            &&   ! iIsDirType(dir->d_name) ) {
                vToUpper(dir->d_name);  // in situ conversion
                strcpy(paFileList[iFileIdx++], dir->d_name);
            }
        }
        closedir(d);
        return 1;
    }
    return 0;
}


// Image files, -- not immediately searchable
int isImage(const char *path) {

    if (strstr(path, ".JPG")  != NULL   ||
        strstr(path, ".JPEG") != NULL   ||
        strstr(path, ".BMP")  != NULL   ||
        strstr(path, ".PNG")  != NULL   ||
        strstr(path, ".GIF")  != NULL   ||
        strstr(path, ".SVG")  != NULL   ||
        strstr(path, ".AVI")  != NULL   ||
        strstr(path, ".FIT")  != NULL
        ) return 1;

    return 0;
}

// Binary files, -- not immediately searchable
int isBinary(const char *path) {

    if (strstr(path, ".DOC")  != NULL       ||
        strstr(path, ".DOCX") != NULL       ||
        strstr(path, ".PDF")  != NULL       ||
        strstr(path, ".LNK")  != NULL       ||
        strstr(path, ".WEBSITE")  != NULL   ||
        strstr(path, "HTTP")  != NULL       ||
        strstr(path, ".EXE")  != NULL
        ) return 1;

    return 0;
}

// In situ conversion of string to upper case
void vToUpper(char* string) {
    while (*string++ = toupper(*string));
}



// ==========================================================================
//          Run TBM search on directory <dirName>, file <fileName>.
// ==========================================================================
#define MAXLIN 1024 /* Max. #chars in one line of text */
#define MAXPAT 256  /* Max. #chars in search pattern */

int iSearchText(const char *dirName, const char *fileName) {
    /* File handle for textfile */
    BYTE  cBuf[MAXLIN+MAXPAT]; 	/* Linebuffer for textfile */
    DWORD dwCount = 0L; 		/* Linecount for textlines */
    int   iMatch; 				/* Integer return code (line matches) */
    DWORD dwMatchTotal = 0L;	/* Total number matches (input stream)*/


    /* Open file to search */
    chdir(dirName);
    printf("\n\tInput File: %s\n", fileName);
    FILE *fdTxt = fopen(fileName, "r");
    if (fdTxt == NULL) {
        printf("\tERROR: Function fopen(%s)\n", fileName);
        return 0;
    }
    int iRet = setvbuf(fdTxt, NULL, _IOFBF, 4 * 1024);
    if (iRet != 0) {
        printf("Function setvbuf()");
        return 0;
    }

    /* Perform TBM search on textfile, one line at a time, until EOF */
    while (true) {          /*CONSTEXPR*/

        /* -- Read next input line from file to buffer; - Break at EOF */
        if (fgets((char *) cBuf, MAXLIN, fdTxt) == NULL || feof(fdTxt))
            break;
        dwCount++;

        /* -- Run TBM search on text in buffer; - echo text if match */
        if (iMatch = iRunTBM(cBuf, strlen((char *) cBuf))) {
            printf("\t[%ld:%d]\t%s", dwCount, iMatch, (char *) cBuf);
            dwMatchTotal += iMatch;
        }
    }

    printf("\tTotal match of searchPattern in input stream [%s]: [%ld]\n",
           fileName, dwMatchTotal);

    /* Cleanup & Terminate */
    fclose(fdTxt);
    return 1;
}


// ==========================================================================
// Spawn PowerShell <psScript> to recursively find <cpPattern> in files
// ==========================================================================
int iSearchPS( const char *psScript, const char *dirName, const char *cpPattern )
{
    char cpPS[1024] = "C:/Users/allan/CLionProjects/fileutil/cmake-build-debug";
    strcat(cpPS, psScript);
    char ps[maxIdx];
    snprintf(ps, sizeof ps, "pwsh.exe %s %s %s %s \"%s\"",  \
        "-WorkingDirectory", dirName,                               \
        "-Command ", cpPS,                                          \
        cpPattern);   // Script args

    printf("\nSPAWNING: %s\n", ps);

    const int status = system(ps);      // run the bugger!

    printf("Spawn of %s completed: ", ps);
    printf( status==0 ? "successfully\n" : "***error***\n" );
    return status;
}