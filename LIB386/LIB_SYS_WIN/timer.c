/* Windows Timer Implementation */

#include <windows.h>
#include <mmsystem.h>

/* Timer Global Variables */
volatile unsigned long TimerSystem = 0;
volatile unsigned long TimerRef = 0;
unsigned short NbFramePerSecond = 0;
unsigned short WaitNbTicks = 1;
unsigned short CmptFrame = 0;
unsigned short Cmpt_18 = 0;

static unsigned long timer_start = 0;
static unsigned long last_update = 0;

void InitTimer(void)
{
	timeBeginPeriod(1);
	timer_start = timeGetTime();
	last_update = timer_start;
	
	TimerSystem = 0;
	TimerRef = 0;
	NbFramePerSecond = 0;
	WaitNbTicks = 1;
	CmptFrame = 0;
	Cmpt_18 = 0;
}

void ClearTimer(void)
{
	timeEndPeriod(1);
}

void Sys_SetTimer(unsigned short divisor)
{
	unsigned long current;
	unsigned long elapsed;
	
	(void)divisor;
	
	current = timeGetTime();
	elapsed = current - last_update;
	
	if (elapsed > 0) {
		TimerRef += elapsed;
		TimerSystem += elapsed;
		last_update = current;
	}
}
