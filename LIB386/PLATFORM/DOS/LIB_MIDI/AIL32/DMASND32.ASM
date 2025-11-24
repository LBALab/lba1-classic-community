;ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
;лл                                                                         лл
;лл  DMASND32.ASM                                                           лл
;лл                                                                         лл
;лл  Digital sound driver/emulator for Sound Blaster-type audio devices     лл
;лл                                                                         лл
;лл  Version 1.00 of 29-Oct-92: 32-bit conversion (Rational Systems DOS/4G) лл
;лл          1.01 of 30-Dec-92: Do IRQ ack if SB port write times out       лл
;лл          1.02 of  1-May-93: Flashtek X32 compatibility added            лл
;лл          1.03 of 17-Jun-93: PAS detection restored                      лл
;лл          1.04 of 14-Sep-93: ALWAYS_ACK_IRQ option added                 лл
;лл          1.05 of 23-Sep-93: CHECK_ISR option added                      лл
;лл          1.06 of 25-Oct-93: Don't check ISR on IRQs other than 7        лл
;лл          1.07 of 18-Nov-93: Added DETECT_SBPRO option                   лл
;лл                                                                         лл
;лл  80386 ASM source compatible with Microsoft Assembler v6.0 or later     лл
;лл  Author: John Miles                                                     лл                            лл
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

                OPTION SCOPED           ;Enable local labels
                .386                    ;Enable 386 instruction set
 	.MODEL FLAT,C           ;Flat memory model, C calls

FALSE           equ 0
TRUE            equ -1

                ;
                ;External/configuration equates
                ;


ALWAYS_ACK_IRQ  equ TRUE                ;TRUE to acknowledge any spurious 
                                        ;Sound Blaster IRQs prior to all port
                                        ;writes -- may help avoid delays
                                        ;in starting sound effects

CHECK_END_DMA   equ TRUE                ;FALSE to inhibit checking for end-
                                        ;of-DMA conditions; disabling this 
                                        ;option is _not_ recommended!

CHECK_ISR       equ TRUE                ;FALSE to read the DMA controller's
                                        ;word count to guard against spurious
                                        ;IRQ 7 triggers ("old" method); TRUE 
                                        ;to verify IRQs by reading 8259's ISR
                                        ;status ("new" method)

PAS_FILTER      equ FALSE               ;TRUE to enable PAS PCM antialiasing
                                        ;(also degrades FM treble response)

DETECT_SBPRO    equ FALSE               ;TRUE to enable verification of SB Pro
                                        ;mixer chip during detection

DAC_STOPPED     equ 0
DAC_PAUSED      equ 1
DAC_PLAYING     equ 2
DAC_DONE        equ 3

                ;
                ;Macros, internal equates
                ;

                INCLUDE 386.mac         ;DOS extender macros
                INCLUDE ail32.inc

                ;
                ;Normalize far pointer
                ;(real-mode seg:off)
                ;

FAR_TO_HUGE     MACRO fp_seg,fp_off             
                push ax
                push bx                      
                mov ax,fp_seg
                mov bx,fp_off
                shr bx,1
                shr bx,1
                shr bx,1
                shr bx,1
                add ax,bx
                mov fp_seg,ax
                and fp_off,0fh
                pop bx 
                pop ax
                ENDM

                ;
                ;Add 32-bit dword to far ptr
                ;(real-mode seg:off)
                ;

ADD_PTR         MACRO add_l,add_h,pseg,poff     
                push bx                         
                push cx
                mov bx,pseg
                xor cx,cx
                REPT 4
                shl bx,1
                rcl cx,1
                ENDM
                add bx,poff
                adc cx,0
                add bx,add_l
                adc cx,add_h
                mov poff,bx
                and poff,1111b
                REPT 4
                shr cx,1
                rcr bx,1
                ENDM
                mov pseg,bx
                pop cx
                pop bx
                ENDM

                IFDEF PAS
STEREO          EQU 1
                ENDIF

                IFDEF SBSTD
SBLASTER        EQU 1
                ENDIF

                IFDEF SBPRO
SBLASTER        EQU 1
STEREO          EQU 1
                ENDIF

                IFDEF ADLIBG
STEREO          EQU 1
NEEDS_FORMAT    EQU 1
                ENDIF

VOC_MODE        equ 0                   ;Creative Voice File playback mode
BUF_MODE        equ 1                   ;Dual-buffer DMA playback mode

                IFDEF PAS
BI_OUTPUTMIXER  equ 00h                 ;PAS equates
BI_L_PCM        equ 06h
BI_R_PCM        equ 0dh
INTRCTLRST      equ 0b89h
AUDIOFILT       equ 0b8ah
INTRCTLR        equ 0b8bh
PCMDATA 	equ 0f88h	      
CROSSCHANNEL    equ 0f8ah
TMRCTLR 	equ 138bh
SAMPLERATE	equ 1388h
SAMPLECNT       equ 1389h
                ENDIF

sbuffer         STRUC
pack_type       dd ?
sample_rate     dd ?
ptr_data        dd ?                    ;ssss:oooooooo prot. mode far *
sel_data        dw ?                    
seg_data        dd ?                    ;ssss:oooo real-mode far *
len             dd ?
sbuffer         ENDS

xfer_chunk      PROTO C
next_block      PROTO C
next_buffer     PROTO C
process_block   PROTO C
process_buffer  PROTO C,Buf:DWORD
stop_d_pb       PROTO C
halt_DMA        PROTO C

                .CODE

                ;
                ;Vector table
                ;

                PUBLIC driver_start

driver_start    dd OFFSET driver_index
                db 'Copyright (C) 1991,1992 Miles Design, Inc.',01ah

driver_index    LABEL DWORD
                dd AIL_DESC_DRVR,OFFSET describe_driver 
                dd AIL_DET_DEV,OFFSET detect_device   
                dd AIL_INIT_DRVR,OFFSET init_driver     
                dd AIL_SHUTDOWN_DRVR,OFFSET shutdown_driver 
                dd AIL_P_VOC_FILE,OFFSET play_VOC_file       
                dd AIL_START_D_PB,OFFSET start_d_pb      
                dd AIL_STOP_D_PB,OFFSET stop_d_pb       
                dd AIL_PAUSE_D_PB,OFFSET pause_d_pb      
                dd AIL_RESUME_D_PB,OFFSET cont_d_pb       
                dd AIL_VOC_PB_STAT,OFFSET get_VOC_status
                dd AIL_SET_D_PB_VOL,OFFSET set_d_pb_vol    
                dd AIL_D_PB_VOL,OFFSET get_d_pb_vol
                dd AIL_SET_D_PB_PAN,OFFSET set_d_pb_pan
                dd AIL_D_PB_PAN,OFFSET get_d_pb_pan
                dd AIL_INDEX_VOC_BLK,OFFSET index_VOC_blk
                dd AIL_REG_SND_BUFF,OFFSET register_sb
                dd AIL_SND_BUFF_STAT,OFFSET get_sb_status
                dd AIL_F_VOC_FILE,OFFSET format_VOC_file       
                dd AIL_F_SND_BUFF,OFFSET format_sb
                dd -1

                ;
                ;Driver Description Table (DDT)
                ;Returned by describe_driver() proc
                ;

DDT             LABEL WORD
min_API_version dd 200                  ;Minimum API version required = 2.00
driver_type     dd 2                    ;Type 2: SBlaster DSP emulation
data_suffix     db 'VOC',0              ;Supports .VOC files directly
device_name_o   dd OFFSET devnames      ;Pointer to list of supported devices
default_IO      LABEL WORD              ;Factory default I/O parameters
                IFDEF PAS       
                dd -1                   ;(determined from MVSOUND.SYS)
                ELSEIFDEF ADLIBG
                dd 388h
                ELSE
                dd 220h                 
                ENDIF
default_IRQ     LABEL WORD
                IFDEF SBSTD
                dd 7
                ELSEIFDEF SBPRO
                dd 7                    ;(pre-prod. = 5; prod. = 7)
                ELSEIFDEF PAS
                dd -1                   ;(determined from MVSOUND.SYS)
                ELSEIFDEF ADLIBG
                dd -1                   ;(determined from control regs)
                ENDIF
default_DMA     LABEL WORD
                IFDEF PAS
                dd -1                   ;(determined from MVSOUND.SYS)
                ELSEIFDEF ADLIBG
                dd -1                   ;(determined from control regs)
                ELSE
                dd 1
                ENDIF
default_DRQ     dd -1
service_rate    dd -1                   ;No periodic service required
display_size    dd 0                    ;No display

devnames        LABEL BYTE
                IFDEF SBSTD
                db 'Creative Labs Sound Blaster(TM) Digital Sound',0
                db 'Media Vision Thunderboard(TM) Digital Sound',0
                ELSEIFDEF SBPRO
                db 'Creative Labs Sound Blaster Pro(TM) Digital Sound',0
                ELSEIFDEF PAS
                db 'Media Vision Pro Audio Spectrum(TM) Digital Sound',0
                ELSEIFDEF ADLIBG
                db 'Ad Lib(R) Gold Music Synthesizer Card',0
                ENDIF
                db 0                    ;0 to end list of device names

                ;
                ;Misc. data
                ;

                IFDEF ADLIBG
CTRL_ADDR       dd ?
CTRL_DATA       dd ?
DSP_ADDR        dd ?
DSP_DATA        dd ?

mask_save       dd ?
pack_mode       dd ?
PRC_0_shadow    dd ?
                ENDIF

                IFDEF SBPRO
MIXADDR         dd ?
MIXDATA         dd ?
                ENDIF

                IFDEF SBLASTER
DSP_RESET       dd ?                    ;IO_Addr+06h
DSP_READ        dd ?                    ;+0Ah
DSP_WRITE_STAT  dd ?                    ;+0Ch
DSP_DATA_RDY    dd ?                    ;+0Eh
                ENDIF

DSP_IRQ         dd ?
DSP_DMA         dd ?

playing         dd ?
main_volume     dd ?
panpot_val      dd ?
block_seg_ptr   dd ?                    ;real-mode far pointer

block_far_ptr   LABEL FAR PTR
block_off       dd ?                    ;far pointer offset
block_sel       dd ?                    ;far pointer segment

