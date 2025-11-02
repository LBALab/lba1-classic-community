/*
 * Platform Abstraction Layer - Windows Memory Implementation
 * Little Big Adventure (LBA) Classic Community Edition
 */

/* Include Windows headers first to avoid type conflicts */
#include <windows.h>

/*-----------------------------------------------------------------------------
 * DOS Memory Management - Windows Implementation
 * These functions allocate DOS conventional memory (below 1MB).
 * On Windows, these are not supported and return NULL.
 *---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * Memory Platform Interface - Windows Implementation
 * See lib_sys/lib_sys_platform.h for function documentation
 *---------------------------------------------------------------------------*/

void* DosMalloc(LONG size, ULONG *handle)
{
	/* DOS memory below 1MB is not available on Windows */
	/* Return NULL to indicate failure */
	(void)size;
	(void)handle;
	return NULL;
}

void DosFree(ULONG handle)
{
	/* No-op on Windows */
	(void)handle;
}
