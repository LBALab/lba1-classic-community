#include "DEFINES.H"

extern UBYTE *Log;
extern UBYTE *Phys;

void Mcga_Cls(void)
{
	memset(Log, 0, 64000);
}

void Mcga_Flip(void)
{
	memcpy(Phys, Log, 64000);
}
