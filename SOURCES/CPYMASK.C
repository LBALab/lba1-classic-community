#include "C_EXTERN.H"


void CopyMask(LONG nummask, LONG x, LONG y, void *bankmask, void *screen)
{
	UBYTE *pMask;
	UBYTE *pSrc;
	UBYTE *pDest;
	LONG dx, dy;
	LONG x1, y1;
	LONG n;
	UBYTE NbBlock;
	ULONG *pTabOffLine = (ULONG *)&TabOffLine;
	ULONG *pBank = (ULONG *)bankmask;

	pMask = (UBYTE *)bankmask + pBank[nummask];
	pSrc = (UBYTE *)screen;
	
	dx = pMask[0];
	dy = pMask[1];
	x += pMask[2];
	y += pMask[3];
	
	pMask += 4;
	
	x1 = dx + x - 1;
	y1 = dy + y - 1;
	
	if ((x < ClipXmin) || (y < ClipYmin) || (x1 > ClipXmax) || (y1 > ClipYmax))
	{
		UBYTE *pSrcLine;
		UBYTE *pDestLine;
		LONG OffsetBegin = 0;
		LONG NbPix, pixLeft, offset;
		
		if ((x > ClipXmax) || (y > ClipYmax) || (x1 < ClipXmin) || (y1 < ClipYmin))
		{
			return;
		}
		
		if (y < ClipYmin)
		{
			for (n = ClipYmin - y; n > 0; n--)
			{
				NbBlock = *pMask;
				pMask += NbBlock + 1;
			}
			y = ClipYmin;
		}
		
		if (y1 > ClipYmax)
		{
			y1 = ClipYmax;
		}
		
		if (x < ClipXmin)
		{
			OffsetBegin = ClipXmin - x;
		}
		
		NbPix = x1 - x - OffsetBegin + 1;
		if (x1 > ClipXmax)
		{
			NbPix -= x1 - ClipXmax;
		}
		
		n = pTabOffLine[y] + x + OffsetBegin;
		pSrcLine = pSrc + n;
		pDestLine = Log + n;
		
		for (dy = y1 - y + 1; dy; dy--)
		{
			offset = OffsetBegin;
			pixLeft = NbPix;
			
			pSrc = pSrcLine;
			pDest = pDestLine;
			
			NbBlock = *pMask++;
			
			while ((NbBlock > 1) && (pixLeft > 0))
			{
				n = *pMask++;
				NbBlock--;
				
				if (offset)
				{
					offset -= n;
					
					if (offset < 0)
					{
						pSrc -= offset;
						pDest -= offset;
						pixLeft += offset;
						offset = 0;
					}
				}
				else if (n)
				{
					pDest += n;
					pSrc += n;
					pixLeft -= n;
				}
				
				if (pixLeft > 0)
				{
					n = *pMask++;
					NbBlock--;
					
					if (offset)
					{
						offset -= n;
						if (offset <= 0)
						{
							n = -offset;
							offset = 0;
						}
					}
					
					if (!offset && n > 0)
					{
						if (n > pixLeft)
						{
							n = pixLeft;
						}
						pixLeft -= n;
						
						memcpy(pDest, pSrc, n);
						pDest += n;
						pSrc += n;
					}
				}
			}
			
			pMask += NbBlock;
			pDestLine += Screen_X;
			pSrcLine += Screen_X;
		}
	}
	else
	{
		n = pTabOffLine[y] + x;
		pDest = Log + n;
		pSrc += n;
		dx = Screen_X - dx;
		
		for (; dy; dy--)
		{
			NbBlock = *pMask++;
			while (NbBlock)
			{
				n = *pMask++;
				pDest += n;
				pSrc += n;
				NbBlock--;
				
				if (NbBlock)
				{
					n = *pMask++;
					
					memcpy(pDest, pSrc, n);
					pDest += n;
					pSrc += n;
					NbBlock--;
				}
			}
			
			pDest += dx;
			pSrc += dx;
		}
	}
}