old_IRQ_o       dd ?
old_IRQ_s       dd ?
old_IRQ_real    dd ?
packing         dd ?
stereo_flag     dd ?
current_rate    dd ?
pack_byte       dd ?
DMA_ptr         dd ?
DMA_len         dd ?
blk_len         dd ?
loop_off        dd ?
loop_seg_ptr    dd ?
loop_cnt        dd ?
IRQ_confirm     dd ?

buff_seg_data   dd 2 dup (?)
buff_len        dd 2 dup (?)
buff_pack       dd 2 dup (?)
buff_sample     dd 2 dup (?)
buff_status     dd 2 dup (?)

buffer_mode     dd ?                   
DAC_status      dd ?
current_buffer  dd ?

PIC0_val        db ?
PIC1_val        db ?

local_DS        dw ?

sample_cnt      dd ?
spkr_status     dd ?
old_freq        dd ?
old_stereo      dd ?

xblk_status     dd ?
xblk_tc         dd ?
xblk_pack       dd ?

int_vector      dd ?

silence_flag    dd ?

init_OK         dd 0

                IFDEF PAS

MV_filter       dd ?
MV_xchannel     dd ?

MVP_name        db 'MVPROAS',0

MV_vl_txt       db 'SET OUTPUT MIXER LEFT PCM TO '
MV_vl           db '0'
                db '0'
                db '0'
                db '%',13
MV_vl_len       equ ($-MV_vl_txt)

MV_vr_txt       db 'SET OUTPUT MIXER RIGHT PCM TO '
MV_vr           db '0'
                db '0'
                db '0'
                db '%',13
MV_vr_len       equ ($-MV_vr_txt)

;----------------------------------------------------------------------------
                IFDEF DPMI
DPMI_real_int   LABEL BYTE            ;DPMI real-mode interrupt structure

int_DI          dw ?                  ;*** MUST REMAIN CONTIGUOUS ***
                dw 0            
int_SI          dw ?
                dw 0
int_BP          dw ?
                dw 0
                dd 0
int_BX          dw ?
                dw 0
int_DX          dw ?
                dw 0
int_CX          dw ?
                dw 0
int_AX          dw ?
                dw 0
int_flags       dw ?
int_ES          dw ?
int_DS          dw ?
                dw 0
                dw 0
                dw 0
                dw 0
                dw 0
                dw 0

                ELSEIFDEF INT21
X32_real_int    LABEL BYTE            ;Flashtek X32 real-mode int structure

interrupt_num   dw ?                  ;*** MUST REMAIN CONTIGUOUS ***
selector_ds     dw ?
selector_es     dw ?
selector_fs     dw ?
selector_gs     dw ?
register_eax    dd ?
register_edx    dd ?

                ENDIF
;----------------------------------------------------------------------------

                ENDIF

default_vol     LABEL DWORD
                IFDEF SBPRO
                dd 110
                ELSEIFDEF PAS
                dd 80
                ELSE
                dd 127
                ENDIF

default_pan     dd 64

                IFDEF SBLASTER
pack_opcodes    dd 14h,75h,77h,17h      ;type 0-3, init xfer
                dd 14h,74h,76h,16h      ;type 0-3, cont xfer
                ENDIF

DMAPAG_offset   db 07h,03h,01h,02h,-1,0bh,09h,0ah

                IFDEF PAS
                IF PAS_FILTER
filter_cutoff   dd 17897,15909,11931,8948,5965,2982
filter_value    db 00001b,00010b,01001b,10001b,11001b,00100b
                ENDIF
                ENDIF

                IFDEF ADLIBG
selected_IRQ    db 3,4,5,7,10,11,12,15

PCM_Hz          dd 44100,22050,11025,7350
ADPCM_Hz        dd 22050,11025,7350,5513

freq_bits       db 00000000b,00001000b,00010000b,00011000b
pack_modes      db 0,1,128,129,4,132

;                  m8 PCM    m4 ADPCM  s8 PCM    s4 ADPCM  m16 PCM   s16 PCM
PRC_0_values    db 01100110b,01100010b,01000110b,00100010b,01100110b,01000110b
PRC_1_values    db 00000000b,00000000b,00100110b,01000010b,00000000b,00100110b

SFC_0_values    db 00000101b,00000101b,10000101b,10000101b,01000101b,11000101b
SFC_1_values    db 00000010b,00000010b,00000011b,00000011b,00000010b,01000011b
                ENDIF

                IFDEF STEREO
pan_graph       db 0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30                      
                db 32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62                
                db 64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94                
                db 96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,127  
                db 127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127
                db 127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127
                db 127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127
                db 127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127
                ENDIF

;****************************************************************************
;*                                                                          *
;*  Interface primitives                                                    *
;*                                                                          *
;****************************************************************************

                IFDEF SBLASTER

send_timeout    PROC USES ebx esi edi,\
                BData

                IF ALWAYS_ACK_IRQ

                mov edx,DSP_DATA_RDY    ;acknowledge any false IRQs that may
                in al,dx                ;be preventing SB port writes

                ENDIF

                mov ecx,200h
                mov edx,DSP_WRITE_STAT
__poll_cts:     in al,dx
                test al,80h
                jz __cts
                loop __poll_cts
                mov eax,0
                jmp __exit

__cts:          mov eax,[BData]
                out dx,al
                mov eax,-1
__exit:         
                ret
send_timeout    ENDP

;****************************************************************************
read_timeout    PROC USES ebx esi edi

                mov edx,DSP_DATA_RDY
                mov ecx,200h
__poll_rdy:     in al,dx
                test al,80h
                jnz __rdy
                loop __poll_rdy
                mov eax,-1
                jmp __exit

__rdy:          mov eax,0
                mov edx,DSP_READ
                in al,dx
__exit:
                ret
read_timeout    ENDP

;****************************************************************************
send_byte       PROC USES ebx esi edi,\
                BData
                LOCAL retry

                IF ALWAYS_ACK_IRQ

                mov edx,DSP_DATA_RDY    ;acknowledge any false IRQs that may
                in al,dx                ;be preventing SB port writes

                ENDIF

                mov retry,0

__try:          mov edx,DSP_WRITE_STAT
                    
                mov ecx,30000h
__wait_free_1:  sub ecx,1
                js __timeout
                in al,dx
                or al,al
                js __wait_free_1

__write_byte:   mov eax,[BData]
                out dx,al

                mov ecx,30000h          ;wait until command received 
__wait_free_2:  sub ecx,1
                js __exit
                in al,dx
                or al,al
                js __wait_free_2

__exit:         ret

__timeout:      cmp retry,0             ;already retried?
                jne __write_byte        ;yes, continue & return

                inc retry               

                mov edx,DSP_DATA_RDY    ;acknowledge any false IRQs that may
                in al,dx                ;be preventing SB port writes

                jmp __try

send_byte       ENDP

                ELSEIFDEF ADLIBG

;****************************************************************************
IO_wait         PROC USES ebx esi edi

                mov ecx,500
                mov edx,CTRL_ADDR
__wait:         in al,dx
                and al,01000000b
                loopnz __wait

                ret
IO_wait         ENDP

;****************************************************************************
enable_ctrl     PROC USES ebx esi edi

                mov edx,CTRL_ADDR   
                mov al,0ffh               
                out dx,al

                ret
enable_ctrl     ENDP

;****************************************************************************
disable_ctrl    PROC USES ebx esi edi

                invoke IO_wait

                mov edx,CTRL_ADDR   
                mov al,0feh               
                out dx,al

                ret
disable_ctrl    ENDP

;****************************************************************************
get_ctrl_reg    PROC USES ebx esi edi,\
                RegNum                  ;Get control chip register value

                invoke IO_wait

                mov edx,CTRL_ADDR
                mov eax,[RegNum]
                out dx,al

                invoke IO_wait

                mov eax,0
                mov edx,CTRL_DATA
                in al,dx

                ret
get_ctrl_reg    ENDP

;****************************************************************************
set_ctrl_reg    PROC USES ebx esi edi,\
                RegNum,Val              ;Set control chip register value

                invoke IO_wait

                mov edx,CTRL_ADDR
                mov eax,[RegNum]
                out dx,al

                invoke IO_wait

                mov edx,CTRL_DATA
                mov eax,[Val]
                out dx,al
                ret
set_ctrl_reg    ENDP

;****************************************************************************
MMA_wait        PROC USES ebx esi edi

                mov ecx,200
__kill_time:    jmp $+2
                loop __kill_time
                ret
MMA_wait        ENDP

;****************************************************************************
MMA_write       PROC USES ebx esi edi,\
                Chan,Reg,Val            ;Write byte to MMA register

                mov eax,[Reg]
                mov edx,DSP_ADDR
                out dx,al

                invoke MMA_wait

                mov edx,[Chan]
                shl edx,1
                add edx,DSP_DATA
                mov eax,[Val]
                out dx,al

                invoke MMA_wait
                ret
MMA_write       ENDP

                ENDIF

;****************************************************************************
reset_DSP       PROC USES ebx esi edi

                IFDEF PAS

                mov eax,1

                ELSEIFDEF ADLIBG

                mov eax,0
                mov ebx,9
                mov ecx,10000000b
                invoke MMA_write,eax,ebx,ecx

                mov eax,0
                mov ebx,9
                mov ecx,01110110b
                invoke MMA_write,eax,ebx,ecx

                mov eax,1
                mov ebx,9
                mov ecx,10000000b
                invoke MMA_write,eax,ebx,ecx

                mov eax,1
                mov ebx,9
                mov ecx,01110110b
                invoke MMA_write,eax,ebx,ecx

                mov eax,01110110b
                mov PRC_0_shadow,eax
                
                mov eax,1

                ELSEIFDEF SBLASTER

                mov edx,DSP_RESET       ;assert reset
                mov al,1
                out dx,al

                mov ecx,20
__wait:         in al,dx                ;wait > 3 uS
                loop __wait

                mov al,0                ;drop reset
                out dx,al

                mov esi,10h             ;try 16 times
__try_read:     invoke read_timeout
                cmp eax,0aah
                je __exit               ;(reset succeeded)
                dec esi
                jnz __try_read

                mov eax,0               ;return 0 if failed

                ENDIF
__exit:
                ret
reset_DSP       ENDP

;****************************************************************************
;*                                                                          *
;*  Internal procedures                                                     *
;*                                                                          *
;****************************************************************************

