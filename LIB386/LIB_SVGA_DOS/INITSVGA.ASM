;----------------------------------------------------------------------------
;				INITSVGA.ASM 386
;			      (c) Adeline 1993
;----------------------------------------------------------------------------
				.386p
				.model FLAT, SYSCALL

;----------------------------------------------------------------------------
				.data

			include	vesa.ash

			public Log
			public Phys
			public Text_Ink
			public Text_Paper
			public Screen_X
			public Screen_Y
			public MemoLog
			public ClipXmin
			public ClipYmin
			public ClipXmax
			public ClipYmax
			public TabOffLine
			public OldVideo
			public SizeCar
			public BankChange
			public BankCurrent

			public InitSvgaMode
			public NewBank
			public Enable


ClipXmin	dd	0
ClipYmin	dd	0
ClipXmax	dd	639
ClipYmax	dd	479

MemoClipXmin	dd	0
MemoClipYmin	dd	0
MemoClipXmax	dd	639
MemoClipYmax	dd	479

TabOffLine	dd	480 dup(?)	; LONG et non pas WORD comme MCGA

TabOffPhysLine	dd	480 dup(?)	; LONG et non pas WORD comme MCGA

OldVideo 	db	-1

Text_Ink	db	15
Text_Paper	db	-1
		db	0

Screen_X	dd	640
Screen_Y	dd	480
SizeCar		dw	8			; 6 old affstring

		ALIGN 2

Save_1		dd	?

Log		dd	0
Phys		dd	0A0000h
MemoLog		dd	0
BankChange	dd	?
BankCurrent	dd	?

InitSvgaMode	dd	offset	InitModeVesa
NewBank		dd	offset	NewBankVesa
Enable		dd	0

;----------------------------------------------------------------------------
				.code

			InitModeVesa		PROTO
			NewBankVesa             PROTO

			extrn ScanLine:DWORD

			public InitSvga
			public InitMcga
			public InitMcgaMode
			public SimpleInitSvga
			public ClearVideo

			public SetClip
			public UnSetClip
			public MemoClip
			public RestoreClip


;*══════════════════════════════════════════════════════════════════════════*
;      ██▀▀▀ █▀▀▀▀ █▀▀▀█ █▀▀▀▀ █▀▀▀▀ ██▄ █       █▄ ▄█ █▀▀▀█ █▀▀▀▄ █▀▀▀▀
;      ▀▀▀▀█ ██    ██▀█▀ ██▀▀  ██▀▀  ██▀██       ██▀ █ ██  █ ██  █ ██▀▀
;      ▀▀▀▀▀ ▀▀▀▀▀ ▀▀  ▀ ▀▀▀▀▀ ▀▀▀▀▀ ▀▀  ▀ ▀▀▀▀▀ ▀▀  ▀ ▀▀▀▀▀ ▀▀▀▀  ▀▀▀▀▀
;*══════════════════════════════════════════════════════════════════════════*
;*──────────────────────────────────────────────────────────────────────────*

;----------------------------------------------------------------------------
InitSvga		proc

;---------------------- Save regs

			pushad

;----------------------	memo video mode

			mov	ah, 0Fh
			int	10h
			mov	[OldVideo], al

;---------------------- Video Mode

			mov	edx, [InitSvgaMode]
			or	edx, edx
			jnz	ok

			mov	edx, offset InitModeVESA
			mov	[InitSvgaMode], edx
ok:
			mov	eax, offset ScanLine
			call	edx

			mov	edx, [Enable]
			or	edx, edx
			jz	skip

			call	edx
skip:

;---------------------- Restore regs

			popad

;---------------------- Table Offset Line

			mov	ecx, Screen_Y
			xor	eax, eax
			mov	edx, eax
Again:
			mov	TabOffLine[edx*4], eax
			inc	edx
			add	eax, Screen_X
			dec	ecx
			jnz	Again

			mov	dword ptr[BankCurrent], -1

			ret

