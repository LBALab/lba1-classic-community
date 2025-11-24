;ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
;лл                                                                         лл
;лл  SBMIDI32.INC                                                           лл
;лл                                                                         лл
;лл  IBM Audio Interface Library -- Sound Blaster MIDI I/O routines         лл
;лл                                                                         лл
;лл  Version 1.00 of  4-Apr-93: 32-bit conversion (Rational Systems DOS/4G) лл
;лл          1.01 of  1-May-93: Flashtek X32 compatibility added            лл
;лл                                                                         лл
;лл  80386 ASM source compatible with Microsoft Assembler v6.0 or later     лл
;лл  Author: John Miles                                                     лл 
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

DSP_RESET       dd ?                    ;IO_Addr+06h
ADDR_STAT       dd ?                    ;+08h
DATA_OUT        dd ?                    ;+09h
DSP_READ        dd ?                    ;+0Ah
DSP_WRITE_STAT  dd ?                    ;+0Ch
DSP_DATA_RDY    dd ?                    ;+0Eh

;****************************************************************************
set_IO_parms    PROC\
                USES ebx esi edi\
                ,IO_ADDR,IRQ,DMA,DRQ

                mov eax,[IO_ADDR]
                add eax,6
                mov DSP_RESET,eax
                add eax,2
                mov ADDR_STAT,eax
                add eax,1
                mov DATA_OUT,eax
                add eax,1
                mov DSP_READ,eax
                add eax,2
                mov DSP_WRITE_STAT,eax
                add eax,2
                mov DSP_DATA_RDY,eax

                ret
set_IO_parms    ENDP

;****************************************************************************
detect_device   PROC\
                USES ebx esi edi\
                ,H,IO_ADDR,IRQ,DMA,DRQ    ;Attempt to detect Ad Lib chip on
                pushfd                    ;Sound Blaster card
                cli

                push DSP_RESET            ;preserve current I/O addresses
                push ADDR_STAT      
                push DATA_OUT       
                push DSP_READ       
                push DSP_WRITE_STAT 
                push DSP_DATA_RDY   

                invoke set_IO_parms,[IO_ADDR],[IRQ],[DMA],[DRQ]
                call detect_Adlib

                pop DSP_DATA_RDY   
                pop DSP_WRITE_STAT 
                pop DSP_READ       
                pop DATA_OUT       
                pop ADDR_STAT      
                pop DSP_RESET

                POP_F                     ;return AX=0 if not found
                ret
detect_device   ENDP

read_status     PROC                      ;Read YM3812 status register
                mov edx,ADDR_STAT        
                mov eax,0
                in al,dx
                ret
read_status     ENDP

detect_send     PROC Address,Data         ;Write data byte to specified AL reg
                mov edx,ADDR_STAT        
                mov al,BYTE PTR [Address]
                out dx,al                 ;select register address
                mov ecx,6
__3_3_us:       in al,dx                  ;delay 3.3 uS
                loop __3_3_us
                mov edx,DATA_OUT
                mov al,BYTE PTR [Data]
                out dx,al
                mov edx,ADDR_STAT         
                mov ecx,42
__23_us:        in al,dx                  ;delay 23 uS
                loop __23_us
                ret
detect_send     ENDP

detect_Adlib    PROC\                     ;Detect standard YM3812 timer regs
                USES ebx esi edi
                invoke detect_send,4,60h  ;reset T1 and T2
                invoke detect_send,4,80h  ;reset IRQ
                call read_status
                mov edi,eax               ;save timer status
                invoke detect_send,2,0ffh ;set T1 to 0FFh
                invoke detect_send,4,21h  ;unmask and start T1
                mov esi,200               ;wait 100 uS for timer to count down
__wait_100_uS:  call read_status
                dec esi
                jnz __wait_100_uS
                mov esi,eax               ;save timer status
                invoke detect_send,4,60h  ;reset T1 and T2
                invoke detect_send,4,80h  ;reset IRQ
                and esi,0e0h              ;mask off undefined bits
                and edi,0e0h
                mov eax,0                 ;assume board not detected
                cmp edi,0
                jne __return              ;initial timer value not 0, exit
                cmp esi,0c0h
                jne __return              ;timer didn't overflow, exit
                mov eax,1                 ;else Ad Lib-compatible board exists
__return:       ret
detect_Adlib    ENDP

;****************************************************************************
DSP_send        PROC\
                USES ebx esi edi\
                ,DataByte

                mov edx,DSP_WRITE_STAT   
                mov ecx,0
                mov esi,64
__wait_cts:     in al,dx
                test al,80h
                loopnz __wait_cts
                dec esi
                jnz __wait_cts
                or ecx,esi
                jz __timeout

                mov al,BYTE PTR [DataByte]
                out dx,al

                mov eax,1
                ret

__timeout:      mov eax,0
                ret

DSP_send        ENDP

;****************************************************************************
send_byte       PROC\
                USES ebx esi edi\
                ,DataByte

                invoke DSP_send,38h     ;MIDI WRITE command
                invoke DSP_send,[DataByte]

                ret
send_byte       ENDP

;****************************************************************************
reset_interface PROC                    ;Returns 1 if SB interface available

                pushfd         
                cli

                mov eax,1

                POP_F
                ret

reset_interface ENDP

;****************************************************************************
init_interface  PROC                    ;Init MIDI interface

                ret
init_interface  ENDP