sysex_wait      PROC USES ebx esi edi,\     ;Generate a 14-millisecond (typ.)
                Delay                   ;delay with interrupts enabled
                                        
                pushfd                  ;Requires CGA/EGA/VGA/XGA video)
                sti

                mov ebx,63h
                GET_BIOS_DATA

                mov edx,eax
                add edx,6               ;get CRTC Status register location

                mov ecx,[Delay]
                jecxz __exit

__sync_1:       in al,dx                ;wait for leading edge of vertical
                test eax,8              ;retrace signal
                jz __sync_1             

__sync_2:       in al,dx
                test eax,8
                jnz __sync_2

                loop __sync_1

__exit:         POP_F
                ret
sysex_wait      ENDP

;****************************************************************************
                IFDEF PAS

INT2F           PROC\                   ;Perform DPMI call to real-mode INT 2F
                USES esi edi\           ;for Pro Audio Spectrum driver access
                ,regAX,regBX

                IFDEF DPMI              ;Rational Systems DOS/4GW

                mov ax,WORD PTR [regAX]
                mov int_AX,ax

                mov ax,WORD PTR [regBX]
                mov int_BX,ax

                mov int_DS,0
                mov int_ES,0

                pushf
                pop ax
                mov int_flags,ax

                mov eax,0300h
                mov ebx,002fh
                mov ecx,0
                mov edi,OFFSET DPMI_real_int
                int 31h

                mov ax,int_AX
                mov bx,int_BX
                mov cx,int_CX
                mov dx,int_DX
                ret

                ELSEIFDEF INT21         ;Flashtek X32

                mov interrupt_num,2fh

                mov selector_ds,ds
                mov selector_es,es
                mov selector_fs,fs
                mov selector_gs,gs

                mov eax,[regAX]
                mov ebx,[regBX]

                mov register_eax,eax
                mov register_edx,0

                mov eax,2511h
                mov edx,OFFSET X32_real_int
                int 21h

                mov edx,register_edx
                ret

                ENDIF

INT2F           ENDP

                ENDIF

;****************************************************************************
sub_ptr         PROC USES ebx esi esi,\
                Off1,Seg1,Off2,Seg2     ;Return DX:AX = ptr 2 - ptr 1

                mov ax,WORD PTR [Seg2] 
                mov dx,0
                REPT 4
                shl ax,1
                rcl dx,1
                ENDM
                add ax,WORD PTR [Off2]
                adc dx,0

                mov bx,WORD PTR [Seg1]
                mov cx,0
                REPT 4
                shl bx,1
                rcl cx,1
                ENDM
                add bx,WORD PTR [Off1]
                adc cx,0

                sub ax,bx
                sbb dx,cx

                ret
sub_ptr         ENDP

;****************************************************************************
block_type      PROC USES ebx esi edi es

                les esi,block_far_ptr   ;Return EAX=current block type
                mov eax,0
                mov al,es:[esi]

                ret
block_type      ENDP

;****************************************************************************
set_xblk        PROC USES ebx esi edi es

                les esi,block_far_ptr        
                cmp BYTE PTR es:[esi],8
                jne __exit              ;(not an extended block)

                mov eax,0

                mov al,es:[esi+5]       ;get extended voice parameters
                mov xblk_tc,eax         ;high byte of TC = normal sample rate

                mov ax,es:[esi+6]       ;get pack (AL) and mode (AH)
                cmp ah,1                ;stereo?
                jne __set_pack

                or al,80h               ;yes, make pack byte negative

__set_pack:     and eax,0ffh
                mov xblk_pack,eax

                mov xblk_status,1       ;flag extended block override
__exit:         
                ret
set_xblk        ENDP

;****************************************************************************
marker_num      PROC USES ebx esi edi es

                les esi,block_far_ptr   ;Return EAX=block's marker #
                cmp BYTE PTR es:[esi],4
                mov eax,-1       
                jne __exit              ;(not a marker block)

                mov eax,0               ;return marker #
                mov ax,WORD PTR es:[esi+4] 
__exit:         
                ret
marker_num      ENDP

                IFDEF SBLASTER

;****************************************************************************
DAC_spkr_on     PROC USES ebx esi edi

                cmp spkr_status,1
                je __exit               ;already on, exit
                mov spkr_status,1

                invoke send_byte,0d1h
                invoke sysex_wait,8     ;112 ms delay (SB Dev Kit, p. 14-11)
__exit:
                ret
DAC_spkr_on     ENDP

;****************************************************************************
DAC_spkr_off    PROC USES ebx esi edi

                cmp spkr_status,0
                je __exit               ;already off, exit
                mov spkr_status,0

                invoke halt_DMA
                invoke send_byte,0d3h
                invoke sysex_wait,16    ;224 ms delay (SB Dev Kit, p. 14-11)
__exit:
                ret
DAC_spkr_off    ENDP

                ENDIF

;****************************************************************************
continue_DMA    PROC USES ebx esi edi

                IFDEF PAS

                mov eax,MV_filter
                or al,01000000b
                mov edx,AUDIOFILT
                out dx,al               ;start the transfer
                mov MV_filter,eax

                ELSEIFDEF ADLIBG

                mov eax,PRC_0_shadow
                or eax,00000001b        ;set GO bit
                mov ebx,0
                mov ecx,9
                invoke MMA_write,ebx,ecx,eax

                ELSEIFDEF SBLASTER

                mov eax,0d4h
                invoke send_byte,eax

                ENDIF

                mov playing,1

                ret
continue_DMA    ENDP

;****************************************************************************
halt_DMA        PROC USES ebx esi edi

                IFDEF PAS

                mov eax,MV_filter
                and al,10111111b
                mov edx,AUDIOFILT
                out dx,al               ;suspend the transfer
                mov MV_filter,eax

                ELSEIFDEF ADLIBG

                mov eax,PRC_0_shadow
                and eax,11111110b       ;clear GO bit
                mov ebx,0
                mov ecx,9
                invoke MMA_write,ebx,ecx,eax

                ELSEIFDEF SBLASTER

                pushfd                  ;save i-flag status

                mov edx,DSP_WRITE_STAT  ;register busy flag
                mov ecx,70000h

__wait_busy:    sub ecx,1
                js __send_halt

                sti                     ;wait for busy status, abort if 
                jmp $+2                 ;playback ends by itself
                jmp $+2
                jmp $+2
                cmp playing,0
                je __not_playing
                cli

                in al,dx
                or al,al
                jns __wait_busy         

                mov ecx,32768           ;wait for free edge
__wait_free:    dec ecx
                jz __send_halt
                in al,dx
                or al,al
                js __wait_free

__send_halt:    mov al,0d0h             ;send halt DMA opcode
                out dx,al

__not_playing:  POP_F                   ;recover i-flag

                ENDIF

                mov playing,0
                
                ret
halt_DMA        ENDP

;****************************************************************************
                IFNDEF SBLASTER         

match_constant  PROC USES ebx esi edi,\
                TAddr,TSize,N           ;Get entry index for nearest match to
                                        ;constant N
                cld
                mov esi,[TAddr]
                mov ebx,0ffffh
                mov ecx,0
                mov edi,0

__abs_delta:    lods DWORD PTR cs:[esi]
                sub eax,[N]
                cdq
                xor eax,edx
                sub eax,edx

                cmp eax,ebx
                ja __next
                
                mov ebx,eax
                mov edi,ecx

__next:         inc ecx
                cmp ecx,[TSize]
                jne __abs_delta

                mov eax,edi
                ret
match_constant  ENDP
                ENDIF

;****************************************************************************
set_sample_rate PROC USES ebx esi edi,\
                SB_Rate,StereoSample
                LOCAL freq

                pushfd
                cli                     ;make sure IRQ's are off

                mov eax,[SB_Rate]       ;f in Hz. = 1E6 / (256 - SB_Rate)
                and eax,0ffh
                mov ebx,256
                sub ebx,eax
                mov eax,1000000
                mov edx,0
                div ebx
                mov freq,eax

                IFDEF PAS

                mov eax,1
                mov ecx,freq
                mov ebx,[StereoSample]

                cmp ecx,old_freq        ;avoid clicks by sending
                jne __new_parms         ;new settings only when changed
                cmp ebx,old_stereo
                je __exit

__new_parms:    mov old_freq,ecx
                mov old_stereo,ebx

	mov edx,PCMDATA         ;silence PCM output
	mov al,80h
	out dx,al

                mov eax,1193180
                mov edx,0
                div freq
                mov ecx,eax
                                                
                mov al,00110110b        ;timer 0, square wave, binary mode
                mov edx,TMRCTLR
                out dx,al
                mov edx,SAMPLERATE
                mov al,cl
                out dx,al
                jmp $+2
                mov al,ch
                out dx,al

                IF PAS_FILTER
                mov eax,OFFSET filter_cutoff   
                mov ecx,freq            ;select filter for freq in Hz. / 2
                shr ecx,1
                mov ebx,6
                invoke match_constant,eax,ebx,ecx
                mov esi,eax             ;ESI = index into filter_value

                mov eax,MV_filter
                and al,11100000b
                or al,filter_value[esi]
                mov edx,AUDIOFILT
                out dx,al
                mov MV_filter,eax
                ENDIF

                ELSEIFDEF ADLIBG

                mov eax,1
                mov ecx,freq
                mov ebx,[StereoSample]

                cmp ecx,old_freq        ;avoid Ad Lib Gold clicks by sending
                jne __new_parms         ;new settings only when changed
                cmp ebx,old_stereo
                je __exit

__new_parms:    mov old_freq,ecx
                mov old_stereo,ebx

                mov eax,pack_byte
                and eax,10000111b
                mov esi,0
__find_pack:    cmp pack_modes[esi],al
                je __pack_found
                inc esi
                cmp esi,6
                jne __find_pack
                mov esi,0
__pack_found:   mov pack_mode,esi

                mov eax,OFFSET ADPCM_Hz
                cmp esi,1
                je __find_freq
                cmp esi,3
                je __find_freq
                mov eax,OFFSET PCM_Hz

__find_freq:    cmp ebx,0
                je __lookup
                shr ecx,1                       ;sample rate /= 2 for stereo

