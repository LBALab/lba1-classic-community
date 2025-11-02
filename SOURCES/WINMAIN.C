/*
 * Windows Entry Point - WinMain wrapper
 * Little Big Adventure (LBA) Classic Community Edition
 *
 * This file provides the WinMain entry point for Windows GUI applications.
 * It converts the Windows entry point to the standard main() function.
 */

#ifdef _WIN32

/* Include Windows headers first */
#include <windows.h>

/* Forward declare main from PERSO.C */
extern void main(int argc, unsigned char *argv[]);

/*
 * WinMain - Windows GUI Application Entry Point
 * 
 * This is the entry point for Windows GUI applications. It converts
 * the Windows-specific parameters to standard argc/argv format and
 * calls the main() function defined in PERSO.C.
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int argc = 1;
	unsigned char *argv[2];
	unsigned char exePath[MAX_PATH];
	
	/* Suppress unused parameter warnings */
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nCmdShow;
	
	/* Get executable path for argv[0] */
	GetModuleFileNameA(NULL, (char*)exePath, MAX_PATH);
	argv[0] = exePath;
	argv[1] = NULL;
	
	/* Call the main function from PERSO.C */
	main(argc, argv);
	
	return 0;
}

#endif /* _WIN32 */
