#include "ADELINE.H"
#include "LIB_SYS/LIB_SYS.H"
#include "LIB_3D/LIB_3D.H"

#define ANGLE_MASK 1023
#define QUARTER_TURN 256

LONG Distance = 0;
LONG EX0 = 0;
LONG EY0 = 0;

static void init_real_value_from_time(WORD start,
				      WORD end,
				      WORD duration,
				      T_REAL_VALUE *ptrstruct,
				      ULONG current_time)
{
	ptrstruct->StartValue = start;
	ptrstruct->EndValue = end;
	ptrstruct->TimeValue = duration;
	ptrstruct->MemoTicks = current_time;
}

static WORD get_real_value_from_time(T_REAL_VALUE *ptrstruct, ULONG current_time)
{
	if (ptrstruct->TimeValue)
	{
		LONG delta = (LONG)current_time - (LONG)ptrstruct->MemoTicks;
		if (delta < (LONG)ptrstruct->TimeValue)
		{
			LONG value = ((ptrstruct->EndValue - ptrstruct->StartValue) * delta) / ptrstruct->TimeValue;
			value += ptrstruct->StartValue;
			return (WORD)value;
		}

		ptrstruct->TimeValue = 0;
	}

	return ptrstruct->EndValue;
}

void InitRealValue(WORD start, WORD end, WORD duration, T_REAL_VALUE *ptrstruct)
{
    init_real_value_from_time(start, end, duration, ptrstruct, TimerRef);
}

void InitRealAngle(WORD start, WORD end, WORD duration, T_REAL_VALUE *ptrstruct)
{
    init_real_value_from_time(start & ANGLE_MASK,
				       end & ANGLE_MASK,
				       duration,
				       ptrstruct,
				       TimerRef);
}

void InitRealAngleConst(WORD start, WORD end, LONG duration, T_REAL_VALUE *ptrstruct)
{
    WORD t;

    ptrstruct->StartValue = start & ANGLE_MASK;
    ptrstruct->EndValue = end & ANGLE_MASK;

    t = (ptrstruct->StartValue - ptrstruct->EndValue) << 6;
    if (t < 0)
    {
        t = -t;
    }
    t = ((UWORD)t >> 6);
    ptrstruct->TimeValue = (WORD)((t * duration) >> 8);
    ptrstruct->MemoTicks = TimerRef;
}

WORD GetRealValue(T_REAL_VALUE *ptrstruct)
{
	return get_real_value_from_time(ptrstruct, TimerRef);
}

WORD GetRealAngle(T_REAL_VALUE *ptrstruct)
{
	if (ptrstruct->TimeValue)
	{
		LONG delta = (LONG)TimerRef - (LONG)ptrstruct->MemoTicks;
		if (delta < (LONG)ptrstruct->TimeValue)
		{
			LONG value = ptrstruct->EndValue - ptrstruct->StartValue;
			if (value < -512)
			{
				value += 1024;
			}
			else if (value > 512)
			{
				value -= 1024;
			}

			value = (value * delta) / ptrstruct->TimeValue;
			value += ptrstruct->StartValue;
			return (WORD)value;
		}

		ptrstruct->TimeValue = 0;
	}

	return ptrstruct->EndValue;
}

LONG BoundRegleTrois(LONG val1, LONG val2, LONG nbstep, LONG step)
{
	if (step <= 0)
	{
		return val1;
	}

	if (step >= nbstep)
	{
		return val2;
	}

	return val1 + ((val2 - val1) * step) / nbstep;
}

LONG RegleTrois32(LONG val1, LONG val2, LONG nbstep, LONG step)
{
	if (nbstep <= 0)
	{
		return val2;
	}

	return (((val2 - val1) * step) / nbstep) + val1;
}

