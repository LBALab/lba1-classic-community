#include "DEFINES.H"

extern UBYTE *Log;
extern ULONG TabOffLine;
extern UBYTE *BufSpeak;

static UBYTE *Imagen = NULL;
static UBYTE *Imagen2 = NULL;

void ComputeFire(void)
{
	UBYTE *esi, *edi;
	WORD cx;
	UWORD ax;
	UBYTE al;
	ULONG edx;
	
	Imagen = BufSpeak;
	Imagen2 = BufSpeak + (320 * 50);
	
	esi = Imagen + 321;
	edi = Imagen2 + 321;
	
	cx = 48 * 320 - 2;
	
	while (cx > 0) {
		ax = 0;
		ax += esi[-1];
		ax += esi[-320];
		ax += esi[-319];
		ax += esi[-321];
		ax += esi[1];
		ax += esi[320];
		ax += esi[319];
		ax += esi[321];
		
		ax = (ax >> 3) | ((ax & 0x07) << 13);
		
		if (!(ax & 0x6500)) {
			edx = (ULONG)(Imagen2 + 46 * 320);
			
			if ((ax & 0xFF) || ((ULONG)edi >= edx)) {
				ax--;
			}
		}
		
		*edi = (UBYTE)(ax & 0xFF);
		
		esi++;
		edi++;
		cx--;
	}
	
	{
		UBYTE *src = Imagen2 + 320;
		UBYTE *dst = Imagen;
		ULONG count = 160 * 48 / 2;
		ULONG *src32 = (ULONG *)src;
		ULONG *dst32 = (ULONG *)dst;
		
		while (count > 0) {
			*dst32++ = *src32++;
			count--;
		}
	}
	
	{
		esi = Imagen2 + 320 * 39;
		cx = 5 * 320;
		
		while (cx > 0) {
			al = *esi;
			
			if (al <= 15) {
				al = 11 - al;
			}
			
			*esi = al;
			esi++;
			cx--;
		}
	}
}

void DoFire(LONG lig, LONG coul)
{
	UBYTE *esi;
	UWORD *edi;
	WORD dx, cx;
	UBYTE bl, bh, al, ah;
	
	ComputeFire();
	
	esi = Imagen + 5 * 320;
	edi = (UWORD *)(Log + ((ULONG*)(&TabOffLine))[lig]);
	
	bl = (UBYTE)coul;
	bh = bl + 15;
	
	dx = 25;
	
	while (dx > 0) {
		cx = 320;
		
		while (cx > 0) {
			al = *esi;
			al = al >> 1;
			al += bl;
			
			if (al > bh) {
				al = bh;
			}
			
			ah = al;
			*edi = (UWORD)((ah << 8) | al);
			edi[320] = (UWORD)((ah << 8) | al);
			
			esi++;
			edi++;
			cx--;
		}
		
		edi += 320;
		dx--;
	}
}
