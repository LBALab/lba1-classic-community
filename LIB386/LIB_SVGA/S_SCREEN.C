#include "LIB_SYS/ADELINE.H"
#include "LIB_SYS/LIB_SYS.H"
#include "LIB_SVGA/LIB_SVGA.H"

void Cls(void)
{
	memset(Log, 0, Screen_X * Screen_Y);
}

void CopyScreen(void *src, void *dst)
{
	memcpy(dst, src, Screen_X * Screen_Y);
}
