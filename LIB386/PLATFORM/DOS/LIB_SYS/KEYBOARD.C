#include <dos.h>
#include <conio.h>

#include "LIB_SYS/ADELINE.H"
#include "LIB_SYS/LIB_SYS.H"

#define KB_DATA 0x60

#define NB_SPECIAL_KEY 29

WORD _Key = 0;
volatile UWORD Key = 0;
volatile UWORD FuncKey = 0;
volatile UWORD Joy = 0;
volatile UWORD Fire = 0;
UWORD AsciiMode = 0;

ULONG Old_PM09_Off = 0;
WORD Old_PM09_Sel = 0;

static UBYTE NoAscii = 0;
static UBYTE FlagExtendScan = 0;
static void(_interrupt _far *Old_Int_09)(void);

static const UBYTE TabSpecialKey[NB_SPECIAL_KEY] = {
	72, /* UP */
	80, /* DOWN */
	75, /* LEFT */
	77, /* RIGHT */
	71, /* UP LEFT */
	73, /* UP RIGHT */
	81, /* DOWN RIGHT */
	79, /* DOWN LEFT */
	57, /* space */
	28, /* return */
	29, /* CTRL */
	56, /* ALT */
	83, /* SUPPR */
	42, /* SHIFT_LEFT */
	54, /* SHIFT_RIGHT */
	59, /* F1 */
	60, /* F2 */
	61, /* F3 */
	62, /* F4 */
	63, /* F5 */
	64, /* F6 */
	65, /* F7 */
	66, /* F8 */
	67, /* F9 */
	68, /* F10 */
	87, /* F11 */
	88, /* F12 */
	42,
	0};

static const UBYTE TabSpecialFunc[NB_SPECIAL_KEY * 2] = {
	0, 1,	  /* J_UP */
	0, 2,	  /* J_DOWN */
	0, 4,	  /* J_LEFT */
	0, 8,	  /* J_RIGHT */
	0, 1 + 4, /* UP LEFT */
	0, 1 + 8, /* UP RIGHT */
	0, 2 + 8, /* DOWN RIGHT */
	0, 2 + 4, /* DOWN LEFT */
	1, 1,	  /* Fire */
	1, 2,	  /* return */
	1, 4,	  /* ctrl */
	1, 8,	  /* alt */
	1, 16,	  /* suppr */
	1, 32,	  /* shift left */
	1, 32,	  /* shift right */
	2, 1,	  /* F1 */
	2, 2,	  /* F2 */
	2, 4,	  /* F3 */
	2, 8,	  /* F4 */
	2, 16,	  /* F5 */
	2, 32,	  /* F6 */
	2, 64,	  /* F7 */
	2, 128,	  /* F8 */
	3, 1,	  /* F9 */
	3, 2,	  /* F10 */
	3, 4,	  /* F11 */
	3, 8,	  /* F12 */
	255, 0,	  /* ignore 42 */
	255, 0	  /* ignore 0 */
};

void _interrupt _far NewIntPM09(void)
{
	UBYTE al, ah;
	LONG i;
	UBYTE bl, bh;

	al = inp(KB_DATA);
	ah = al;
	_Key = ah;

	if (al == 224)
	{
		FlagExtendScan = 1;
		outp(0x20, 0x20);
		return;
	}

	al &= 127;

	if (al == 0x2A)
	{
		if (FlagExtendScan == 1)
		{
			FlagExtendScan = 0;
			outp(0x20, 0x20);
			return;
		}
	}

	for (i = 0; i < NB_SPECIAL_KEY; i++)
	{
		if (TabSpecialKey[i] == al)
		{
			bl = TabSpecialFunc[i * 2];
			bh = TabSpecialFunc[i * 2 + 1];

			if (bl == 0)
			{
				if (!(ah & 128))
				{
					Joy |= bh;
				}
				else
				{
					Joy &= ~bh;
				}
			}
			else if (bl == 1)
			{
				if (!(ah & 128))
				{
					Fire |= bh;

					if ((Fire & (4 + 8 + 16)) == (4 + 8 + 16))
					{
						*((WORD *)0x472) = 0x1234;
						_asm {
							_asm push 0xFFFF
							_asm push 0x0000
							_asm retf
						}
					}
				}
				else
				{
					Fire &= ~bh;
				}
			}
			else if (bl == 2)
			{
				if (!(ah & 128))
				{
					*((UBYTE *)&FuncKey) |= bh;
				}
				else
				{
					*((UBYTE *)&FuncKey) &= ~bh;
				}
			}
			else if (bl == 3)
			{
				if (!(ah & 128))
				{
					*((UBYTE *)&FuncKey + 1) |= bh;
				}
				else
				{
					*((UBYTE *)&FuncKey + 1) &= ~bh;
				}
			}

			FlagExtendScan = 0;
			outp(0x20, 0x20);
			return;
		}
	}

	if (!(ah & 128))
	{
		Key = al;
	}
	else
	{
		Key = 0;
	}

	FlagExtendScan = 0;

	if (AsciiMode != 0)
	{
		_chain_intr(Old_Int_09);
	}
	else
	{
		outp(0x20, 0x20);
	}
}

UWORD GetAscii(void)
{
	union REGS regs;

	regs.h.ah = 1;
	int386(0x16, &regs, &regs);

	if (regs.x.cflag)
	{
		return 0;
	}

	regs.h.ah = 0;
	int386(0x16, &regs, &regs);

	return regs.w.ax;
}

void ClearAsciiBuffer(void)
{
	union REGS regs;

	do
	{
		regs.h.ah = 1;
		int386(0x16, &regs, &regs);

		if (regs.x.cflag)
		{
			break;
		}

		regs.h.ah = 0;
		int386(0x16, &regs, &regs);
	} while (1);
}
