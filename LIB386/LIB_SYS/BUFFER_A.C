#include <string.h>
#include "LIB_SYS/ADELINE.H"
#include "LIB_SYS/LIB_SYS.H"

void MovMem(void *pt0, void *pt1, ULONG size)
{
	memcpy(pt1, pt0, size);
}

void RazMem(void *pt, ULONG size)
{
	memset(pt, 0, size);
}

LONG CompBuf(void *pt0, void *pt1, ULONG size)
{
	if (memcmp(pt0, pt1, size) == 0) {
		return 1;
	}
	return 0;
}
