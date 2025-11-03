/*
 * Platform Abstraction Layer - Windows Keyboard Implementation
 * Little Big Adventure (LBA) Classic Community Edition
 */

/* Include Windows headers first to avoid type conflicts */
#include <windows.h>
#include <string.h>

/*-----------------------------------------------------------------------------
 * Keyboard Global Variables (exported to match KEYBOARD.ASM)
 * These are declared as extern in LIB_SYS.H
 *---------------------------------------------------------------------------*/

volatile unsigned short Key = 0;        /* Current scan code */
volatile unsigned short FuncKey = 0;    /* Function key mask */
volatile unsigned short Joy = 0;        /* Joystick emulation (cursor keys) */
volatile unsigned short Fire = 0;       /* Fire button emulation */
unsigned short AsciiMode = 0;           /* ASCII input mode */

/* ASCII translation buffer (used by GetAscii) */
static char ascii_buffer[256];
static int ascii_read_pos = 0;
static int ascii_write_pos = 0;

/*-----------------------------------------------------------------------------
 * Keyboard Platform Interface - Windows Implementation
 *---------------------------------------------------------------------------*/

void InitKeyboard(void)
{
	/* Windows keyboard is handled via message queue - no special init needed */
	/* Initialize globals */
	Key = 0;
	FuncKey = 0;
	Joy = 0;
	Fire = 0;
	AsciiMode = 0;
	
	memset(ascii_buffer, 0, sizeof(ascii_buffer));
	ascii_read_pos = 0;
	ascii_write_pos = 0;
}

void ClearKeyboard(void)
{
	/* No cleanup needed for Windows keyboard */
	Key = 0;
	FuncKey = 0;
	Joy = 0;
	Fire = 0;
}

/*-----------------------------------------------------------------------------
 * ASCII Input Helper (exported for compatibility with DOS version)
 *---------------------------------------------------------------------------*/

/* Get ASCII character from input buffer */
unsigned short GetAscii(void)
{
	if (ascii_read_pos != ascii_write_pos) {
		unsigned short c = (unsigned short)ascii_buffer[ascii_read_pos];
		ascii_read_pos = (ascii_read_pos + 1) % 256;
		return c;
	}
	return 0;
}

/* Add ASCII character to input buffer (called by Windows message loop) */
void PutAscii(char c)
{
	int next_pos = (ascii_write_pos + 1) % 256;
	if (next_pos != ascii_read_pos) {
		ascii_buffer[ascii_write_pos] = c;
		ascii_write_pos = next_pos;
	}
}
