;ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
;лл                                                                         лл
;лл  MPU40132.INC                                                           лл
;лл                                                                         лл
;лл  IBM Audio Interface Library -- Roland MPU-401 I/O routines             лл
;лл                                                                         лл
;лл  Version 1.00 of 29-Jul-92: 32-bit conversion (Rational Systems DOS/4G) лл
;лл          1.01 of  1-May-93: Flashtek X32 compatibility added            лл
;лл                                                                         лл
;лл  80386 ASM source compatible with Microsoft Assembler v6.0 or later     лл
;лл  Author: John Miles (32-bit flat model conversion by John Lemberger)    лл                            лл
;лл                                                                         лл
;ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
;лл                                                                         лл
;лл  Copyright (C) 1991-1993 Miles Design, Inc.                             лл
;лл                                                                         лл
;лл  Miles Design, Inc.                                                     лл
;лл  6702 Cat Creek Trail                                                   лл
;лл  Austin, TX 78731                                                       лл
;лл  (512) 345-2642 / FAX (512) 338-9630 / BBS (512) 454-9990               лл
;лл                                                                         лл
;ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

DATA_IO         dw ?                    ;IO_addr
STAT_COM        dw ?                    ;IO_addr+1

;****************************************************************************
set_IO_parms    PROC\
                USES ebx esi edi\
                ,IO_ADDR,IRQ,DMA,DRQ

                mov eax,[IO_ADDR]
                mov DATA_IO,ax
                inc eax
                mov STAT_COM,ax

                ret
set_IO_parms    ENDP

;****************************************************************************
detect_device   PROC\
                USES ebx esi edi\
                ,H,IO_ADDR,IRQ,DMA,DRQ    ;Attempt to detect MPU-401 card
                pushfd
                cli

                push DATA_IO              ;preserve current I/O address
                push STAT_COM

                invoke set_IO_parms,[IO_ADDR],[IRQ],[DMA],[DRQ]

                call reset_interface      ;attempt to reset card

                pop STAT_COM
                pop DATA_IO

                POP_F                     ;return AX=0 if not found
                ret
detect_device   ENDP

;****************************************************************************
send_byte       PROC DataByte

                mov dx,STAT_COM
__wait_cts:     in al,dx
                test al,40h
                je __write_it
                test al,80h
                jne __wait_cts
                pushfd
                sti                     ;if MIDI byte incoming, give receiving
                jmp $+2                 ;process a chance to catch it ...
                jmp $+2                 ;prevents lost data and/or hangups
                jmp $+2                 ;during Cakewalk and MIDIECHO
                POP_F
                mov dx,DATA_IO
                in al,dx
                mov dx,STAT_COM
                jmp __wait_cts

__write_it:     mov dx,DATA_IO
                mov al,BYTE PTR [DataByte]
                out dx,al

                ret
send_byte       ENDP

;****************************************************************************
send_cmd        PROC CmdByte

                pushfd
                cli
                mov dx,STAT_COM
                mov ecx,0ffffh
__wait_cts_1:   in al,dx
                test al,40h
                jz __cts_1
                loop __wait_cts_1
                jmp __exit_bad

__cts_1:        mov al,BYTE PTR [CmdByte]
                out dx,al

                mov ecx,0ffffh
__wait_ack:     mov dx,STAT_COM
                in al,dx
                test al,80h
                jnz __next_loop
                mov dx,DATA_IO
                in al,dx
                cmp al,0feh
                je __exit_OK
__next_loop:    loop __wait_ack

__exit_bad:     mov eax,0
                jmp __exit
__exit_OK:      mov eax,1
__exit:         POP_F
                ret
send_cmd        ENDP

;****************************************************************************
reset_interface PROC\                   ;Returns 1 if MPU interface available
                USES ebx esi edi
                pushfd                  ;don't allow interruption
                cli

                invoke send_cmd,0ffh    ;reset MPU-401 interface
                or eax,eax
                jne __exit_reset        ;(worked the first time)

                invoke send_cmd,0ffh    ;try again in case UART mode active

__exit_reset:   POP_F
                ret
reset_interface ENDP

;****************************************************************************
init_interface  PROC\                   ;Init MPU-401-compatible interface
                USES ebx esi edi
                pushfd                  ;don't allow interruption
                cli

                invoke send_cmd,03fh    ;assert UART mode

                POP_F
                ret

init_interface  ENDP
;****************************************************************************
