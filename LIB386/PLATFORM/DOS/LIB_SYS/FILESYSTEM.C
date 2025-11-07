/*===========================================================================
 * FILESYSTEM.C - DOS Implementation of File System API
 * 
 * DOS-specific implementation using _dos_findfirst/_dos_findnext
 *===========================================================================*/

#include "lib_sys/sys_filesystem.h"
#include <dos.h>
#include <i86.h>
#include <string.h>
#include <stdlib.h>

/*---------------------------------------------------------------------------
 * File Enumeration - DOS Implementation
 *---------------------------------------------------------------------------*/

int SYS_FindFirst(const char *pattern, unsigned attr, SYS_FileInfo *info)
{
    struct find_t *dos_info;
    int rc;
    
    if (!info) return -1;
    
    // Allocate DOS find_t structure
    dos_info = (struct find_t *)malloc(sizeof(struct find_t));
    if (!dos_info) return -1;
    
    // Call DOS findfirst
    rc = _dos_findfirst(pattern, attr, dos_info);
    
    if (rc == 0) {
        // Success - copy to platform-independent structure
        strncpy(info->name, dos_info->name, sizeof(info->name) - 1);
        info->name[sizeof(info->name) - 1] = '\0';
        info->size = dos_info->size;
        info->wr_date = dos_info->wr_date;
        info->wr_time = dos_info->wr_time;
        info->attrib = dos_info->attrib;
        info->platform_handle = dos_info;
    } else {
        // Failed - cleanup
        free(dos_info);
        info->platform_handle = NULL;
    }
    
    return rc;
}

int SYS_FindNext(SYS_FileInfo *info)
{
    struct find_t *dos_info;
    int rc;
    
    if (!info || !info->platform_handle) return -1;
    
    dos_info = (struct find_t *)info->platform_handle;
    
    // Call DOS findnext
    rc = _dos_findnext(dos_info);
    
    if (rc == 0) {
        // Success - copy to platform-independent structure
        strncpy(info->name, dos_info->name, sizeof(info->name) - 1);
        info->name[sizeof(info->name) - 1] = '\0';
        info->size = dos_info->size;
        info->wr_date = dos_info->wr_date;
        info->wr_time = dos_info->wr_time;
        info->attrib = dos_info->attrib;
    }
    
    return rc;
}

void SYS_FindClose(SYS_FileInfo *info)
{
    if (info && info->platform_handle) {
        free(info->platform_handle);
        info->platform_handle = NULL;
    }
}

/*---------------------------------------------------------------------------
 * Drive Management - DOS Implementation
 *---------------------------------------------------------------------------*/

unsigned SYS_GetDrive(void)
{
    unsigned drive;
    _dos_getdrive(&drive);
    return drive;
}

void SYS_SetDrive(unsigned drive, unsigned *total_drives)
{
    unsigned total;
    _dos_setdrive(drive, &total);
    if (total_drives) {
        *total_drives = total;
    }
}

/*---------------------------------------------------------------------------
 * Disk Space Management - DOS Implementation
 *---------------------------------------------------------------------------*/

unsigned long SYS_GetDiskFreeSpace(void)
{
    union REGS regs;
    unsigned long eax, ebx, ecx;
    
    // Get disk free space using DOS interrupt 21h, function 36h
    regs.h.ah = 0x36;
    regs.h.dl = 0;  // Default drive
    int386(0x21, &regs, &regs);
    
    eax = regs.w.ax;  // Sectors per cluster - use .w for WORD access
    ebx = regs.w.bx;  // Available clusters
    ecx = regs.w.cx;  // Bytes per sector
    
    // Calculate free space: sectors * clusters * bytes
    eax = eax * ebx * ecx;
    
    return eax;
}
