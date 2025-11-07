#include "DEFINES.H"
#include "BALANCE.H"

void Balance(ULONG balance, ULONG volume, ULONG *vol_left, ULONG *vol_right)
{
	UINT computed_volume_right = 0;
	UINT computed_volume_left = 0;

	computed_volume_right = (P_SinTab[balance] * volume) >> 14;
	computed_volume_left = (P_SinTab[balance + 256] * volume) >> 14;

	*(vol_right) = computed_volume_right;
	*(vol_left) = computed_volume_left;
}

void BalanceWord(ULONG balance, ULONG volume, UWORD *vol_left, UWORD *vol_right)
{
	UINT computed_volume_right = 0;
	UINT computed_volume_left = 0;

	computed_volume_right = (P_SinTab[balance] * volume) >> 14;
	computed_volume_left = (P_SinTab[balance + 256] * volume) >> 14;

	*(vol_right) = computed_volume_right;
	*(vol_left) = computed_volume_left;
}
