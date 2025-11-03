#include "DEFINES.H"

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
#ifdef __DOS__
	ULONG cpttime = 0;
	ULONG cptdate = 0;
	union REGS regs;
	
	regs.h.ah = 0x2C;
	int86(0x21, &regs, &regs);
	cpttime = regs.h.ch;
	cpttime <<= 6;
	cpttime |= regs.h.cl;
	cpttime <<= 5;
	cpttime |= (regs.h.dh >> 1);
	
	regs.h.ah = 0x2A;
	int86(0x21, &regs, &regs);
	cptdate = regs.x.cx - 1980;
	cptdate <<= 4;
	cptdate |= regs.h.dh;
	cptdate <<= 5;
	cptdate |= regs.h.dl;
	
	return cpttime + cptdate;
#else
    return TimerRef;
#endif
}

void SmallSort(void *objetlist, LONG nbobjets, LONG structsize)
{
	UBYTE *pObj;
	UBYTE *pNext;
	UBYTE *pSmallest;
	UBYTE tmp[256];
	LONG n;
	
	if (nbobjets <= 1) {
		return;
	}
	
	pObj = (UBYTE *)objetlist;
	nbobjets--;
	
	while (nbobjets > 0) {
		pSmallest = pObj;
		pNext = pObj + structsize;
		
		for (n = nbobjets; n > 0; n--) {
			if (*((UWORD *)pNext) < *((UWORD *)pSmallest)) {
				pSmallest = pNext;
			}
			pNext += structsize;
		}
		
		if (pSmallest != pObj) {
			memcpy(tmp, pSmallest, structsize);
			memcpy(pSmallest, pObj, structsize);
			memcpy(pObj, tmp, structsize);
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
	
	while (ebx > 0) {
		ecx = eax >> 2;
		while (ecx > 0) {
			*((ULONG *)edi) = *((ULONG *)esi);
			esi += 4;
			edi += 4;
			ecx--;
		}
		
		ecx = eax & 3;
		while (ecx > 0) {
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
	
	if (eax > ClipXmax || ecx < ClipXmin || ebx > ClipYmax || edx < ClipYmin) {
		return;
	}
	
	if (eax < ClipXmin) {
		eax = ClipXmin;
	}
	if (ecx > ClipXmax) {
		ecx = ClipXmax;
	}
	if (ebx < ClipYmin) {
		ebx = ClipYmin;
	}
	if (edx > ClipYmax) {
		edx = ClipYmax;
	}
	
	edi = Log + ((ULONG *)(&TabOffLine))[ebx] + eax;
	
	edx = edx - ebx + 1;
	ebx = edx;
	
	edx = ecx - eax + 1;
	
	esi = Screen_X - edx;
	
	ecx = deccoul;
	
	while (ebx > 0) {
		ebp = edx;
		
		while (ebp > 0) {
			al = *edi;
			ah = al;
			al &= 0x0F;
			ah &= 0xF0;
			
			if (al >= (UBYTE)ecx) {
				al -= (UBYTE)ecx;
				al += ah;
				*edi = al;
			} else {
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
#ifdef __DOS__
	union REGS regs;
	ULONG eax, ebx, ecx;
	
	regs.h.ah = 0x36;
	regs.h.dl = 0;
	int86(0x21, &regs, &regs);
	
	eax = regs.x.ax;
	ebx = regs.x.bx;
	ecx = regs.x.cx;
	
	eax = eax * ebx * ecx;
	
	return eax;
#else
    return 700 * 1024 * 1024;
#endif
}
