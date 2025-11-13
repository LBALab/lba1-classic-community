;-----------------------------------------------------------------------------
;			Extern lib_svga 386
;-----------------------------------------------------------------------------
				.data

		extrn ClipXmin	:DWORD
		extrn ClipYmin	:DWORD
		extrn ClipXmax	:DWORD
		extrn ClipYmax	:DWORD
		extrn Phys		:DWORD
		extrn Log		:DWORD
		extrn MemoLog		:DWORD
		extrn BankCurrent	:DWORD

		extrn Screen_X	:DWORD
		extrn Screen_Y	:DWORD
		extrn TabOffLine	:DWORD

		extrn SizeCar		:WORD

		extrn Text_Ink	:BYTE
		extrn Text_Paper	:BYTE
		extrn OldVideo	:BYTE

POLY_TRISTE	equ	0
POLY_COPPER	equ	1
POLY_BOPPER	equ	2
POLY_MARBRE	equ	3
POLY_TELE	equ	4
POLY_TRANS	equ	5
POLY_TRAME	equ	6
POLY_GOURAUD	equ	7
POLY_DITHER	equ	8