static ULONG bsr32(ULONG value)
{
	ULONG bit = 31;

	if ((value & 0xFFFF0000UL) == 0)
	{
		value <<= 16;
		bit = 15;
	}

	if ((value & 0xFF000000UL) == 0)
	{
		value <<= 8;
		bit -= 8;
	}

	if ((value & 0xF0000000UL) == 0)
	{
		value <<= 4;
		bit -= 4;
	}

	if ((value & 0xC0000000UL) == 0)
	{
		value <<= 2;
		bit -= 2;
	}

	if ((value & 0x80000000UL) == 0)
	{
		--bit;
	}

	return bit;
}

static ULONG shld32(ULONG dest, ULONG src, ULONG count)
{
	dest <<= count;
	count = 32 - count;
	dest |= (src >> count);
	return dest;
}

ULONG Sqr(ULONG value)
{
    ULONG src;
    ULONG bit;
    ULONG acc;
    ULONG res = 1;

	if (value <= 3)
	{
		return (value == 0) ? 0 : 1;
	}

    src = value;
    bit = bsr32(src);
    bit = (33 - bit) & 0xFFFFFFFEUL;
    acc = shld32(0, src, bit) - 1;
    src <<= bit;

    for (bit >>= 1; bit > 0; --bit)
    {
        acc = shld32(acc, src, 2);
        src <<= 2;
        res <<= 1;

        if (acc >= res)
        {
            ++res;
            if (acc >= res)
            {
                acc -= res;
                ++res;
            }
        }

        res >>= 1;
    }

    return res;
}

LONG Distance2D(LONG x0, LONG z0, LONG x1, LONG z1)
{
	x1 -= x0;
	z1 -= z0;

	return (LONG)Sqr((ULONG)(x1 * x1) + (ULONG)(z1 * z1));
}

LONG Distance3D(LONG x0, LONG y0, LONG z0, LONG x1, LONG y1, LONG z1)
{
	x1 -= x0;
	y1 -= y0;
	z1 -= z0;

	return (LONG)Sqr((ULONG)(x1 * x1) + (ULONG)(y1 * y1) + (ULONG)(z1 * z1));
}

LONG GetAngle(LONG x0, LONG z0, LONG x1, LONG z1)
{
	LONG angle = 0;
	LONG x2, z2;
	LONG start;
	LONG end;
	LONG diff = 0;
	LONG tmp;

	x1 -= x0;
	z1 -= z0;

	x2 = x1 * x1;
	z2 = z1 * z1;

	Distance = Sqr((ULONG)(x2 + z2));
	if (Distance)
	{
		if (z2 > x2)
		{
			LONG swap = x1;
			x1 = z1 | 1;
			z1 = swap;
		}
		else
		{
			x1 &= -2;
		}

		tmp = (z1 << 14) / Distance;
		start = 384;
		end = 384 + 256;

		while (start < (end - 1))
		{
			angle = (start + end) >> 1;
			diff = tmp - P_SinTab[angle];
			if (diff > 0)
			{
				end = angle;
			}
			else
			{
				start = angle;
				if (diff == 0)
				{
					break;
				}
			}
		}

		if (diff)
		{
			LONG avg = (P_SinTab[start] + P_SinTab[end]) >> 1;
			if (tmp <= avg)
			{
				start = end;
			}
		}

		angle = start - 256;
		if (x1 < 0)
		{
			angle = -angle;
		}

		if (x1 & 1)
		{
			angle = 256 - angle;
		}

		angle &= ANGLE_MASK;
	}

	return angle;
}

void Rot2D(LONG x, LONG y, LONG angle)
{
	if (angle)
	{
		LONG sine;
		LONG cosine;

		angle &= ANGLE_MASK;
		sine = P_SinTab[angle];
		cosine = P_SinTab[(angle + QUARTER_TURN) & ANGLE_MASK];

		EX0 = ((x * cosine) - (y * sine)) >> 15;
		EY0 = ((x * sine) + (y * cosine)) >> 15;
	}
	else
	{
		EX0 = x;
		EY0 = y;
	}
}
