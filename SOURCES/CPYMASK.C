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
	// ULONG* pOffset = &((ULONG*)bank)[num];
	// UBYTE* pMask = (UBYTE*)bank + *pOffset;
	// UBYTE *pSrc = (UBYTE *)screen;
	// UBYTE *pDest;
	// LONG dx, dy;
	// LONG x1, y1;
	// LONG n;
	// UBYTE NbBlock;

	// dx = pMask[0];
	// dy = pMask[1];
	// x += pMask[2]; // Hot X
	// y += pMask[3]; // Hot Y

	// pMask += 4;

	// // test clipping
	// x1 = dx + x - 1;
	// y1 = dy + y - 1;

	// if ((x < ClipXmin) || (y < ClipYmin) || (x1 > ClipXmax) || (y1 > ClipYmax))
	// {
	// 	UBYTE *pSrcLine;
	// 	UBYTE *pDestLine;
	// 	LONG OffsetBegin = 0;
	// 	LONG NbPix, pixLeft, offset;
	// 	LBARect brickData;

	// 	// clipped
	// 	if ((x > ClipXmax) || (y > ClipYmax) || (x1 < ClipXmin) || (y1 < ClipYmin))
	// 	{
	// 		// outside clipping area
	// 		return;
	// 	}

	// 	if (y < ClipYmin)
	// 	{
	// 		// Clipping Haut, Saute ClipYmin-y Line(s)
	// 		for (y = ClipYmin - y; y > 0; --y)
	// 		{
	// 			NbBlock = *pMask;
	// 			pMask += NbBlock + 1;
	// 		}
	// 		y = ClipYmin;
	// 	}

	// 	if (y1 > ClipYmax)
	// 	{
	// 		// Clipping Bas
	// 		y1 = ClipYmax;
	// 	}

	// 	if (x < ClipXmin)
	// 	{
	// 		// Clipping Gauche
	// 		OffsetBegin = ClipXmin - x;
	// 	}

	// 	NbPix = x1 - x - OffsetBegin + 1;
	// 	if (x1 > ClipXmax)
	// 	{
	// 		// Clipping Droit
	// 		NbPix -= x1 - ClipXmax;
	// 	}

	// 	n = GetTabOffLine(y) + x + OffsetBegin;
	// 	pSrcLine = pSrc + n;
	// 	pDestLine = Log + n;

	// 	int i = 0;
	// 	for (dy = y1 - y + 1; dy; dy--)
	// 	{
	// 		brickData.x = (float)(x + OffsetBegin);
	// 		brickData.y = (float)(y + i);
	// 		i++;

	// 		offset = OffsetBegin;
	// 		pixLeft = NbPix;

	// 		pSrc = pSrcLine;
	// 		pDest = pDestLine;

	// 		NbBlock = *pMask++;

	// 		while ((NbBlock > 1) && (pixLeft > 0))
	// 		{
	// 			n = *pMask++;
	// 			NbBlock--;

	// 			if (offset)
	// 			{
	// 				offset -= n;

	// 				if (offset < 0)
	// 				{
	// 					brickData.x -= offset;

	// 					if (brickData.x < 0)
	// 					{
	// 						brickData.y -= 1;
	// 						brickData.x += Screen_X;

	// 						assert(brickData.y >= 0);
	// 					}

	// 					pSrc -= offset;
	// 					pDest -= offset;
	// 					pixLeft += offset;
	// 					offset = 0;
	// 				}
	// 			}
	// 			else if (n)
	// 			{
	// 				brickData.x += n;

	// 				if (brickData.x > Screen_X)
	// 				{
	// 					brickData.y += 1;
	// 					brickData.x -= Screen_X;

	// 					assert(brickData.y <= Screen_Y);
	// 				}

	// 				pDest += n;
	// 				pSrc += n;
	// 				pixLeft -= n;
	// 			}

	// 			if (pixLeft > 0)
	// 			{
	// 				n = *pMask++; // Nb pixels to Write
	// 				NbBlock--;

	// 				if (offset)
	// 				{
	// 					offset -= n;
	// 					if (offset <= 0)
	// 					{
	// 						n = -offset;
	// 						offset = 0;
	// 					}
	// 				}

	// 				if (!offset && n > 0)
	// 				{
	// 					if (n > pixLeft)
	// 					{
	// 						n = pixLeft;
	// 					}
	// 					pixLeft -= n;

	// 					brickData.w = (float)n;
	// 					brickData.h = 1;

	// 					brickData.y = (float)floor((double)(((double)(pDest - Log)) / (double)Screen_X));
	// 					brickData.x = pDest - Log - brickData.y*Screen_X;

	// 					memcpy(pDest, pSrc, n);
	// 					pDest += n;
	// 					pSrc += n;
	// 				}
	// 			}
	// 		}

	// 		pMask += NbBlock;
	// 		pDestLine += Screen_X;
	// 		pSrcLine += Screen_X;
	// 	}
	// }
	// else
	// {
	// 	// not clipped
	// 	n = GetTabOffLine(y) + x;
	// 	pDest = Log + n;
	// 	pSrc += n;
	// 	dx = Screen_X - dx; // Next Line

	// 	for (; dy; dy--)
	// 	{
	// 		NbBlock = *pMask++;
	// 		while (NbBlock)
	// 		{
	// 			n = *pMask++; // Nb pixels to Jump
	// 			pDest += n;
	// 			pSrc += n;
	// 			NbBlock--;

	// 			if (NbBlock)
	// 			{
	// 				n = *pMask++; // Nb pixels to Write

	// 				memcpy(pDest, pSrc, n);
	// 				pDest += n;
	// 				pSrc += n;
	// 				NbBlock--;
	// 			}
	// 		}

	// 		pDest += dx;
	// 		pSrc += dx;
	// 	}
	// }
}