__lookup:       mov ebx,4
                invoke match_constant,eax,ebx,ecx
                mov edi,eax

                mov eax,0
                mov ebx,9
                mov ecx,10000000b
                invoke MMA_write,eax,ebx,ecx    ;reset both FIFOs

                mov eax,1
                mov ebx,9
                mov ecx,10000000b
                invoke MMA_write,eax,ebx,ecx

                REPT 4                          
                mov eax,0                       ;write 4 dummy bytes to
                mov ebx,11                      ;allow proper FIFO DMA 
                mov ecx,0                       ;initialization
                invoke MMA_write,eax,ebx,ecx
                ENDM

                mov eax,0
                mov al,freq_bits[edi]
                or al,PRC_0_values[esi]
                mov PRC_0_shadow,eax
                mov ebx,0
                mov ecx,9
                invoke MMA_write,ebx,ecx,eax

                mov al,freq_bits[edi]
                or al,PRC_1_values[esi]
                mov ebx,1
                mov ecx,9
                invoke MMA_write,ebx,ecx,eax

                mov al,SFC_0_values[esi]
                mov ebx,0
                mov ecx,12
                invoke MMA_write,ebx,ecx,eax

                mov al,SFC_1_values[esi]
                mov ebx,1
                mov ecx,12
                invoke MMA_write,ebx,ecx,eax

                ELSEIFDEF SBLASTER

                mov eax,40h
                invoke send_byte,eax
                invoke send_byte,[SB_Rate]

                ENDIF

                mov eax,1

__exit:         POP_F
                ret
set_sample_rate ENDP

                IFDEF PAS
;****************************************************************************
decstr          PROC USES ebx esi edi es,\  ;Decimal ASCII to string buffer
                Buf,Num                 ;(no lz suppression)
                LOCAL accum

                cld

                push ds
                pop es

                mov edi,[Buf]
                mov eax,[Num]
                mov accum,eax

                mov ecx,100
__div_loop:     mov eax,accum
                mov edx,0
                div ecx
                mov accum,edx
                add eax,'0'

                stosb

                mov eax,ecx
                mov edx,0
                mov ebx,10
                div ebx
                mov ecx,eax
                cmp eax,0
                jne __div_loop

                ret
decstr          ENDP

;****************************************************************************
send_MV_cmd     PROC USES ebx esi edi,\ ;Send command string to MVPROAS
                String,Len              ;(device alias for MVSOUND.SYS)
                LOCAL handle

                mov eax,3d02h           ;open MVPROAS for writing
                mov edx,OFFSET MVP_name
                int 21h
                jc __exit
                mov handle,eax

                mov ebx,eax             ;write the command string
                mov ah,40h
                mov ecx,[Len]
                mov edx,[String]
                int 21h                 

                mov ah,3eh              ;close MVPROAS
                mov ebx,handle
                int 21h                 
__exit:
                ret
send_MV_cmd     ENDP
                ENDIF

;****************************************************************************
set_volume      PROC USES ebx esi edi

                IFDEF STEREO            ;Establish output lvl w/vol, pan

                mov esi,127
                sub esi,panpot_val
                movzx eax,pan_graph[esi]
                mul BYTE PTR main_volume
                mov edi,eax             ;DI = right volume 0-16129
                mov esi,panpot_val
                movzx eax,pan_graph[esi]
                mul BYTE PTR main_volume
                mov esi,eax             ;SI = left volume 0-16129

                IFDEF SBPRO

                mov eax,edi
                mov ecx,10
                shr eax,cl
                mov bh,al               ;right volume 0-15
                mov eax,esi
                mov ecx,6
                shr eax,cl
                and eax,11110000b
                mov bl,al               ;left volume 0-15
                mov edx,MIXADDR
                mov al,4                ;select voice volume register
                out dx,al
                jmp $+2
                mov edx,MIXDATA
                or bl,bh
                mov al,bl
                out dx,al             

                ELSEIFDEF PAS

                mov ecx,161
                mov eax,edi
                mov edx,0
                div ecx                 ;AX=right volume 0-100%
                mov edi,OFFSET MV_vr
                invoke decstr,edi,eax
                mov edi,OFFSET MV_vr_txt
                mov ecx,MV_vr_len
                invoke send_MV_cmd,edi,ecx
                mov ecx,161
                mov eax,esi
                mov edx,0
                div ecx                 ;AX=left volume 0-100%
                mov edi,OFFSET MV_vl
                invoke decstr,edi,eax
                mov edi,OFFSET MV_vl_txt
                mov ecx,MV_vl_len
                invoke send_MV_cmd,edi,ecx

                ELSEIFDEF ADLIBG

                cmp stereo_flag,0       ;ALG panning works only in stereo mode
                jne __set_vol
                add esi,edi
                shr esi,1
                mov edi,esi

__set_vol:      mov ecx,6
                shr edi,cl              ;right volume 0-252
                mov eax,0
                mov ebx,10
                invoke MMA_write,eax,ebx,edi

                mov ecx,6
                shr esi,cl              ;left volume 0-252
                mov eax,1
                mov ebx,10
                invoke MMA_write,eax,ebx,esi

                ENDIF

                ENDIF                   ;IFDEF STEREO

                IFDEF SBLASTER
                invoke DAC_spkr_on
                ENDIF
__exit:
                ret
set_volume      ENDP

;****************************************************************************
DMAC_word_cnt   PROC USES ebx esi edi

                pushfd
                cli

                cmp silence_flag,0      ;if silence packing was used, return
                mov silence_flag,0      ;0ffffh (transfer done)
                mov ax,0ffffh
                jne __exit              

                mov edx,DSP_DMA
                shl edx,1
                add edx,1                
                in al,dx                ;DMAnCNT: Channel n Word Count
                mov bl,al
                in al,dx
                mov ah,al
                mov al,bl

__exit:         POP_F
                ret

DMAC_word_cnt   ENDP

;****************************************************************************
program_DMAC    PROC USES ebx esi edi,\
                DAddr,DPage,Len

                pushfd                  ;make sure interrupts are off
                cli

                IFDEF PAS
                mov eax,MV_xchannel
                mov edx,CROSSCHANNEL
                or eax,10000000b        ;secure the DMA channel
                out dx,al
                mov MV_xchannel,eax
                ENDIF

                mov eax,DSP_DMA
                or eax,4h               ;DMASET: Set bit (mask or request)
                out 0ah,al   

                mov ebx,DSP_DMA
                mov edx,80h             ;DMAPAG: Base address
                add dl,DMAPAG_offset[ebx]
                mov eax,[DPage]
                out dx,al

                mov eax,0
                out 0ch,al              ;DMACLFF: Clear Byte Pointer Flip Flop
                
                mov edx,DSP_DMA
                shl edx,1
                mov eax,[DAddr]
                out dx,al               ;DMAnADR: Channel n Current Address
                mov al,ah
                out dx,al

                mov edx,DSP_DMA
                shl edx,1
                add edx,1
                mov eax,[Len]
                out dx,al               ;DMAnCNT: Channel n Word Count
                mov al,ah
                out dx,al

                mov eax,48h             
                or eax,DSP_DMA
                out 0bh,al              ;DMAMD: Write Mode Register

                mov eax,DSP_DMA
                or eax,0h               ;DMARST: Reset bit (mask or request)
                out 0ah,al   

                POP_F
                ret
program_DMAC    ENDP

;****************************************************************************
IRQ_test        PROC                    ;DMA IRQ handler for IRQ detection

                push edx
                push eax
                push ds

                mov ds,cs:local_DS

                IF CHECK_END_DMA
                IF CHECK_ISR

                cmp DSP_IRQ,7
                jne __DMAC_OK

                mov al,00001011b        ;OCW3: read ISR 
                out 20h,al              ;(Intel Peripheral Components, 1991,
                jmp $+2                 ; p. 3-188)
                in al,20h               
                test al,80h             
                jz __exit               ;ignore IRQ if ISR bit not set

__DMAC_OK:
                ENDIF
                ENDIF

                IFDEF SBLASTER

                mov edx,DSP_DATA_RDY
                in al,dx                ;acknowledge the interrupt

                mov eax,DSP_DMA         ;mask DMA channel
                or eax,4h                        
                out 0ah,al   

                ENDIF

                mov IRQ_confirm,1       ;flag interrupt received OK
                mov playing,0

                mov al,20h              ;send EOI to PIC
                cmp DSP_IRQ,8           ;clear PIC1 if IRQ >= 8
                jb __master
                out 0a0h,al
__master:       out 20h,al

__exit:         pop ds
                pop eax
                pop edx

                iretd
IRQ_test        ENDP

;****************************************************************************
IRQ_play_VOC    PROC                    ;DMA IRQ handler for .VOC file output
                     
                push eax
                push ebx
                push ecx
                push edx
                push esi
                push edi
                push ebp
                push ds
                push es
                cld

                mov ds,cs:local_DS

                IFDEF ADLIBG
                mov edx,DSP_ADDR
                in al,dx
                and eax,00000001b       ;FIF0 interrupt?
                jz __EOI                ;no, exit
                ENDIF

                IF CHECK_END_DMA

                cmp DSP_IRQ,7           ;(spurious IRQs occur only on IRQ 7)
                jne __DMAC_OK

                IF CHECK_ISR

                mov al,00001011b        ;OCW3: read ISR 
                out 20h,al              ;(Intel Peripheral Components, 1991,
                jmp $+2                 ; p. 3-188)
                in al,20h               
                test al,80h             
                jz __exit               ;ignore IRQ if ISR bit not set

                ELSE

                invoke DMAC_word_cnt    ;see if DMA transfer is truly over
                cmp ax,0                
                je __DMAC_OK            ;(can occur during packed transfers)
                cmp ax,0ffffh
                jne __EOI

                ENDIF
__DMAC_OK:
                ENDIF

                IFDEF PAS

                mov edx,INTRCTLRST
                in al,dx                ;acknowledge the interrupt
                test eax,00001000b
                jz __EOI                ;it wasn't caused by our board, exit
                out dx,al

                mov eax,MV_xchannel
                mov edx,CROSSCHANNEL
                and eax,10111111b       ;kill the PCM engine until re-done
                out dx,al
                mov MV_xchannel,eax

                ELSEIFDEF SBLASTER

                mov edx,DSP_DATA_RDY
                in al,dx                ;acknowledge the interrupt

                ENDIF

                cmp playing,0
                je __EOI
                mov playing,0

                IFDEF PAS
                invoke halt_DMA
                ENDIF

                mov eax,DSP_DMA         ;mask DMA channel
                or eax,4h                        
                out 0ah,al   

                mov eax,DMA_len         ;at end of block?
                cmp eax,0
                je __end_of_block

                invoke xfer_chunk       ;no, send next chunk
                jmp __EOI

