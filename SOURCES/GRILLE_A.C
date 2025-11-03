#include "DEFINES.H"

extern LONG XMap;
extern LONG YMap;
extern LONG ZMap;
extern LONG XScreen;
extern LONG YScreen;

extern UBYTE *BufCube;
extern UBYTE *BufMap;
extern UBYTE *TabBlock;
extern UBYTE *BufferBrick;

UBYTE CodeJeu = 0;

#define HEADER_BLOCK 3

#define SIZE_CUBE_X 64
#define SIZE_CUBE_Y 25
#define SIZE_CUBE_Z 64

#define SIZE_BRICK_XZ 512
#define SIZE_BRICK_Y 256
#define DEMI_BRICK_XZ 256
#define DEMI_BRICK_Y 128

UBYTE *GetAdrBlock(LONG numblock)
{
	ULONG *pto;
	
	pto = (ULONG *)TabBlock;
	return TabBlock + pto[numblock];
}

UBYTE WorldColBrickFull(LONG xw, LONG yw, LONG zw, LONG ymax)
{
	UBYTE *ptc;
	LONG xm, ym, zm;
	LONG eax, edx;
	UBYTE al;
	UBYTE *block_adr;
	LONG height_check;
	
	xm = (xw + DEMI_BRICK_XZ) >> 9;
	ym = yw >> 8;
	zm = (zw + DEMI_BRICK_XZ) >> 9;
	
	XMap = xm;
	YMap = ym;
	ZMap = zm;
	
	if (xm < 0 || xm >= 64 || zm < 0 || zm >= 64) {
		return 0;
	}
	
	ptc = BufCube;
	ptc += xm * SIZE_CUBE_Y * 2;
	
	if (ym <= -1) {
		return 1;
	}
	
	ptc += ym * 2;
	ptc += zm * SIZE_CUBE_X * SIZE_CUBE_Y * 2;
	
	al = ptc[0];
	
	if (al != 0) {
		ULONG *pto = (ULONG *)TabBlock;
		block_adr = TabBlock + pto[al - 1];
		block_adr += HEADER_BLOCK;
		block_adr += ptc[1] * 4;
		al = block_adr[0];
		
		height_check = (ymax + 255) >> 8;
		
		while (ym < 24 && height_check > 0) {
			ptc += 2;
			ym++;
			if (*((UWORD *)ptc) != 0) {
				return 1;
			}
			height_check--;
		}
		
		return al;
	} else {
		al = ptc[1];
		
		height_check = (ymax + 255) >> 8;
		
		while (ym < 24 && height_check > 0) {
			ptc += 2;
			ym++;
			if (*((UWORD *)ptc) != 0) {
				return 1;
			}
			height_check--;
		}
		
		return al;
	}
}

UBYTE WorldColBrick(LONG xw, LONG yw, LONG zw)
{
	UBYTE *ptc;
	LONG xm, ym, zm;
	UBYTE al;
	UBYTE *block_adr;
	
	xm = (xw + DEMI_BRICK_XZ) >> 9;
	ym = yw >> 8;
	zm = (zw + DEMI_BRICK_XZ) >> 9;
	
	XMap = xm;
	YMap = ym;
	ZMap = zm;
	
	if (xm < 0 || xm >= 64 || zm < 0 || zm >= 64) {
		return 0;
	}
	
	ptc = BufCube;
	ptc += xm * SIZE_CUBE_Y * 2;
	
	if (ym <= -1) {
		return 1;
	}
	
	if (ym < 0 || ym > 24) {
		return 0;
	}
	
	ptc += ym * 2;
	ptc += zm * SIZE_CUBE_X * SIZE_CUBE_Y * 2;
	
	al = ptc[0];
	
	if (al != 0) {
		ULONG *pto = (ULONG *)TabBlock;
		block_adr = TabBlock + pto[al - 1];
		block_adr += HEADER_BLOCK;
		block_adr += ptc[1] * 4;
		al = block_adr[0];
		return al;
	} else {
		al = ptc[1];
		return al;
	}
}

UBYTE WorldCodeBrick(LONG xw, LONG yw, LONG zw)
{
	UBYTE *ptc;
	LONG xm, ym, zm;
	UBYTE al;
	UBYTE *block_adr;
	
	xm = (xw + DEMI_BRICK_XZ) >> 9;
	XMap = xm;
	
	ptc = BufCube;
	ptc += xm * SIZE_CUBE_Y * 2;
	
	if (yw <= -1) {
		return 0xF0;
	}
	
	ym = yw >> 8;
	YMap = ym;
	ptc += ym * 2;
	
	zm = (zw + DEMI_BRICK_XZ) >> 9;
	ZMap = zm;
	
	ptc += zm * SIZE_CUBE_X * SIZE_CUBE_Y * 2;
	
	al = ptc[0];
	
	if (al != 0) {
		ULONG *pto = (ULONG *)TabBlock;
		block_adr = TabBlock + pto[al - 1];
		block_adr += HEADER_BLOCK;
		al = block_adr[1];
		return al;
	}
	
	return 0xF0;
}

void Map2Screen(LONG xm, LONG ym, LONG zm)
{
	LONG eax, edx;
	
	eax = xm - zm;
	edx = eax;
	eax = (eax << 4) + (edx << 3) + 320 - 8 - 23 - 1;
	XScreen = eax;
	
	eax = xm + zm;
	edx = eax;
	eax = (eax << 3) + (edx << 2);
	edx = ym;
	edx = (edx << 4) - edx;
	eax = eax - edx + 240 - 25;
	YScreen = eax;
}

void DecompColonne(UBYTE *pts, UWORD *ptd)
{
	UBYTE bl, al, cl;
	UWORD ax;
	
	bl = *pts++;
	
	while (bl > 0) {
		al = *pts++;
		cl = (al & 0x3F) + 1;
		
		if ((al & 0xC0) == 0) {
			ax = 0;
			while (cl > 0) {
				*ptd++ = ax;
				cl--;
			}
		} else if (al & 0x40) {
			while (cl > 0) {
				*ptd++ = *((UWORD *)pts);
				pts += 2;
				cl--;
			}
		} else {
			ax = *((UWORD *)pts);
			pts += 2;
			while (cl > 0) {
				*ptd++ = ax;
				cl--;
			}
		}
		
		bl--;
	}
}

void MixteColonne(UBYTE *pts, UWORD *ptd)
{
	UBYTE bl, al, cl;
	UWORD ax;
	
	bl = *pts++;
	
	while (bl > 0) {
		al = *pts++;
		cl = (al & 0x3F) + 1;
		
		if ((al & 0xC0) == 0) {
			ptd += cl;
		} else if (al & 0x40) {
			while (cl > 0) {
				*ptd++ = *((UWORD *)pts);
				pts += 2;
				cl--;
			}
		} else {
			ax = *((UWORD *)pts);
			pts += 2;
			while (cl > 0) {
				*ptd++ = ax;
				cl--;
			}
		}
		
		bl--;
	}
}
