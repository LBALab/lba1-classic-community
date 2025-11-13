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
		// Chain to old interrupt handler without sending EOI
		// This allows the old handler to process the keystroke and put it in BIOS buffer
		// The old handler will send the EOI
		if (Old_PM09_Sel != 0 && Old_PM09_Off != 0)
		{
			// Use inline assembly to chain exactly like the original ASM code
			// The old handler expects to be jumped to, not called
			_asm {
				// Set up far pointer to old handler
				mov ecx, [Old_PM09_Off]
				mov ax, [Old_PM09_Sel]
				// Stack manipulation to set up far return address
				// [esp+4*4] = EIP (return offset)
				// [esp+5*4] = CS (return selector)
				xchg ecx, [esp+4*4]
				xchg eax, [esp+5*4]
				// Now stack has old handler address as return address
			}
			// Function will return with retf, jumping to old handler
			return;
		}
		else
		{
			outp(0x20, 0x20);
		}
	}
	else
	{
		outp(0x20, 0x20);
	}
}

UWORD GetAscii(void)
{
	UWORD result = 0;
	
	// Replicate the original assembly exactly
	// INT 16h is reflected by DOS/4GW from protected mode to real mode
	_asm {
		mov ah, 1        // Function 1: Check for keystroke
		int 16h          // Call BIOS - sets ZF if no key
		jz nokey         // Jump if zero flag set (no key)
		mov ah, 0        // Function 0: Read keystroke  
		int 16h          // Call BIOS - returns key in AX
		mov result, ax   // Store result
	nokey:
	}
	
	return result;  // Returns full 16-bit value: AH=scan code, AL=ASCII
}

void ClearAsciiBuffer(void)
{
	// Use conio.h functions which work properly in protected mode
	while (kbhit())
	{
		getch();
	}
}