__end_of_block: invoke next_block       ;else go on to next block in chain
                invoke process_block

__EOI:          mov al,20h              ;send EOI to PIC
                cmp DSP_IRQ,8           ;clear PIC1 if IRQ >= 8
                jb __master
                out 0a0h,al
__master:       out 20h,al

__exit:         pop es
                pop ds
                pop ebp
                pop edi
                pop esi
                pop edx
                pop ecx
                pop ebx
                pop eax

                iretd
IRQ_play_VOC    ENDP

;****************************************************************************
IRQ_play_buffer PROC                    ;DMA IRQ handler for double-buffering
                     
                push eax
                push ebx
                push ecx
                push edx
                push esi
                push edi
                push ebp
                push ds
                push es
                cld

                mov ds,cs:local_DS

                IFDEF ADLIBG
                mov edx,DSP_ADDR
                in al,dx
                and eax,00000001b       ;FIF0 interrupt?
                jz __EOI                ;no, exit
                ENDIF

                IF CHECK_END_DMA

                cmp DSP_IRQ,7           ;(spurious IRQs occur only on IRQ 7)
                jne __DMAC_OK

                IF CHECK_ISR

                mov al,00001011b        ;OCW3: read ISR 
                out 20h,al              ;(Intel Peripheral Components, 1991,
                jmp $+2                 ; p. 3-188)
                in al,20h               
                test al,80h             
                jz __exit               ;ignore IRQ if ISR bit not set

                ELSE

                invoke DMAC_word_cnt    ;see if DMA transfer is truly over
                cmp ax,0                
                je __DMAC_OK            ;(can occur during packed transfers)
                cmp ax,0ffffh
                jne __EOI

                ENDIF
__DMAC_OK:
                ENDIF

                IFDEF PAS

                mov edx,INTRCTLRST
                in al,dx                ;acknowledge the interrupt
                test eax,00001000b
                jz __EOI                ;it wasn't caused by our board, exit
                out dx,al

                mov eax,MV_xchannel
                mov edx,CROSSCHANNEL
                and eax,10111111b       ;kill the PCM engine until re-done
                out dx,al
                mov MV_xchannel,eax

                ELSEIFDEF SBLASTER

                mov edx,DSP_DATA_RDY
                in al,dx                ;acknowledge the interrupt

                ENDIF

                cmp playing,0
                je __EOI
                mov playing,0

                IFDEF PAS
                invoke halt_DMA
                ENDIF

                mov eax,DSP_DMA         ;mask DMA channel
                or eax,4h                        
                out 0ah,al   

                mov eax,DMA_len         ;at end of block?
                cmp eax,0
                je __end_of_block
                     
                invoke xfer_chunk       ;no, send next chunk
                jmp __EOI

__end_of_block: mov ebx,current_buffer  ;else look for an unplayed buffer...
                mov buff_status[ebx],DAC_DONE

                invoke next_buffer
                cmp eax,-1
                je __EOI                ;no buffers left, terminate playback
                invoke process_buffer,eax

__EOI:          mov al,20h              ;send EOI to PIC
                cmp DSP_IRQ,8           ;clear PIC1 if IRQ >= 8
                jb __master
                out 0a0h,al
__master:       out 20h,al

__exit:         pop es
                pop ds
                pop ebp
                pop edi
                pop esi
                pop edx
                pop ecx
                pop ebx
                pop eax

                iretd
IRQ_play_buffer ENDP

;****************************************************************************
hardware_xfer   PROC USES ebx esi edi

                IFDEF PAS

                mov ecx,blk_len
                jecxz __exit
                add ecx,1               ;1-65535; 0=65536

                mov eax,01110100b       ;program sample buffer counter
                mov edx,TMRCTLR
                out dx,al
                mov edx,SAMPLECNT
                mov al,cl
                out dx,al
                jmp $+2
                mov al,ch
                out dx,al

                mov ah,BYTE PTR MV_xchannel      
                and ah,00001111b        ;reset PCM state machine
                mov al,10110000b
                cmp stereo_flag,0
                je __set_xchan
                mov al,10010000b
__set_xchan:    or al,ah
                mov edx,CROSSCHANNEL
                out dx,al
                jmp $+2
                or al,01000000b
                out dx,al
                mov BYTE PTR MV_xchannel,al

                mov eax,MV_filter
                or eax,11000000b
                mov edx,AUDIOFILT
                out dx,al               ;start the transfer
                mov MV_filter,eax

                ELSEIFDEF ADLIBG

                mov eax,PRC_0_shadow
                or eax,00000001b        ;set GO bit
                mov ebx,0
                mov ecx,9
                invoke MMA_write,ebx,ecx,eax

                ELSEIFDEF SBLASTER

                mov ebx,packing         ;program SB DSP to transfer data
                invoke send_byte,pack_opcodes[ebx*4]

                mov eax,blk_len
                and eax,0ffh
                invoke send_byte,eax

                mov eax,blk_len
                and eax,0ff00h
                xchg al,ah
                invoke send_byte,eax

                ENDIF

                mov playing,1
__exit:
                ret
hardware_xfer   ENDP

;****************************************************************************
xfer_chunk      PROC USES ebx esi edi

                mov si,WORD PTR DMA_ptr ;Get addr, size of next chunk; send it
                mov di,WORD PTR DMA_ptr+2
                FAR_TO_HUGE di,si       
                mov cx,di               ;CX:SI = start of data to send
                and di,0f000h  
                add di,1000h            ;DI:0000 = start of next physical page
                                        
                mov ax,0                ;calculate DI:0000 - CX:SI
                push cx
                invoke sub_ptr,esi,ecx,eax,edi
                pop cx
                sub eax,1
                and eax,0ffffh
                mov blk_len,eax         ;AX = # of bytes left in page -1

                mov eax,DMA_len         ;set EAX = total # of bytes left -1
                sub eax,1
                cmp eax,65535           ;>= 64K?
                ja __len_valid          ;yes, send rest of current page only
                cmp eax,blk_len         ;> # of bytes left in page?
                ja __len_valid          ;yes, send rest of current page only

                mov blk_len,eax         ;else send all remaining data

__len_valid:    mov dx,0                ;program DMA controller with chunk len
                REPT 4                  ;and addr (CX:SI)
                shl cx,1
                rcl dx,1
                ENDM
                add cx,si
                adc dx,0
                invoke program_DMAC,ecx,edx,blk_len

                IFDEF SBPRO
                mov edx,MIXADDR
                mov eax,0eh             ;select DNFI/VSTC flag set
                out dx,al
                jmp $+2
                mov edx,MIXDATA
                mov eax,stereo_flag
                out dx,al               ;Set stereo/mono mode; filtering = ON
                ENDIF

                invoke hardware_xfer

                mov si,WORD PTR DMA_ptr ;add len of chunk +1 to DMA pointer
                mov di,WORD PTR DMA_ptr+2
                mov ax,WORD PTR blk_len
                mov dx,WORD PTR blk_len+2
                add ax,1
                adc dx,0
                ADD_PTR ax,dx,di,si
                mov WORD PTR DMA_ptr,si
                mov WORD PTR DMA_ptr+2,di

                mov eax,DMA_len         ;subtract len of transmitted chunk +1
                sub eax,blk_len
                sub eax,1
                mov DMA_len,eax

                cmp packing,4           ;did we just send an initial chunk?
                jae __exit              
                add packing,4           ;yes, switch to "continue" opcode set
__exit:                                 ;return DX:AX = remaining bytes 
                ret
xfer_chunk      ENDP

;****************************************************************************
DMA_transfer    PROC USES ebx esi edi,\     
                DAddr,DLen              ;(requires segment:offset pointer)

                mov eax,[DAddr]
                mov DMA_ptr,eax

                mov eax,[DLen]
                mov DMA_len,eax

                invoke xfer_chunk
                ret
DMA_transfer    ENDP

;****************************************************************************
next_block      PROC USES ebx esi edi es

                les esi,block_far_ptr

                mov eax,es:[esi+1]      ;get 24-bit block len
                and eax,0ffffffh
                add eax,4               ;AX=offset to next block

                add esi,eax             ;set selector pointer to next block

                mov block_off,esi

                mov si,WORD PTR block_seg_ptr
                mov di,WORD PTR block_seg_ptr+2

                shld edx,eax,16
                ADD_PTR ax,dx,di,si     ;set segment pointer to next block

                mov WORD PTR block_seg_ptr,si
                mov WORD PTR block_seg_ptr+2,di
                ret
next_block      ENDP

;****************************************************************************
process_block   PROC USES ebx esi edi es

__do_block:     invoke block_type
                cmp eax,0               ;terminator?
                je __terminate
                cmp eax,1               ;new voice block?
                je __new_voice
                cmp eax,2               ;continued voice block?
                je __cont_voice
                cmp eax,3               ;silence period?
                je __silence
                cmp eax,4               ;marker (end of data?)
                je __terminate
                cmp eax,6               ;beginning of repeat loop?
                je __rept_loop
                cmp eax,7               ;end of repeat loop?
                je __end_loop
                cmp eax,8               ;extended block type?
                je __extended
                jmp __skip_block        ;else unrecognized block type, skip it

__extended:     invoke set_xblk
                jmp __skip_block

__terminate:    mov DAC_status,DAC_DONE
                jmp __exit

__skip_block:   invoke next_block
                jmp __do_block

__rept_loop:    les esi,block_far_ptr
                movsx eax,WORD PTR es:[esi+4]
                mov loop_cnt,eax

                invoke next_block

                mov eax,block_seg_ptr
                mov loop_seg_ptr,eax
                mov eax,block_off
                mov loop_off,eax
                jmp __do_block

__end_loop:     cmp loop_cnt,0
                je __skip_block

                mov eax,loop_off
                mov block_off,eax
                mov eax,loop_seg_ptr
                mov block_seg_ptr,eax

                cmp loop_cnt,-1
                je __do_block
                dec loop_cnt
                jmp __do_block

