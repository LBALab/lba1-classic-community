#ifdef _WIN32

#include <windows.h>


extern void main(int argc, unsigned char *argv[]);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int argc = 1;
	unsigned char *argv[2];
	unsigned char exePath[MAX_PATH];

	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nCmdShow;

	GetModuleFileNameA(NULL, (char*)exePath, MAX_PATH);
	argv[0] = exePath;
	argv[1] = NULL;

	main(argc, argv);
	
	return 0;
}

#endif /* _WIN32 */
