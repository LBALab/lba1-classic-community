/*
			FILES (c) Adeline 1993

		Functions:

			- OpenRead
			- OpenWrite
			- OpenReadWrite
			- Read
			- Write
			- Close
			- Seek
			- Delete
			- FileSize
			- AddExt
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "ADELINE.H"
#include "LIB_SYS.H"

/*--------------------------------------------------------------------------*/
FILE *OpenRead(char *name)
{
	FILE *fp;
	fp = fopen(name, "rb");
	return fp;
}
/*--------------------------------------------------------------------------*/
FILE *OpenWrite(char *name)
{
	FILE *fp;
	fp = fopen(name, "wb");
	return fp;
}
/*--------------------------------------------------------------------------*/
FILE *OpenReadWrite(char *name)
{
	FILE *fp;
	fp = fopen(name, "rb+");
	return fp;
}
/*--------------------------------------------------------------------------*/
ULONG Read(FILE *handle, void *buffer, ULONG lenread)
{
	ULONG howmuch;

	if (lenread == 0xFFFFFFFFL) /*	-1L	*/
		lenread = 16000000L;	/* Ca Accelere !! 	*/
	howmuch = fread(buffer, lenread, 1, handle);
	return (howmuch);
}
/*--------------------------------------------------------------------------*/
ULONG Write(FILE *handle, void *buffer, ULONG lenwrite)
{
	ULONG howmuch;
	howmuch = fwrite(buffer, lenwrite, 1, handle);
	return (howmuch);
}
/*--------------------------------------------------------------------------*/
void Close(FILE *handle)
{
	fclose(handle);
}
/*--------------------------------------------------------------------------*/
LONG Seek(FILE *handle, LONG position, LONG mode)
{
	return (fseek(handle, position, mode));
}
/*--------------------------------------------------------------------------*/
LONG Delete(char *name)
{
	if (remove(name))
		return (0);
	return (1);
}
/*--------------------------------------------------------------------------*/
ULONG FileSize(char *name)
{
	FILE *handle;
	ULONG fsize;

	handle = OpenRead(name);
	if (handle == 0)
		return (0);

	fseek(handle, 0L, SEEK_END);
	fsize = ftell(handle);

	Close(handle);
	return (fsize);
}
/*-------------------------------------------------------------------------*/
void AddExt(char *path, char *ext)
{
	//	Version Loran

	/*
		char	*pt	;
		pt = path	;
		while(( *pt != '.' ) AND ( *pt != 0 ))	pt++	;
		*pt = 0		;
		strcat( path, ext )	;
	*/

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char name[_MAX_FNAME];
	char oldext[_MAX_EXT];

	_splitpath(path, drive, dir, name, oldext);

	// makepath rajoute le point si necessaire

	// oldext[0] = '.' ;
	// if (*ext == '.' ) ext++ ;
	// strcpy(oldext+1, ext);
	// _makepath( path, drive, dir, name, oldext ) ;

	_makepath(path, drive, dir, name, ext);
}
/*--------------------------------------------------------------------------*/
LONG Copy(UBYTE *sname, UBYTE *dname)
{
	ULONG n, size;
	FILE *shandle;
	FILE *dhandle;
	UBYTE c;

	size = FileSize(sname);
	if (!size)
		return 0L;

	shandle = OpenRead(sname);
	if (!shandle)
		return 0L;

	dhandle = OpenWrite(dname);
	if (!dhandle)
	{
		Close(shandle);
		return 0L;
	}

	for (n = 0; n < size; n++)
	{
		Read(shandle, &c, 1L);
		if (Write(dhandle, &c, 1L) != 1L)
		{
			Close(shandle);
			Close(dhandle);
			return 0L;
		}
	}

	Close(shandle);
	Close(dhandle);

	return 1L;
}
/*--------------------------------------------------------------------------*/

LONG CopyBak(UBYTE *name)
{
	UBYTE string[256];

	strcpy(string, name);
	AddExt(string, ".BAK");
	return Copy(name, string);
}

/*--------------------------------------------------------------------------*/

LONG Exists(UBYTE *name)
{
	FILE *handle;

	handle = OpenRead(name);
	if (handle)
	{
		Close(handle);
		return 1L;
	}
	return 0L;
}