__silence:      IFDEF PAS

                jmp __skip_block

                ELSEIFDEF ADLIBG

                jmp __skip_block

                ELSEIFDEF SBLASTER

                mov DMA_len,0

                les esi,block_far_ptr  ;generate silent period
                movzx eax,BYTE PTR es:[esi+6]
                invoke set_sample_rate,eax,stereo_flag

                invoke send_byte,80h

                les esi,block_far_ptr

                movzx eax,BYTE PTR es:[esi+4]
                invoke send_byte,eax

                movzx eax,BYTE PTR es:[esi+5]
                invoke send_byte,eax

                mov playing,1
                mov silence_flag,1      ;set up to skip DMA test
                jmp __exit

                ENDIF

__cont_voice:   invoke set_sample_rate,current_rate,stereo_flag

                les esi,block_far_ptr
                mov eax,es:[esi+1]      ;get 24-bit block len
                and eax,0ffffffh

                mov si,WORD PTR block_seg_ptr
                mov di,WORD PTR block_seg_ptr+2
                ADD_PTR 4,0,di,si       ;DI:SI -> physical start-of-data

                shl edi,16
                mov di,si

                invoke DMA_transfer,edi,eax
                jmp __exit

__new_voice:    les esi,block_far_ptr   ;initiate output from new voice block
                mov eax,0
                mov ebx,0
                mov bl,es:[esi+4]       ;BL = TC field
                mov al,es:[esi+5]       ;AL = PACK field

                cmp xblk_status,0       ;previous extended block overrides
                je __use_vd             ;data block values
                mov eax,xblk_pack
                mov ebx,xblk_tc
                mov xblk_status,0

__use_vd:       mov pack_byte,eax
                mov packing,eax
                and packing,7fh
                and eax,80h
                mov ecx,6
                shr eax,cl
                and eax,10b
                mov stereo_flag,eax

                mov current_rate,ebx

                invoke set_sample_rate,current_rate,stereo_flag

                les esi,block_far_ptr
                mov eax,es:[esi+1]      ;get 24-bit block len
                and eax,0ffffffh
                sub eax,2               ;EAX = voice len

                mov si,WORD PTR block_seg_ptr
                mov di,WORD PTR block_seg_ptr+2
                ADD_PTR 6,0,di,si       ;DI:SI = physical pointer to data

                shl edi,16
                mov di,si

                invoke DMA_transfer,edi,eax
__exit:         

                ret
process_block   ENDP

;****************************************************************************
next_buffer     PROC USES ebx esi edi

                mov eax,0               ;buffer 0 registered?
                cmp buff_status[0*4],DAC_STOPPED
                je __return             ;yes, return its handle
                mov eax,1               ;buffer 1 registered?
                cmp buff_status[1*4],DAC_STOPPED
                je __return             ;yes, return its handle

                mov DAC_status,DAC_DONE ;else signal playback complete and
                mov eax,-1              ;return AX=-1

__return:       
                ret
next_buffer     ENDP
                     
;****************************************************************************
process_buffer  PROC USES ebx esi edi,\
                Buf

                mov esi,[Buf]           ;get buffer handle 
                shl esi,2               ;derive index

                mov buff_status[esi],DAC_PLAYING
                mov current_buffer,esi  ;save index to playing buffer

                mov eax,buff_pack[esi]
                mov pack_byte,eax
                mov packing,eax
                and packing,7fh
         
                and eax,80h
                mov ecx,6
                shr eax,cl
                and eax,10b
      
                mov stereo_flag,eax

                invoke set_sample_rate,buff_sample[esi],eax

                invoke DMA_transfer,buff_seg_data[esi],buff_len[esi]
__exit:
                ret
process_buffer  ENDP

;****************************************************************************
                IFDEF NEEDS_FORMAT      ;(if PCM format not SB-compatible)

format_block    PROC USES ebx esi edi es,\
                BufPtr,BufSeg,Len

                mov edi,[BufPtr]
                mov es,WORD PTR [BufSeg]

                mov ecx,[Len]

                or ecx,ecx
                jz __exit

__format:       sub BYTE PTR es:[edi],80h
                inc edi

                dec ecx
                jnz __format
__exit:
                ret
format_block    ENDP

                ENDIF

;****************************************************************************
;*                                                                          *
;*  Public (API-accessible) procedures                                      *
;*                                                                          *
;****************************************************************************

describe_driver PROC USES ebx esi edi

                pushfd                  ;Return CS:near ptr to DDT
                cli

                mov eax,OFFSET DDT

                POP_F
                ret
describe_driver ENDP

;****************************************************************************
shutdown_driver PROC USES ebx esi edi,\
                H,SignOff

                pushfd
                cli

                cmp init_OK,0
                je __exit

                IFDEF SBLASTER
                invoke DAC_spkr_off
                ENDIF

                invoke stop_d_pb

                IFDEF PAS

                mov eax,MV_xchannel
                mov edx,CROSSCHANNEL     ;disable DRQs from PAS
                and eax,00111111b        ;and disable PCM state machine
                out dx,al
                mov MV_xchannel,eax

                ELSEIFDEF ADLIBG

                invoke enable_ctrl
                mov eax,13h
                invoke set_ctrl_reg,eax,mask_save
                invoke disable_ctrl

                ENDIF

                invoke reset_DSP

                IFDEF SBLASTER
                invoke DAC_spkr_on
                ENDIF

                mov ecx,DSP_IRQ          ;stop hardware interrupts from DSP
                mov ebx,1
                shl ebx,cl
                in al,0a1h
                or al,bh
                and al,PIC1_val          ;don't kill any interrupts that were
                out 0a1h,al              ;initially active
                in al,21h
                or al,bl
                and al,PIC0_val
                out 21h,al

                mov eax,int_vector
                mov ebx,old_IRQ_real
                SET_REAL_VECT

                mov eax,int_vector
                mov edx,old_IRQ_o
                mov ebx,old_IRQ_s
                SET_PROT_VECT

                mov init_OK,0

__exit:         POP_F
                ret
shutdown_driver ENDP

;****************************************************************************
set_d_pb_pan    PROC USES ebx esi edi,\
                H,Pan

                pushfd                  ;Set digital playback panpot 0-127
                cli

                mov eax,[Pan]
                mov panpot_val,eax

                invoke set_volume 

                POP_F
                ret
set_d_pb_pan    ENDP

;****************************************************************************
get_d_pb_pan    PROC USES ebx esi edi

                pushfd                  ;Get digital playback panpot 0-127
                cli

                mov eax,panpot_val

                POP_F
                ret
get_d_pb_pan    ENDP

;****************************************************************************
set_d_pb_vol    PROC USES ebx esi edi,\
                H,Vol

                pushfd                  ;(0=off; anything else=on)
                cli

                mov eax,[Vol]
                mov main_volume,eax

                invoke set_volume

                POP_F
                ret
set_d_pb_vol    ENDP

;****************************************************************************
get_d_pb_vol    PROC USES ebx esi edi

                pushfd                  ;Get digital playback volume 0-127        
                cli

                mov eax,main_volume

                POP_F
                ret
get_d_pb_vol    ENDP

;****************************************************************************
detect_device   PROC USES ebx esi edi,\
                H,IO_ADDR,IRQ,DMA,DRQ
                LOCAL old_S
                LOCAL old_O
                LOCAL old_real
                LOCAL test_vect
                LOCAL PIC0_cur:BYTE
                LOCAL PIC1_cur:BYTE

                pushfd                    ;Check for presence of hardware
                cli

                mov local_DS,ds

                IFDEF SBLASTER
                push DSP_RESET       
                push DSP_READ        
                push DSP_WRITE_STAT  
                push DSP_DATA_RDY    
                push DSP_IRQ
                push DSP_DMA
                ENDIF

                IFDEF SBPRO
                push MIXDATA
                push MIXADDR
                ENDIF
                
                IFDEF ADLIBG
                push CTRL_ADDR
                push CTRL_DATA
                ENDIF

                mov spkr_status,-1

                IFDEF ADLIBG              ;ALG, check for control chip

                mov edx,IO_ADDR
                add edx,2
                mov CTRL_ADDR,edx
                inc edx
                mov CTRL_DATA,edx

                invoke enable_ctrl

                mov eax,9
                invoke get_ctrl_reg,eax   ;get left volume
                mov esi,eax

                mov eax,10
                invoke get_ctrl_reg,eax   ;get right volume
                mov edi,eax

                xor esi,0101b             ;tweak a few bits
                xor edi,1010b

                mov eax,9                 ;write the tweaked values back
                invoke set_ctrl_reg,eax,esi

                mov eax,10
                invoke set_ctrl_reg,eax,edi

                mov eax,9
                invoke get_ctrl_reg,eax   ;see if changes took effect
                cmp eax,esi
                mov eax,0                 ;(return failure)
                jne __exit

                mov eax,10
                invoke get_ctrl_reg,eax
                cmp eax,edi
                mov eax,0                 ;(return failure)
                jne __exit

                xor esi,0101b             ;control chip found: restore old
                xor edi,1010b             ;values & re-enable FM sound

                mov eax,9
                invoke set_ctrl_reg,eax,esi

                mov eax,10
                invoke set_ctrl_reg,eax,edi

                invoke disable_ctrl
                mov eax,1                 ;return success
                jmp __exit

                ELSEIFDEF PAS

                mov eax,0bc00h            ;PAS, look for MVSOUND.SYS driver
                mov ebx,03f3fh
                invoke INT2F,eax,ebx      ;call DOS MPX interrupt through DPMI
                xor ebx,ecx
                xor ebx,edx
                cmp bx,'MV'               ;MediaVision flag
                mov eax,0
                jne __exit
                mov eax,1
                jmp __exit

                ELSEIFDEF SBLASTER        ;SB / SBPRO, detect SB DSP chip

                mov eax,IO_ADDR
                IFDEF SBPRO
                add eax,4
                mov MIXADDR,eax
                add eax,1
                mov MIXDATA,eax
                add eax,1
                ELSE
                add eax,6
                ENDIF

                mov DSP_RESET,eax
                add eax,4
                mov DSP_READ,eax
                add eax,2
                mov DSP_WRITE_STAT,eax
                add eax,2
                mov DSP_DATA_RDY,eax
                mov eax,IRQ
                mov DSP_IRQ,eax
                mov eax,DMA
                mov DSP_DMA,eax

                mov eax,DSP_IRQ           ;index interrupt vector for IRQ
                cmp eax,8
                jb __calc_vect
                add eax,60h               ;index slave PIC vectors if IRQ > 7