InitSvga		endp

;----------------------------------------------------------------------------

SimpleInitSvga		proc	uses ebx

			call	[InitSvgaMode]

			mov	eax, [Enable]
			or	eax, eax
			jz	skip

			call	eax
skip:
			mov	dword ptr[BankCurrent], -1

			ret

SimpleInitSvga		endp

;----------------------------------------------------------------------------
ClearSvga		proc

			xor	ax, ax
			mov	al, OldVideo
			int	10h
			ret

ClearSvga		endp


;----------------------------------------------------------------------------
InitMcgaMode  		proc	uses	esi edi

			mov	ax, 13h
			int	10h
			ret

InitMcgaMode		endp

;----------------------------------------------------------------------------
InitMcga		proc

;---------------------- Save regs

			pushad

;----------------------	memo video mode

			mov	ah, 0Fh
			int	10h
			mov	[OldVideo], al

;---------------------- Video Mode

			call	InitMcgaMode

;---------------------- Restore regs

			popad

			ret

InitMcga		endp

;----------------------------------------------------------------------------
ClearVideo		proc

			xor	ax, ax
			mov	al, OldVideo
			int	10h
			ret

ClearVideo		endp

;*══════════════════════════════════════════════════════════════════════════*
;               █▀▀▀▀ █      █    █▀▀▀█ █▀▀▀█  █    ██▄ █ █▀▀▀▀
;               ██    ██     ██   ██▀▀▀ ██▀▀▀  ██   ██▀██ ██ ▀█
;               ▀▀▀▀▀ ▀▀▀▀▀  ▀▀   ▀▀    ▀▀     ▀▀   ▀▀  ▀ ▀▀▀▀▀
;*══════════════════════════════════════════════════════════════════════════*
;*──────────────────────────────────────────────────────────────────────────*

SetClip			proc	,\
			x0:DWORD, y0:DWORD, x1:DWORD, y1:DWORD

			mov	eax, x0
			cmp	eax, 0
			jge	OkLeft
			xor	eax, eax
OkLeft:			mov	ClipXmin, eax

			mov	eax, y0
			cmp	eax, 0
			jge	OkUp
			xor	eax, eax
OkUp:			mov	ClipYmin, eax

			mov	eax, x1
			cmp	eax, Screen_X
			jl	OkRight
			mov	eax, Screen_X
			dec	eax
OkRight:		mov	ClipXmax, eax

			mov	eax, y1
			cmp	eax, Screen_Y
			jl	OkDown
			mov	eax, Screen_Y
			dec	eax
OkDown:			mov	ClipYmax, eax

			ret
SetClip			endp
;----------------------------------------------------------------------------
UnSetClip		proc

			xor	eax, eax
			mov	ClipXmin, eax
			mov	ClipYmin, eax

			mov     eax, Screen_X
			dec	eax
			mov	ClipXmax, eax

			mov	eax, Screen_Y
			dec	eax
			mov	ClipYmax, eax

			ret

UnSetClip		endp
;----------------------------------------------------------------------------
MemoClip		proc

			mov	eax,ClipXmin
			mov	MemoClipXmin, eax
			mov	eax,ClipYmin
			mov	MemoClipYmin, eax
			mov	eax,ClipXmax
			mov	MemoClipXmax, eax
			mov	eax,ClipYmax
			mov	MemoClipYmax, eax

			ret

MemoClip		endp
;----------------------------------------------------------------------------
RestoreClip		proc

			mov	eax,MemoClipXmin
			mov	ClipXmin, eax
			mov	eax,MemoClipYmin
			mov	ClipYmin, eax
			mov	eax,MemoClipXmax
			mov	ClipXmax, eax
			mov	eax,MemoClipYmax
			mov	ClipYmax, eax

			ret

RestoreClip		endp

;----------------------------------------------------------------------------
;				The
				End
