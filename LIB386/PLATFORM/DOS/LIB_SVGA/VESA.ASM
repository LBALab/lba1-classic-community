; Simplified VESA initialization for DOSBox compatibility
; This version removes complex DPMI calls and uses direct VESA BIOS

;*--------------------------------------------------------------------------*
;                                VESA.ASM 386
;                             (c) Adeline 1993
;                    Modified for DOSBox compatibility
;*--------------------------------------------------------------------------*

		.386
		.model  FLAT, SYSCALL
		.DATA

		PUBLIC VESA_Error

		extrn ScanLine:	DWORD
		extrn NonStdVESA:	BYTE
		extrn BankShift:	BYTE

;----------------------------------------------------------------------------

Vesa_Error	db	0

; VESA info buffer (allocated in data segment)
VesaInfoBuffer	db	256 dup(0)

;----------------------------------------------------------------------------

		.CODE

		PUBLIC NewBankVesa
		PUBLIC InitModeVesa

;----------------------------------------------------------------------------
; Bank switching routine (unchanged)
;----------------------------------------------------------------------------
NewBankVesa		proc

			push	ebx

Shift			equ	$+3
			shl	ax, 0

			mov	dx, ax
			mov	eax, 4F05h		; change bank VESA
			xor	ebx, ebx		; bh = 0, set bank
			int	10h

			pop	ebx
			ret

NewBankVesa		endp

;----------------------------------------------------------------------------
; Simplified VESA initialization for DOSBox
; Uses direct INT 10h calls instead of DPMI
;----------------------------------------------------------------------------
InitModeVesa		proc

			mov	byte ptr[VESA_Error], 0

			; For DOSBox compatibility, we'll use a simplified approach
			; DOSBox VESA mode 101h always uses 640 bytes per scanline
			; and standard granularity, so we can hardcode these values
			
			; Try to set VESA mode 101h (640x480x256)
			mov	ax, 4F02h		; VESA Set Mode
			mov	bx, 0101h		; Mode 101h
			int	10h
			
			; Check if successful
			cmp	ax, 004Fh
			jne	error
			
			; Mode set successful - use standard VESA values
			mov	dword ptr[ScanLine], 640	; Standard scanline for 640x480
			mov	byte ptr[NonStdVESA], 0		; Standard VESA
			mov	byte ptr[Shift], 0		; 64KB granularity = shift 0
			
			; Success
			mov	byte ptr[VESA_Error], 0
			ret

error:
			; VESA mode failed, try standard VGA mode 13h as fallback
			mov	ax, 0013h		; VGA 320x200x256
			int	10h
			
			; Set VGA mode parameters
			mov	dword ptr[ScanLine], 320
			mov	byte ptr[NonStdVESA], 1
			mov	byte ptr[Shift], 6		; VGA bank shift
			
			; Report error but continue (using VGA fallback)
			mov	byte ptr[VESA_Error], 1
			ret

InitModeVesa		endp

;----------------------------------------------------------------------------

			END
