/*===========================================================================
 * SYS_TIME.H - Platform-Independent Time API
 * 
 * Provides platform-independent time and date functions.
 * Platform-specific implementations are in PLATFORM/DOS/LIB_SYS/ etc.
 *===========================================================================*/

#ifndef _SYS_TIME_PAL_H_
#define _SYS_TIME_PAL_H_

// Note: This header expects UBYTE, UWORD, ULONG, BYTE, WORD, LONG types
// to be defined elsewhere (e.g., in c_extern.h or DEFINES.H)

/*---------------------------------------------------------------------------
 * Time Functions
 *---------------------------------------------------------------------------*/

// Compute a timestamp combining date and time
// Used for file timestamps and save game ordering
// Returns: Packed timestamp value (format: platform-specific but consistent)
unsigned long SYS_ComputeTime(void);

#endif // _SYS_TIME_PAL_H_
