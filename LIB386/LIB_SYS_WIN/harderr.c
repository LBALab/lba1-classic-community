/*
 * Platform Abstraction Layer - Windows Critical Error Handling
 * Little Big Adventure (LBA) Classic Community Edition
 */

/* Include Windows headers first to avoid type conflicts */
#include <windows.h>

/*-----------------------------------------------------------------------------
 * Critical Error Handling - Windows Implementation
 * Prevents Windows error dialogs for critical errors like disk failures.
 *---------------------------------------------------------------------------*/

void InstallCriticalErrorHandler(void)
{
	/* Prevent Windows error dialogs for critical errors */
	/* Application will fail silently instead */
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
}

/*-----------------------------------------------------------------------------
 * DOS Compatibility - _harderr stub
 * DOS uses _harderr to install a critical error handler function.
 * On Windows, we use SetErrorMode instead, so this is a no-op.
 *---------------------------------------------------------------------------*/

/* _HARDERR_RETRY constant for compatibility */
#define _HARDERR_RETRY 1

/* Stub for DOS _harderr function - not used on Windows */
void _harderr(void (*handler)(unsigned, unsigned, unsigned *))
{
	/* On Windows, critical errors are handled by SetErrorMode */
	/* This function is provided for source compatibility only */
	(void)handler;
	
	/* Call our Windows error mode setup */
	InstallCriticalErrorHandler();
}
