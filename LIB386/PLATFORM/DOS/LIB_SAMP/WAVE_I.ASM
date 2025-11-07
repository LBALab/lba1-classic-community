;*──────────────────────────────────────────────────────────────────────────*
;                               SAMP_A.ASM 386
;                             (c) Adeline 1993
;*──────────────────────────────────────────────────────────────────────────*

;*--------------------------------------------------------------------------*

		.386

		.model  SMALL, SYSCALL

		.DATA

		PUBLIC Wave_listfcts
		PUBLIC Wave_Driver_Enable


Wave_listfcts		dd	0
Wave_Driver_Enable	dd	0

		.CODE

		PUBLIC _InitCard
		PUBLIC _ClearCard
		PUBLIC WaveAskVars
		PUBLIC WavePlay
		PUBLIC WaveGiveInfo0
		PUBLIC WaveStop
		PUBLIC WaveStopOne
		PUBLIC WaveInList
		PUBLIC WaveGetSnap
		PUBLIC WavePause
		PUBLIC WaveContinue
		PUBLIC WaveSaveState
		PUBLIC WaveRestoreState
		PUBLIC WaveChangeVolume
		PUBLIC WaveGetAddr
		PUBLIC _GetBufferSize
		PUBLIC WaveChangeVolume
		PUBLIC WaveMove
		PUBLIC WaveStopOneLong

;----------------------------------------------------------------------------
;----------------------------------------------------------------------------

WaveCallFuncs	PROC

		cmp	byte ptr[Wave_Driver_Enable], 0
		je	return

		shl	eax, 2
		add	eax, dword ptr[Wave_listfcts]
		jmp	dword ptr[eax]
return:
		xor	eax, eax			; FALSE, NULL or 0
		ret

WaveCallFuncs	ENDP

;----------------------------------------------------------------------------
;----------------------------------------------------------------------------


_InitCard	PROC

		mov	eax, 0
		jmp	WaveCallFuncs

_InitCard	ENDP

;----------------------------------------------------------------------------

_ClearCard	PROC

		mov	eax, 1
		jmp	WaveCallFuncs

_ClearCard	ENDP

;----------------------------------------------------------------------------

WaveAskVars	PROC

		mov	eax, 2
		jmp	WaveCallFuncs

WaveAskVars	ENDP

;*--------------------------------------------------------------------------*

WavePlay	PROC

		mov	eax, 3
		jmp	WaveCallFuncs

WavePlay	ENDP

;----------------------------------------------------------------------------

WaveGiveInfo0	PROC

		mov	eax, 4
		jmp	WaveCallFuncs

WaveGiveInfo0	ENDP

;*--------------------------------------------------------------------------*

WaveStop	PROC

		mov	eax, 5
		jmp	WaveCallFuncs

WaveStop	ENDP

;*--------------------------------------------------------------------------*

WaveStopOne	PROC

		mov	eax, 6
		jmp	WaveCallFuncs

WaveStopOne	ENDP

;*--------------------------------------------------------------------------*

WaveInList	PROC

		mov	eax, 7
		jmp	WaveCallFuncs

WaveInList	ENDP

;----------------------------------------------------------------------------

WaveGetSnap	PROC

		mov	eax, 8
		jmp	WaveCallFuncs

WaveGetSnap	ENDP

;*--------------------------------------------------------------------------*

WavePause	PROC

		mov	eax, 9
		jmp	WaveCallFuncs

WavePause	ENDP

;*--------------------------------------------------------------------------*

WaveContinue	PROC

		mov	eax, 10
		jmp	WaveCallFuncs

WaveContinue	ENDP

;----------------------------------------------------------------------------

WaveSaveState	PROC

		mov	eax, 11
		jmp	WaveCallFuncs

WaveSaveState	ENDP

;----------------------------------------------------------------------------

WaveRestoreState PROC

		mov	eax, 12
		jmp	WaveCallFuncs

WaveRestoreState ENDP

;----------------------------------------------------------------------------

WaveGetAddr	PROC

		mov	eax, 13
		jmp	WaveCallFuncs

WaveGetAddr	ENDP

;----------------------------------------------------------------------------

_GetBufferSize	PROC

		mov	eax, 14
		jmp	WaveCallFuncs

_GetBufferSize	ENDP

;----------------------------------------------------------------------------

WaveChangeVolume PROC

		mov	eax, 15
		jmp	WaveCallFuncs

WaveChangeVolume ENDP

;----------------------------------------------------------------------------

WaveMove	PROC

		mov	eax, 16
		jmp	WaveCallFuncs

WaveMove	ENDP

;----------------------------------------------------------------------------

WaveStopOneLong	PROC

		mov	eax, 17
		jmp	WaveCallFuncs

WaveStopOneLong	ENDP

;----------------------------------------------------------------------------

		END
