/* MALLOC.C - Cross-platform memory allocation */

#include <stdio.h>
#include <stdlib.h>
#include "ADELINE.H"
#include "LIB_SYS.H"

#ifdef DEBUG_MALLOC
LONG ModeTraceMalloc = FALSE;
#endif

void *Malloc(LONG lenalloc)
{
	void *ptr;
	
	if (lenalloc == -1) {
		return 0; /* Query memory not supported */
	}
	
	ptr = malloc(lenalloc);
	
	if (ptr == NULL) {
		printf("ERROR: MemoryNotAlloc (Malloc): Size = %d/n", lenalloc);
	}
	
	return ptr;
}

void *SmartMalloc(LONG lenalloc)
{
	/* SmartMalloc is just an alias for Malloc */
	return Malloc(lenalloc);
}

void Free(void *buffer)
{
	if (buffer != NULL) {
		free(buffer);
	}
}

void *Mshrink(void *buffer, ULONG taille)
{
	void *new_buffer;
	
	if (buffer == NULL) {
		return NULL;
	}
	
	new_buffer = realloc(buffer, taille);
	return new_buffer ? new_buffer : buffer;
}