__calc_vect:    add eax,8
                mov test_vect,eax

                GET_VECT
                and edx,0ffffh
                mov old_S,edx
                mov old_O,ebx
                mov old_real,ecx

                mov eax,0d3h
                invoke send_timeout,eax   ;turn speaker off and let the new 
                invoke sysex_wait,16      ;setting take effect

                invoke reset_DSP              
                or eax,eax
                jz __exit                 ;(reset failed)

                IFDEF SBPRO               ;look for CT-1345A mixer chip
                IF DETECT_SBPRO
                mov edx,MIXADDR
                mov eax,0ah               ;select Mic Vol control
                out dx,ax
                jmp $+2
                mov edx,MIXDATA
                in al,dx                  ;get original value
                jmp $+2
                mov ah,al                 ;save it
                xor eax,110b              ;toggle its bits
                out dx,al                 ;write it back
                jmp $+2
                in al,dx                  ;read/verify changed value
                xor eax,110b              
                cmp al,ah
                mov al,ah                 ;put the old value back
                out dx,al
                mov eax,0
                jne __exit 
                ENDIF
                ENDIF

                pushfd                    ;prepare for IRQ/DMA test....
                sti                       

                mov IRQ_confirm,0         ;output a few bytes of data to
                mov packing,0             ;trigger EOD IRQ on selected line
                mov pack_byte,0
                mov stereo_flag,0
                mov xblk_status,0
                mov esi,1                 ;assume success
                mov eax,166
                mov ebx,0
                invoke set_sample_rate,eax,ebx

                mov ecx,DSP_IRQ           ;enable hardware interrupts,
                mov ebx,1                 ;saving previous PIC masks
                shl ebx,cl
                not ebx
                in al,0a1h
                mov PIC1_cur,al
                and al,bh
                out 0a1h,al
                in al,21h
                mov PIC0_cur,al
                and al,bl
                out 21h,al

                mov eax,test_vect
                mov edx,OFFSET IRQ_test   ;install test IRQ handler
                mov bx,cs
                SET_VECT

                invoke DMA_transfer,0,4
                mov edi,34                ;wait typ. 500 milliseconds
__poll_confirm: invoke sysex_wait,1
                cmp IRQ_confirm,1         ;EOD interrupt occurred?
                je __end_IRQ_test         ;yes, device IRQ valid
                dec edi
                jnz __poll_confirm
                mov esi,0                 ;IRQ handler never called -- failed

__end_IRQ_test: mov ecx,DSP_IRQ           ;stop hardware interrupts from DSP
                mov ebx,1
                shl ebx,cl
                in al,0a1h
                or al,bh
                and al,PIC1_cur           ;don't kill any interrupts that were
                out 0a1h,al               ;initially active
                in al,21h
                or al,bl
                and al,PIC0_cur
                out 21h,al

                push esi
                mov eax,test_vect
                mov ebx,old_real
                SET_REAL_VECT

                mov eax,test_vect
                mov edx,old_O
                mov ebx,old_S
                SET_PROT_VECT
                pop esi

                POP_F
                mov eax,esi

                ENDIF                     ;ELSEIFDEF SBLASTER

__exit:         IFDEF SBLASTER
                push eax
                or eax,eax
                jnz __success
                mov eax,0d1h              ;re-enable speaker in case SB
                invoke send_timeout,eax   ;card was present
__success:      pop eax
                ENDIF

                IFDEF ADLIBG
                pop CTRL_DATA
                pop CTRL_ADDR
                ENDIF

                IFDEF SBPRO
                pop MIXADDR
                pop MIXDATA
                ENDIF

                IFDEF SBLASTER
                pop DSP_DMA
                pop DSP_IRQ
                pop DSP_DATA_RDY
                pop DSP_WRITE_STAT
                pop DSP_READ
                pop DSP_RESET
                ENDIF

                POP_F                     ;return AX=0 if not found
                ret
detect_device   ENDP

;****************************************************************************
init_driver     PROC USES ebx esi edi,\
                H,IO_ADDR,IRQ,DMA,DRQ

                pushfd
                cli

                mov local_DS,ds

                mov eax,IO_ADDR

                IFDEF SBPRO
                add eax,4
                mov MIXADDR,eax
                add eax,1
                mov MIXDATA,eax
                add eax,1
                ELSEIFDEF SBSTD
                add eax,6
                ENDIF

                IFDEF SBLASTER

                mov DSP_RESET,eax
                add eax,4
                mov DSP_READ,eax
                add eax,2
                mov DSP_WRITE_STAT,eax
                add eax,2
                mov DSP_DATA_RDY,eax

                mov eax,IRQ
                mov DSP_IRQ,eax
                mov eax,DMA
                mov DSP_DMA,eax

                ELSEIFDEF ADLIBG

                mov eax,IO_ADDR
                add eax,2
                mov CTRL_ADDR,eax         ;set I/O parms for control chip
                inc eax
                mov CTRL_DATA,eax
                inc eax
                mov DSP_ADDR,eax          ;set I/O parms for sampling channels
                inc eax
                mov DSP_DATA,eax

                ENDIF                     ;IFDEF SBLASTER

                mov spkr_status,-1
                mov xblk_status,0
                mov silence_flag,0
                mov old_freq,-1
                mov old_stereo,-1
                mov playing,0

                invoke detect_device,0,IO_ADDR,IRQ,DMA,DRQ
                or eax,eax
                jz __exit_init

                IFDEF PAS

                mov eax,0bc04h          ;get current DMA and IRQ settings
                invoke INT2F,eax,eax
                and ebx,0ffh
                and ecx,0ffh
                mov DSP_DMA,ebx
                mov DSP_IRQ,ecx

                mov eax,0bc07h          ;get state table entries
                invoke INT2F,eax,eax

                mov eax,0               ;update local state table
                mov al,bh
                mov MV_xchannel,eax
                mov al,ch
                mov MV_filter,eax

                mov eax,MV_xchannel     ;secure the DMA channel
                mov edx,CROSSCHANNEL
                or eax,10000000b       
                out dx,al
                mov MV_xchannel,eax

                mov edx,INTRCTLRST      ;enable IRQs on sample buffer empty
                out dx,al
                jmp $+2
                in al,dx
                mov edx,INTRCTLR
                in al,dx
                or eax,00001000b         
                out dx,al

                ELSEIFDEF ADLIBG

                invoke enable_ctrl

                mov eax,11h
                invoke get_ctrl_reg,eax  ;(Audio Selection)

                and eax,11111100b        ;set filters to playback mode

                mov ebx,11h
                invoke set_ctrl_reg,ebx,eax

                mov eax,13h
                invoke get_ctrl_reg,eax  ;(Audio IRQ/DMA Select - Channel 0)
                mov mask_save,eax

                mov esi,eax
                or eax,10001000b         ;(DEN0 | AEN)

                mov ebx,13h
                invoke set_ctrl_reg,ebx,eax
                invoke disable_ctrl

                mov eax,esi
                and eax,01110000b        ;isolate DMA SEL 0 bits
                mov ecx,4
                shr eax,cl
                and eax,0ffh
                mov DSP_DMA,eax          ;record DMA channel in use

                and esi,00000111b        ;isolate INT SEL A bits
                mov al,selected_IRQ[esi]
                and eax,0ffh
                mov DSP_IRQ,eax          ;record IRQ line in use

                invoke reset_DSP

                ENDIF

                mov eax,default_pan
                mov panpot_val,eax
                mov eax,default_vol
                mov main_volume,eax
                invoke set_volume

                mov loop_cnt,0
                mov DAC_status,DAC_STOPPED
                mov buffer_mode,BUF_MODE

                mov buff_status[0*4],DAC_DONE
                mov buff_status[1*4],DAC_DONE

                mov eax,DSP_IRQ         ;index interrupt vector for IRQ
                cmp eax,8
                jb __calc_vect
                add eax,60h             ;index slave PIC vectors if IRQ > 7
__calc_vect:    add eax,8
                mov int_vector,eax

                GET_VECT
                and edx,0ffffh
                mov old_IRQ_s,edx
                mov old_IRQ_o,ebx
                mov old_IRQ_real,ecx

                mov ecx,DSP_IRQ         ;enable hardware interrupts from DSP,
                mov ebx,1               ;saving previous PIC masks
                shl ebx,cl
                not ebx
                in al,0a1h
                mov PIC1_val,al
                and al,bh
                out 0a1h,al
                in al,21h
                mov PIC0_val,al
                and al,bl
                out 21h,al

                mov init_OK,1

                mov eax,1
__exit_init:    POP_F
                ret
init_driver     ENDP

;****************************************************************************
index_VOC_blk   PROC USES ebx esi edi es,\
                H,VOCFile:FAR PTR,VOCSeg,Block,SBuf
                LOCAL index_off
                LOCAL index_seg
                LOCAL x_status
                LOCAL x_pack:BYTE
                LOCAL x_tc:BYTE

                ASSUME edi:PTR sbuffer

                pushfd
                cli
                cld

                mov x_status,0

                les esi,[VOCFile]
                movzx eax,WORD PTR es:[esi+14h]

                add esi,eax
                mov index_off,esi       ;get selector pointer to data block

                mov esi,[VOCSeg]        ;get segment pointer to data block
                add esi,eax
                mov index_seg,esi

                mov ebx,[Block]

__get_type:     mov esi,index_off
                mov eax,0
                mov al,es:[esi]         ;get block type
                cmp eax,0               ;terminator block?
                je __exit               ;yes, return AX=0 (block not found)

                cmp eax,8
                jne __chk_voice

                mov al,es:[esi+5]       ;get extended voice parameters
                mov x_tc,al             ;high byte of TC = normal sample rate
                mov ax,es:[esi+6]       ;get pack (AL) and mode (AH)
                cmp ah,1                ;stereo?
                jne __set_pack
                or al,80h               ;yes, make pack byte negative
__set_pack:     mov x_pack,al
                mov x_status,1          ;flag extended block override
                jmp __next_blk

