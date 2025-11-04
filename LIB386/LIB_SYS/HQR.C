#include "LIB_SYS/ADELINE.H"
#include "LIB_SYS/LIB_SYS.H"

void *HQR_GiveIndex(LONG index, LONG nbindex, void *ptrlist)
{
	T_HQR_BLOC *ptr;
	LONG i;
	
	if (nbindex == 0) {
		return NULL;
	}
	
	ptr = (T_HQR_BLOC *)((UBYTE *)ptrlist + (nbindex * sizeof(T_HQR_BLOC)));
	
	for (i = nbindex; i > 0; i--) {
		ptr--;
		if (ptr->Index == (WORD)index) {
			return ptr;
		}
	}
	
	return NULL;
}
