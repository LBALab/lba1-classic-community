/*===========================================================================
 * SYS_FILESYSTEM.H - Platform-Independent File System API
 * 
 * Provides platform-independent file enumeration and drive management.
 * Platform-specific implementations are in PLATFORM/DOS/LIB_SYS/ etc.
 *===========================================================================*/

#ifndef _FILESYSTEM_PAL_H_
#define _FILESYSTEM_PAL_H_

// Note: This header expects UBYTE, UWORD, ULONG, BYTE, WORD, LONG types
// to be defined elsewhere (e.g., in c_extern.h or DEFINES.H)

/*---------------------------------------------------------------------------
 * File Enumeration
 *---------------------------------------------------------------------------*/

#define SYS_FA_NORMAL   0x00    // Normal file
#define SYS_FA_RDONLY   0x01    // Read-only file
#define SYS_FA_HIDDEN   0x02    // Hidden file
#define SYS_FA_SYSTEM   0x04    // System file
#define SYS_FA_LABEL    0x08    // Volume label
#define SYS_FA_DIREC    0x10    // Directory
#define SYS_FA_ARCH     0x20    // Archive file

// Platform-independent file information
typedef struct {
    char name[260];             // File name
    unsigned long size;         // File size in bytes
    unsigned long wr_date;      // Last write date
    unsigned long wr_time;      // Last write time
    unsigned char attrib;       // File attributes
    void *platform_handle;      // Platform-specific search handle (internal)
} SYS_FileInfo;

// Find first file matching pattern
// Returns: 0 on success, non-zero on error
int SYS_FindFirst(const char *pattern, unsigned attr, SYS_FileInfo *info);

// Find next file in enumeration
// Returns: 0 on success, non-zero on no more files
int SYS_FindNext(SYS_FileInfo *info);

// Close file search (cleanup resources)
void SYS_FindClose(SYS_FileInfo *info);

/*---------------------------------------------------------------------------
 * Drive Management
 *---------------------------------------------------------------------------*/

// Get current drive (1=A:, 2=B:, 3=C:, etc.)
unsigned SYS_GetDrive(void);

// Set current drive
// Returns total number of available drives in *total_drives
void SYS_SetDrive(unsigned drive, unsigned *total_drives);

/*---------------------------------------------------------------------------
 * Disk Space Management
 *---------------------------------------------------------------------------*/

// Get free space on current drive (in bytes)
unsigned long SYS_GetDiskFreeSpace(void);

#endif // _FILESYSTEM_PAL_H_
