#include "LIB_SYS/ADELINE.H"
#include "LIB_SYS/LIB_SYS.H"

void Expand(UBYTE *src, UBYTE *dest, LONG count)
{
	UBYTE *esi, *edi;
	LONG ebp;
	UWORD dx, bx;
	UWORD ax;
	LONG ecx, eax;
	UBYTE *temp;

	esi = src;
	edi = dest;
	ebp = count;

	while (ebp > 0)
	{
		dx = 8;
		bx = *esi++;

		while (dx > 0)
		{
			if (bx & 1)
			{
				*edi++ = *esi++;
				ebp--;
				if (ebp == 0)
				{
					return;
				}
			}
			else
			{
				ax = *((UWORD *)esi);
				esi += 2;

				ecx = ax & 0x0F;
				ecx += 2;
				ebp -= ecx;

				eax = ax >> 4;
				eax = ~eax;
				eax += (LONG)edi;

				temp = esi;
				esi = (UBYTE *)eax;

				while (ecx > 0)
				{
					*edi++ = *esi++;
					ecx--;
				}

				esi = temp;

				if (ebp == 0)
				{
					return;
				}
			}

			bx >>= 1;
			dx--;
		}
	}
}
