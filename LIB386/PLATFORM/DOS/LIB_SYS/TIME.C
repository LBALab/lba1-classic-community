/*===========================================================================
 * TIME.C - DOS Implementation of Time API
 * 
 * DOS-specific implementation using BIOS interrupts
 *===========================================================================*/

#include "lib_sys/sys_time.h"
#include <dos.h>
#include <i86.h>

/*---------------------------------------------------------------------------
 * Time Functions - DOS Implementation
 *---------------------------------------------------------------------------*/

unsigned long SYS_ComputeTime(void)
{
    unsigned long cpttime = 0;
    unsigned long cptdate = 0;
    union REGS regs;
    
    // Get time using DOS interrupt 21h, function 2Ch
    regs.h.ah = 0x2C;
    int386(0x21, &regs, &regs);
    
    // Pack time: hours * 2048 + minutes * 32 + seconds/2
    cpttime = regs.h.ch;        // Hours (0-23)
    cpttime <<= 6;              // * 64
    cpttime |= regs.h.cl;       // Minutes (0-59)
    cpttime <<= 5;              // * 32
    cpttime |= (regs.h.dh >> 1);// Seconds/2 (0-29)
    
    // Get date using DOS interrupt 21h, function 2Ah
    regs.h.ah = 0x2A;
    int386(0x21, &regs, &regs);
    
    // Pack date: (year-1980) * 512 + month * 32 + day
    cptdate = regs.w.cx - 1980; // Year (1980-2099) - use .w for WORD access
    cptdate <<= 4;              // * 16
    cptdate |= regs.h.dh;       // Month (1-12)
    cptdate <<= 5;              // * 32
    cptdate |= regs.h.dl;       // Day (1-31)
    
    return cpttime + cptdate;
}