__chk_voice:    cmp eax,1               ;voice data block?
                jne __chk_marker        ;no

                cmp ebx,-1              ;marker found (or disregarded)?
                je __vblk_found         ;yes, use this voice data block
                jmp __next_blk          ;no, keep looking

__chk_marker:   cmp eax,4               ;marker block?
                jne __next_blk          ;no, keep looking

                cmp bx,es:[esi+4]       ;yes, compare marker numbers
                jne __next_blk

                mov ebx,-1              ;marker found, use next voice block

__next_blk:     mov esi,index_off
                mov eax,es:[esi+1]                   
                and eax,0ffffffh        ;get len
                add eax,4               ;determine offset to next block
                add esi,eax             ;skip current block

                mov index_off,esi       ;set selector pointer to next block

                mov si,WORD PTR index_seg
                mov di,WORD PTR index_seg+2
                shld edx,eax,16
                ADD_PTR ax,dx,di,si     ;set segment pointer to next block
                mov WORD PTR index_seg,si
                mov WORD PTR index_seg+2,di
                jmp __get_type

__vblk_found:   mov edi,[SBuf]          ;get pointer to output structure
                mov esi,index_off       ;get pointer to voice block

                mov eax,0
                mov ebx,0
                mov bl,es:[esi+4]       ;copy sampling rate
                mov al,es:[esi+5]       ;copy packing type

                cmp x_status,0          ;previous extended block overrides
                je __use_vd             ;data block values
                mov al,x_pack
                mov bl,x_tc
                mov x_status,0

__use_vd:       mov [edi].sample_rate,ebx
                mov [edi].pack_type,eax

                mov esi,index_off
                mov eax,es:[esi+1]
                and eax,0ffffffh        ;get data len

                sub eax,2               ;EAX = voice len
                mov [edi].len,eax       ;copy voice data length

                mov esi,index_off
                add esi,6               ;copy selector pointer to voice data
                mov [edi].ptr_data,esi
                mov [edi].sel_data,es

                mov ax,WORD PTR index_seg
                mov dx,WORD PTR index_seg+2
                ADD_PTR 6,0,dx,ax       ;copy segment pointer to voice data
                mov WORD PTR [edi].seg_data,ax
                mov WORD PTR [edi].seg_data+2,dx

                mov eax,1

__exit:         POP_F
                ret
index_VOC_blk   ENDP

;****************************************************************************
register_sb     PROC USES ebx esi edi,\
                H,BufNum,SBuf

                pushfd
                cli

                cmp buffer_mode,VOC_MODE        
                jne __get_bufnum          ;not in VOC mode, proceed
                invoke stop_d_pb          ;else stop VOC file output first
                mov buffer_mode,BUF_MODE

__get_bufnum:   mov edi,[BufNum]          ;get buffer #0-1
                shl di,2

                ASSUME esi:PTR sbuffer

                mov esi,[SBuf]            ;copy structure data to buffer 
                mov eax,[esi].pack_type   ;descriptor fields
                mov buff_pack[edi],eax

                mov eax,[esi].sample_rate
                mov buff_sample[edi],eax

                mov eax,[esi].seg_data
                mov buff_seg_data[edi],eax

                mov eax,[esi].len
                mov buff_len[edi],eax

                mov buff_status[edi],DAC_STOPPED

__exit:         POP_F                  
                ret
register_sb     ENDP

;****************************************************************************
get_sb_status   PROC USES ebx esi edi,\
                H,HBuffer

                pushfd
                cli

                mov ebx,[HBuffer]
                mov eax,buff_status[ebx*4]

                POP_F
                ret
get_sb_status   ENDP

;****************************************************************************
play_VOC_file   PROC USES ebx esi edi es,\
                H,VOCFile:FAR PTR,VOCSeg,Block

                pushfd
                cli
                cld

                mov xblk_status,0

                les esi,[VOCFile]
                movzx eax,WORD PTR es:[esi+14h]

                add esi,eax
                mov block_off,esi       ;get selector pointer to data block
                mov block_sel,es

                mov si,WORD PTR [VOCSeg]
                mov di,WORD PTR [VOCSeg]+2

                ADD_PTR ax,0,di,si      ;DI:SI -> physical start-of-data
                mov WORD PTR block_seg_ptr,si
                mov WORD PTR block_seg_ptr+2,di

                invoke stop_d_pb        ;assert VOC mode
                mov buffer_mode,VOC_MODE
                mov DAC_status,DAC_DONE 

                les esi,[VOCFile]       ;get selector pointer to VOC file

                cmp [Block],-1          ;play 1st block if no marker specified
                je __do_it

__find_blk:     invoke block_type 
                cmp eax,0               ;terminator block?
                je __exit               ;yes, exit (block not found)
                invoke set_xblk
                invoke marker_num       ;get marker # (or -1 if non-marker)
                mov esi,eax
                invoke next_block
                cmp esi,[Block]
                jne __find_blk

__do_it:        mov DAC_status,DAC_STOPPED         

__exit:         POP_F
                ret
play_VOC_file   ENDP

;****************************************************************************
format_VOC_file PROC USES ebx esi edi es,\
                H,VOCFile:FAR PTR,Block
                LOCAL pack              ;leave interrupts enabled; this might
                                        ;take awhile
                IFDEF NEEDS_FORMAT

                mov pack,-1

                les esi,[VOCFile]
                movzx eax,WORD PTR es:[esi+14h]

                add esi,eax
                mov block_off,esi       ;get selector pointer to data block
                mov block_sel,es

                cmp [Block],-1          ;format 1st blk if no marker specified
                je __preform_blk

__form_find:    invoke block_type 
                cmp eax,0               ;terminator block?
                je __exit               ;yes, exit (block not found)
                invoke marker_num       ;get marker # (or -1 if non-marker)
                mov esi,eax
                invoke next_block
                cmp esi,[Block]
                jne __form_find
           
__preform_blk:  invoke block_type
                cmp eax,0
                je __exit
                cmp eax,1
                jne __not_vdata

                les esi,block_far_ptr
                mov al,BYTE PTR es:[esi+5]
                and eax,0fh
                mov pack,eax

                mov eax,DWORD PTR es:[esi+1]
                and eax,0ffffffh
                sub eax,2               ;EAX = voice data len = BLKLEN - 2
                add esi,6               ;ESI -> voice data
                jmp __preform

__not_vdata:    cmp ax,2
                jne __preform_next

                les esi,block_far_ptr
                mov eax,DWORD PTR es:[esi+1]
                and eax,0ffffffh        ;EAX = voice data len = BLKLEN
                add esi,4               ;ESI -> voice data

__preform:      cmp pack,0              ;preformat only 8-bit PCM data
                jne __preform_next

                mov ecx,0
                mov cx,es
                invoke format_block,esi,ecx,eax

__preform_next: call next_block
                jmp __preform_blk

__exit:
                ENDIF
                ret
format_VOC_file ENDP

;****************************************************************************
format_sb       PROC USES ebx esi edi,\
                H,SBuf

                IFDEF NEEDS_FORMAT

                mov esi,[SBuf]           

                mov eax,[esi].pack_type
                and eax,0fh
                jnz __exit              ;format only 8-bit PCM data

                mov ebx,[esi].ptr_data
                movzx ecx,[esi].sel_data
                mov eax,[esi].len

                invoke format_block,ebx,ecx,eax
__exit:
                ENDIF

                ret
format_sb       ENDP

;****************************************************************************
start_d_pb      PROC USES ebx esi edi

                pushfd
                cli

                cmp buffer_mode,VOC_MODE
                je __voc_mode           ;start Creative Voice File playback

                cmp DAC_status,DAC_PLAYING
                je __exit               ;bail out if already playing

                invoke next_buffer      ;start dual-buffer playback                
                cmp eax,-1
                je __exit               ;no buffers registered, exit
                mov DAC_status,DAC_PLAYING

                mov old_freq,-1
                mov old_stereo,-1

                push eax
                mov eax,int_vector
                mov edx,OFFSET IRQ_play_buffer
                mov bx,cs
                SET_VECT
                pop eax

                invoke process_buffer,eax
                jmp __exit

__voc_mode:     cmp DAC_status,DAC_STOPPED
                jne __exit
                mov DAC_status,DAC_PLAYING
                
                mov old_freq,-1
                mov old_stereo,-1

                mov eax,int_vector
                mov edx,OFFSET IRQ_play_VOC
                mov bx,cs
                SET_VECT

                invoke process_block

__exit:         POP_F
                ret
start_d_pb      ENDP

;****************************************************************************
stop_d_pb       PROC USES ebx esi edi

                pushfd
                cli

                mov DAC_status,DAC_STOPPED

                invoke halt_DMA

                mov eax,DSP_DMA                 ;mask DMA channel
                or eax,4h                        
                out 0ah,al   

                mov buff_status[0*4],DAC_DONE
                mov buff_status[1*4],DAC_DONE

                POP_F
                ret
stop_d_pb       ENDP

;****************************************************************************
pause_d_pb      PROC USES ebx esi edi

                pushfd
                cli

                cmp DAC_status,DAC_PLAYING
                jne __exit              ;(not playing)
                mov DAC_status,DAC_PAUSED

                IFDEF PAS
                mov edx,INTRCTLRST      ;disable IRQs on sample buffer empty
                out dx,al
                jmp $+2
                in al,dx
                mov edx,INTRCTLR
                in al,dx
                and eax,11110111b        
                out dx,al
                ENDIF

                invoke halt_DMA

__exit:         POP_F
                ret
pause_d_pb      ENDP

;****************************************************************************
cont_d_pb       PROC USES ebx esi edi

                pushfd
                cli

                cmp DAC_status,DAC_PAUSED
                jne __exit              ;(not paused)
                mov DAC_status,DAC_PLAYING

                IFDEF PAS
                mov edx,INTRCTLRST      ;enable IRQs on sample buffer empty
                out dx,al
                jmp $+2
                in al,dx
                mov edx,INTRCTLR
                in al,dx
                or eax,00001000b         
                out dx,al
                ENDIF

                invoke continue_DMA

__exit:         POP_F
                ret
cont_d_pb       ENDP

;****************************************************************************
get_VOC_status  PROC USES ebx esi edi

                pushfd
                cli

                mov eax,DAC_status

                POP_F
                ret
get_VOC_status  ENDP

;****************************************************************************
                END
