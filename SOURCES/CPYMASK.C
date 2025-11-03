#include "C_EXTERN.H"

typedef struct {
	UBYTE DeltaX;
	UBYTE DeltaY;
	UBYTE HotX;
	UBYTE HotY;
} Struc_Mask_Header;

void CopyMask(LONG nummask, LONG x, LONG y, void *bankmask, void *src)
{
	ULONG *pTabOffLine;
	ULONG *pBankMask;
	Struc_Mask_Header *maskHeader;
	UBYTE *maskData;
	UINT screenWidth;
	UINT xMin, yMin, xMax, yMax;
	UINT marginTop, marginLeft, marginRight, marginBottom;
	UINT deltaX, deltaY;
	UINT initialOffset;
	UBYTE *screen;
	UBYTE *source;
	UINT lineOffset;
	UINT yIdx, xIdx;
	UBYTE numberOfBlocks;
	UINT numberOfZeroToJump;
	UINT numberOfPixelToCopy;

	pTabOffLine = (ULONG *)&TabOffLine;
	screenWidth = pTabOffLine[1];

	if (y < 0)
		y = 0;

	pBankMask = (ULONG *)bankmask;
	maskHeader = (Struc_Mask_Header *)((UBYTE *)bankmask + pBankMask[nummask]);
	xMin = maskHeader->HotX + x;
	yMin = maskHeader->HotY + y;
	maskData = (UBYTE *)maskHeader + sizeof(Struc_Mask_Header); /* Skip header */

	/* Test Clipping */
	xMax = maskHeader->DeltaX + xMin - 1;
	yMax = maskHeader->DeltaY + yMin - 1;

	marginTop = 0;
	marginLeft = 0;
	marginRight = 0;
	marginBottom = 0;

	if (xMin < ClipXmin || yMin < ClipYmin || xMax > ClipXmax || yMax > ClipYmax)
	{
		/* ClippingMask: */
		if (xMin > ClipXmax || yMin > ClipYmax || xMax < ClipXmin || yMax < ClipYmin)
		{
			return;
		}

		if (yMin < ClipYmin)
		{
			/* Clipping top */
			marginTop = ClipYmin - yMin;
		}
		/* Clipping bottom */
		/* PasHaut: */
		if (yMax > ClipYmax)
		{
			marginBottom = yMax - ClipYmax;
		}
		/* Clipping left */
		/* PasBas: */
		if (xMin < ClipXmin)
		{
			marginLeft = ClipXmin - xMin;
		}
		/* Clipping right */
		/* PasGauche: */
		if (xMax > ClipXmax)
		{
			marginRight = xMax - ClipXmax;
		}
	}

	/* Calculate Offset Screen */
	deltaX = xMax - xMin + 1; /* (deltaX) */
	initialOffset = pTabOffLine[yMin] + xMin;
	screen = (UBYTE *)Log + initialOffset;
	source = (UBYTE *)src + initialOffset;
	deltaY = yMax - yMin + 1; /* NbLine (deltaY) */
	lineOffset = screenWidth - deltaX;
	
	for (yIdx = 0; yIdx < deltaY; yIdx++)
	{
		/* NextLine: */
		numberOfBlocks = *maskData; /* Nb Block for this line */
		maskData++;
		xIdx = 0;

		do
		{
			/* SameLine: */
			numberOfZeroToJump = *maskData; /* Nb Zero to Jump */
			maskData++;
			screen += numberOfZeroToJump; /* Incrust on Log */
			source += numberOfZeroToJump; /* And on PtSrc */
			numberOfBlocks--;
			if (numberOfBlocks == 0)
			{
				break;
			}

			xIdx += numberOfZeroToJump;

			numberOfPixelToCopy = *maskData; /* Nb Zero to Write */
			maskData++;
			do
			{
				/* loopb: */
				if (xIdx >= marginLeft && xIdx < deltaX - marginRight && yIdx >= marginTop && yIdx < deltaY - marginBottom)
				{
					*screen = *source;
				}
				xIdx++;
				source++;
				screen++;
				--numberOfPixelToCopy;
			} while (numberOfPixelToCopy);
			numberOfBlocks--;
		} while (numberOfBlocks);

		screen += lineOffset;
		source += lineOffset;
	}
}
