#include <dos.h>
#include <stdio.h>
#include "LIB_SYS/ADELINE.H"
#include "LIB_SYS/LIB_SYS.H"

void Touch(UBYTE *filename)
{
	FILE *fp;
	int handle;
	WORD time_val, date_val;
	union REGS regs;

	time_val = 0;
	date_val = 0;

	fp = OpenRead(filename);
	if (fp == NULL)
	{
		return;
	}

	handle = fileno(fp);

	regs.h.ah = 0x2C;
	int386(0x21, &regs, &regs);

	time_val = (regs.h.dh >> 1);
	time_val |= ((WORD)regs.h.cl << 5);
	time_val |= ((WORD)regs.h.ch << 11);

	regs.h.ah = 0x2A;
	int386(0x21, &regs, &regs);

	date_val = regs.h.dl;
	date_val |= ((WORD)regs.h.dh << 5);
	date_val |= ((WORD)(regs.w.cx - 1980) << 9);

	regs.h.ah = 0x57;
	regs.h.al = 1;
	regs.w.bx = (WORD)handle;
	regs.w.cx = time_val;
	regs.w.dx = date_val;
	int386(0x21, &regs, &regs);

	Close(fp);
}
