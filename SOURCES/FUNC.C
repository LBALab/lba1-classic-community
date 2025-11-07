#include "DEFINES.H"
#include "../LIB386/LIB_SYS/SYS_TIME.H"
#include "../LIB386/LIB_SYS/SYS_FILESYSTEM.H"

extern ULONG TabOffLine;
extern UBYTE *Screen;
extern UBYTE *Log;
extern WORD ClipXmin;
extern WORD ClipYmin;
extern WORD ClipXmax;
extern WORD ClipYmax;
extern WORD Screen_X;

static UBYTE IndexCoul0 = 4 * 16;
static UBYTE PalRed = 0;
static UBYTE PalGreen = 0;
static UBYTE PalBlue = 0;
static UBYTE PalXor = 0;

ULONG ComputeTime(void)
{
	return SYS_ComputeTime();
}

void SmallSort(void *objetlist, LONG nbobjets, LONG structsize)
{
	UBYTE *pObj;
	UBYTE *pNext;
	UBYTE *pSmallest;
	ULONG tmp[64]; // Changed from UBYTE tmp[256] - swap uses DWORDs
	LONG n;
	LONG numDwords;
	ULONG *pSrc, *pDest, *pTmp;

	if (nbobjets <= 1)
	{
		return;
	}

	pObj = (UBYTE *)objetlist;
	nbobjets--;
	numDwords = structsize >> 2; // structsize / 4

	while (nbobjets > 0)
	{
		pSmallest = pObj;
		pNext = pObj + structsize;

		for (n = nbobjets; n > 0; n--)
		{
			if (*((WORD *)pNext) < *((WORD *)pSmallest))
			{ // SIGNED comparison (jl)
				pSmallest = pNext;
			}
			pNext += structsize;
		}

		if (pSmallest != pObj)
		{
			// Swap using DWORDs like the assembly version
			pSrc = (ULONG *)pSmallest;
			pDest = (ULONG *)pObj;
			pTmp = tmp;

			for (n = numDwords; n > 0; n--)
			{
				*pTmp = *pSrc;
				*pSrc = *pDest;
				*pDest = *pTmp;
				pSrc++;
				pDest++;
				pTmp++;
			}
		}

		pObj += structsize;
		nbobjets--;
	}
}

void CopyBlockMCGA(LONG x0, LONG y0, LONG x1, LONG y1, UBYTE *src, LONG xd, LONG yd, UBYTE *dst)
{
	UBYTE *esi, *edi;
	LONG ebx, eax, edx, ebp;
	LONG ecx;

	ebx = y0;
	esi = src + ((ULONG *)(&TabOffLine))[ebx] + x0;

	edi = dst;

	ebx = y1 - y0 + 1;
	eax = x1 - x0 + 1;

	edx = 320 - eax;
	ebp = 640 - eax;

	while (ebx > 0)
	{
		ecx = eax >> 2;
		while (ecx > 0)
		{
			*((ULONG *)edi) = *((ULONG *)esi);
			esi += 4;
			edi += 4;
			ecx--;
		}

		ecx = eax & 3;
		while (ecx > 0)
		{
			*edi = *esi;
			esi++;
			edi++;
			ecx--;
		}

		esi += ebp;
		edi += edx;
		ebx--;
	}
}

void ShadeBox(LONG x0, LONG y0, LONG x1, LONG y1, LONG deccoul)
{
	UBYTE *edi;
	LONG eax, ebx, ecx, edx;
	LONG esi, ebp;
	UBYTE al, ah;

	eax = x0;
	ebx = y0;
	ecx = x1;
	edx = y1;

	if (eax > ClipXmax || ecx < ClipXmin || ebx > ClipYmax || edx < ClipYmin)
	{
		return;
	}

	if (eax < ClipXmin)
	{
		eax = ClipXmin;
	}
	if (ecx > ClipXmax)
	{
		ecx = ClipXmax;
	}
	if (ebx < ClipYmin)
	{
		ebx = ClipYmin;
	}
	if (edx > ClipYmax)
	{
		edx = ClipYmax;
	}

	edi = Log + ((ULONG *)(&TabOffLine))[ebx] + eax;

	edx = edx - ebx + 1;
	ebx = edx;

	edx = ecx - eax + 1;

	esi = Screen_X - edx;

	ecx = deccoul;

	while (ebx > 0)
	{
		ebp = edx;

		while (ebp > 0)
		{
			al = *edi;
			ah = al;
			al &= 0x0F;
			ah &= 0xF0;

			if (al >= (UBYTE)ecx)
			{
				al -= (UBYTE)ecx;
				al += ah;
				*edi = al;
			}
			else
			{
				*edi = ah;
			}

			edi++;
			ebp--;
		}

		edi += esi;
		ebx--;
	}
}

ULONG GetHDFreeSize(void)
{
	return SYS_GetDiskFreeSpace();
}
