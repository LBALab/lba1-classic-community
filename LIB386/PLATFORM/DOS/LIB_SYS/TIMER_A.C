#include <dos.h>
#include <conio.h>
#include "LIB_SYS/ADELINE.H"
#include "LIB_SYS/LIB_SYS.H"

#define DIVISOR 23864
#define FREQUENCE 50

volatile ULONG TimerRef = 0;
volatile ULONG TimerSystem = 0;
UWORD NbFramePerSecond = 0;
UWORD WaitNbTicks = 1;
UWORD CmptFrame = 0;
UWORD Cmpt_18 = 0;

extern ULONG Old_PM08_Off;
extern WORD Old_PM08_Sel;

void Sys_SetTimer(ULONG counter)
{
	_disable();
	
	outp(0x43, 0x36);
	outp(0x40, (UBYTE)(counter & 0xFF));
	outp(0x40, (UBYTE)((counter >> 8) & 0xFF));
	
	_enable();
}

void _interrupt _far NewIntPM08(void)
{
	UWORD old_cmpt;
	static void (_interrupt _far *old_handler)(void) = NULL;
	
	TimerSystem++;
	TimerRef++;
	
	WaitNbTicks--;
	if (WaitNbTicks == 0) {
		WaitNbTicks = FREQUENCE;
		NbFramePerSecond = CmptFrame;
		CmptFrame = 0;
	}
	
	old_cmpt = Cmpt_18;
	Cmpt_18 += DIVISOR;
	
	if (Cmpt_18 >= old_cmpt) {
		outp(0x20, 0x20);
	} else {
		if (old_handler == NULL) {
			old_handler = (void (_interrupt _far *)())MK_FP(Old_PM08_Sel, Old_PM08_Off);
		}
		_chain_intr(old_handler);
	}
}

void NewProc08(void)
{
	TimerRef++;
	TimerSystem++;
	
	WaitNbTicks--;
	if (WaitNbTicks == 0) {
		WaitNbTicks = FREQUENCE;
		NbFramePerSecond = CmptFrame;
		CmptFrame = 0;
	}
}
