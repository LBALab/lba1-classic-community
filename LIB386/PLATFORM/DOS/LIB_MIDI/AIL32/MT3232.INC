;ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
;лл                                                                         лл
;лл  MT3232.INC                                                             лл
;лл                                                                         лл
;лл  IBM Audio Interface Library -- Roland MT-32-series MIDI interpreter    лл
;лл                                                                         лл
;лл  Version 1.00 of 28-Jul-92: 32-bit conversion by John Lemberger         лл
;лл                             (Rational Systems DOS/4G)                   лл
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

send_sysex_msg  PROTO C,H:DWORD,AddrA:DWORD,AddrB:DWORD,AddrC:DWORD\
                       ,Data:PTR,C_Size:DWORD,C_Wait:DWORD
setup_patch     PROTO C,Patch:DWORD,Bank:DWORD
index_timbre    PROTO C,GNum:DWORD      ;Get global timbre's local index 0-63

                ;
                ;Driver-specific configuration equates
                ;

MAX_REC_CHAN    equ 10                  ;Max channel recognized by synths
MAX_TRUE_CHAN   equ 9                   ;Max channel available for locking
MIN_TRUE_CHAN   equ 2                   ;Min channel # (1-based)

SYSEX_SIZE      equ 32                  ;Controller sysex queue size in bytes
SYSEX_Q_CNT     equ 3                   ;# of sysex controller queues
DEF_SYNTH_VOL   equ 90                  ;init vol=90% (avoid MT-32 distortion)
CLEAR_SYNTH     equ TRUE                ;TRUE to reset Roland on init/shutdown
ADJUST_PART_RSV equ TRUE                ;TRUE to set AIL partial reserve

                ;
                ;Driver Description Table (DDT)
                ;Returned by describe_driver() proc
                ;

DDT             LABEL WORD
min_API_version dd 200                  ;Minimum API version required = 2.00
drvr_type       dd 3                    ;Type 3: XMIDI driver
data_suffix     db 'MT',0,0             ;Native data/instrument file suffix
device_name_o   dd OFFSET devnames      ;Pointer to list of supported devices
default_IO      LABEL WORD
                IFDEF MPU401
                dd 330h                 ;Factory default I/O parameters
                ELSEIFDEF SBMIDI
                dd 220h
                ENDIF
default_IRQ     dd -1
default_DMA     dd -1
default_DRQ     dd -1
service_rate    dd QUANT_RATE           ;Typically 120 calls/second 
display_size    dd 20                   ;20 char LCD display (MT-32)

devnames        db 'Roland MT-32 or compatible with Roland MPU MIDI Interface',0
                db 'Roland LAPC-1',0
                db 0                    ;0 to end list of device names

                ;
                ;Default setup values & constants
                ;

init_reverb     db 0,3,2                ;reverb mode = 0, time = 3, level = 2
part_chans      db 1,2,3,4,5,6,7,8,9    ;default (0-based) chans for each part
part_rsv        db 3,4,3,4,3,4,3,4,4    ;# of reserved AIL partials/channel

NUM_TIMBS       equ 64                  ;# of RAM timbres

SYSEX_BLK_SIZE  = (FINAL_BYTE_1 - START_MSB_1) + 1
SYSEX_RANGE_BEG = START_MSB_1
SYSEX_RANGE_END = SYSEX_RANGE_BEG + (SYSEX_BLK_SIZE * SYSEX_Q_CNT) - 1

                ;
                ;Misc. data
                ;

patch_bank      db 128 dup (?)

chksum          dw ?
string_buf      db 32 dup (?)

sysex_queues    db SYSEX_SIZE*SYSEX_Q_CNT dup (?)

M_ad            db SYSEX_Q_CNT dup (?)
K_ad            db SYSEX_Q_CNT dup (?)
L_ad            db SYSEX_Q_CNT dup (?)
queue_ptrs      db SYSEX_Q_CNT dup (?)

note_event      dd ?                            ;used for LRU counting
timb_hist       dd NUM_TIMBS dup (?)            ;last note event count for LRU
timb_bank       db NUM_TIMBS dup (?)            ;GTR bank
timb_num        db NUM_TIMBS dup (?)            ;GTR #
timb_attribs    db NUM_TIMBS dup (?)            ;bit 7=in use 6=protected

chan_timbs      db NUM_CHANS dup (?)            ;indexes used by channels 1-16
                                                ;(-1 if internal/uninit'd)

MIDI_bank       db NUM_CHANS dup (?)            ;Patch Bank Select values
MIDI_program    db NUM_CHANS dup (?)            ;program change # / channel

temp_buf        dd ?                            ;Temp for short sysex's

                IFDEF MPU401
                INCLUDE mpu40132.inc    ;Roland MPU-401-compatible interface
                ENDIF

                IFDEF SBMIDI
                INCLUDE sbmidi32.inc    ;Sound Blaster MIDI port
                ENDIF

;****************************************************************************
;*                                                                          *
;*  MIDI interpreter and related procedures                                 *
;*                                                                          *
;****************************************************************************

reset_synth     PROC\
                USES ebx esi edi
                pushfd                
                cli

                IF CLEAR_SYNTH          ;reset all internal parameters/patches
                invoke send_sysex_msg,0,7fh,00h,00h,OFFSET part_chans,1,12
                ENDIF
                                        ;init part/channel assignments
                invoke send_sysex_msg,0,10h,00h,0dh,OFFSET part_chans,9,4

                POP_F
                ret
reset_synth     ENDP

;****************************************************************************
init_synth      PROC\
                USES ebx esi edi
                pushfd           
                cli
                                        ;send adjusted partial reserve table
                IF ADJUST_PART_RSV      ;to avoid dropout on high channel #s
                invoke send_sysex_msg,0,10h,00h,04h,OFFSET part_rsv,9,4
                ENDIF                   

                invoke send_sysex_msg,0,10h,00h,01h,OFFSET init_reverb,3,4

                mov ebx,0
__init_sysex:   mov queue_ptrs[ebx],0
                inc ebx
                cmp ebx,SYSEX_Q_CNT
                jne __init_sysex

                mov note_event,0

                mov edi,0
__init_tcache:  mov timb_attribs[edi],00000000b
                inc edi
                cmp edi,NUM_TIMBS
                jne __init_tcache

                mov edi,0
__init_tchans:  mov chan_timbs[edi],-1
                mov MIDI_program[edi],-1
                mov MIDI_bank[edi],0
                inc edi
                cmp edi,NUM_CHANS
                jne __init_tchans

                mov edi,0
__init_patches: mov patch_bank[edi],0
                inc edi
                cmp edi,128
                jne __init_patches

                POP_F
                ret
init_synth      ENDP

;****************************************************************************
add_sysex_addr  PROC\
                USES ebx esi edi\                  ;Add value to 21-bit address
                ,Addend,MSB,KSB,LSB

                mov eax,0
                mov ebx,0
                mov ecx,0
                mov al,BYTE PTR [LSB]
                mov bl,BYTE PTR [KSB]
                mov cl,BYTE PTR [MSB]

                add eax,[Addend]

__chk_l:        cmp eax,80h
                jb __chk_k
                sub eax,80h
                inc ebx
                jmp __chk_l

__chk_k:        cmp ebx,80h
                jb __store_sum
                sub ebx,80h
                inc ecx
                jmp __chk_k

__store_sum:    mov ah,bl
                mov edx,0
                mov dl,cl
                ret     
add_sysex_addr  ENDP

;****************************************************************************
send_MIDI_sysex PROC\
                USES ebx esi edi\           ;Send MIDI System Exclusive message
                ,Src:PTR,C_Type,Len

                cld

                cmp [C_Type],0f0h       ;F0 type?
                jne __get_msg_len       ;no, don't send explicit F0

                invoke send_byte,0f0h

__get_msg_len:  mov esi,[Src]
                mov ecx,[Len]           ;get length
                jcxz __exit

__send_bytes:   push ecx
                lodsb
                invoke send_byte,eax
                pop ecx
                loop __send_bytes

__exit:         ret
send_MIDI_sysex ENDP

;****************************************************************************
sysex_wait      PROC USES ebx esi edi,\     ;Generate a 14-millisecond (typ.)
                Delay                   ;delay with interrupts enabled
                                        
                pushfd                  ;Requires CGA/EGA/VGA/XGA video)
                sti

                mov bx,63h
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
__exit:         
                POP_F
                ret
sysex_wait      ENDP

;****************************************************************************
write_system    PROC\
                USES ebx esi edi\           ;Write value to System area
                ,Index,Value            ;(Warning: No delay!)

                mov ebx,[Value]
                mov temp_buf,ebx
                mov ebx,OFFSET temp_buf
                mov eax,[Index]
                invoke send_sysex_msg,0,10h,0,eax,ebx,1,0

                ret
write_system    ENDP

;****************************************************************************
write_rhythm_setup PROC\
                USES ebx esi edi\	 ;Write value to Rhythm Setup area
             	  ,Keynum,C_Offset,Value

                mov eax,[Keynum]
                sub eax,24
                shl eax,2
                add eax,[C_Offset]
                invoke add_sysex_addr,eax,3,1,16
                
                mov cl,ah
                mov ebx,[Value]
                mov temp_buf,ebx
                mov ebx,OFFSET temp_buf
                invoke send_sysex_msg,0,edx,ecx,eax,ebx,1,4

                ret
write_rhythm_setup ENDP

;****************************************************************************
write_patch     PROC\
                USES ebx esi edi\          ;Write byte or word to Patch area
                ,Patch,Index,Value,C_Size

                mov eax,[Patch]
                shl eax,3
                add eax,[Index]
                invoke add_sysex_addr,eax,5,0,0

                mov cl,ah
                mov ebx,[Value]
                mov temp_buf,ebx
                mov ebx,OFFSET temp_buf
                invoke send_sysex_msg,0,edx,ecx,eax,ebx,[C_Size],2

__exit:         ret
write_patch     ENDP

;****************************************************************************
send_MIDI_message PROC\                 ;Send MIDI Channel Voice message
                USES ebx esi edi\
                Stat,D1,D2
                LOCAL op
                OPTION NOLJMP


                mov esi,[D1]
                and esi,0ffh            ;SI=data 1 / controller #
                mov edi,[Stat]
                mov eax,edi               
                and edi,00fh            ;DI=channel

                and eax,0f0h            ;AX=status
                cmp eax,0b0h             
                je __go_cc
                cmp eax,0c0h
                je __prg_change
                cmp eax,090h            ;Note On (or Note Off)?
                jne __send              

                add note_event,1        ;yes, update timbre cache LRU counters
                movzx ebx,chan_timbs[edi]
                cmp bl,-1
                je __send       
                shl ebx,1               ;Convert to word ptr
                mov eax,note_event
                mov timb_hist[ebx*2],eax

__send:         mov edi,[Stat]
                invoke send_byte,edi
                invoke send_byte,esi
                and edi,0f0h
                cmp edi,0c0h
                je __exit
                cmp edi,0d0h
                je __exit
                invoke send_byte,[D2]
__exit:         ret

                OPTION LJMP

__go_cc:        jmp __ctrl_change

__prg_change:   mov eax,esi             ;log current patch for channel
                mov MIDI_program[edi],al
                mov ah,MIDI_bank[edi]
                cmp ah,patch_bank[esi]  ;changing patch's timbre bank?
                je __set_index          ;no, just log index & send message

                push eax                ;else send sysex message to associate
                mov al,ah               ;patch with timbre
                invoke setup_patch,esi,eax
                pop ax

__set_index:    invoke index_timbre,eax
                mov chan_timbs[edi],al
                jmp __send

__ctrl_change:  cmp esi,SYSEX_RANGE_BEG
                jb __send
                cmp esi,SYSEX_RANGE_END
                jbe __sysex
                cmp esi,PATCH_REVERB
                je __p_rev
                cmp esi,PATCH_BENDER
                je __p_b_range
                cmp esi,REVERB_MODE
                je __rev_mode
                cmp esi,REVERB_TIME
                je __rev_time
                cmp esi,REVERB_LEVEL
                je __rev_lvl
                cmp esi,PATCH_BANK_SEL
                je __t_bank
                cmp esi,RHYTHM_KEY_TIMB
                je __r_t_sel
                cmp esi,TIMBRE_PROTECT
                je __t_prot
                
                cmp esi,CHAN_LOCK       ;(lowest XMIDI control #)
                jb __send               
                cmp esi,SEQ_INDEX       ;(highest XMIDI control #)
                ja __send               ;keep XMIDI controls out of 
                jmp __exit              ;MIDI data stream for speed

__t_prot:       movzx ebx,chan_timbs[edi]
                cmp bl,-1
                je __exit
                mov al,timb_attribs[ebx]
                and al,10111111b
                cmp [D2],64
                jl __tprot
                or al,01000000b
__tprot:        mov timb_attribs[ebx],al
                jmp __exit

__t_bank:       mov al,BYTE PTR [D2]
                mov MIDI_bank[edi],al
                jmp __exit

__p_rev:        movzx eax,MIDI_program[edi]
                cmp al,-1
                je __exit
                push eax
                invoke write_patch,eax,6,[D2],1
                or edi,0c0h
                invoke send_byte,edi
                pop eax
                invoke send_byte,eax
                jmp __exit

__p_b_range:    movzx eax,MIDI_program[edi]
                cmp al,-1
                je __exit
                push eax
                invoke write_patch,eax,4,[D2],1
                or edi,0c0h
                invoke send_byte,edi
                pop eax
                invoke send_byte,eax
                jmp __exit

__rev_mode:     
                invoke write_system,1,[D2]
                jmp __exit
__rev_time:     invoke write_system,2,[D2]
                jmp __exit
__rev_lvl:      invoke write_system,3,[D2]
                jmp __exit

__r_t_sel:      movzx eax,chan_timbs[edi]
                cmp al,-1
                je __exit
         	invoke write_rhythm_setup,[D2],0,eax
                jmp __exit

__sysex:        sub esi,SYSEX_RANGE_BEG
                mov eax,esi
                mov ecx,SYSEX_BLK_SIZE
                mov edx,0
                div ecx    
                mov op,edx
                mov ebx,eax
                mov al,BYTE PTR [D2]	;EBX=queue #, AL=data, EDX=operation
                cmp edx,0
                je __set_MSB
                cmp edx,1
                je __set_KSB
                cmp edx,2
                je __set_LSB

                mov eax,SYSEX_SIZE      ;EBX = queue #
                mul ebx
                add eax,OFFSET sysex_queues
                mov edi,eax             ;EDI = base queue address
                add al,queue_ptrs[ebx]
                adc ah,0
                mov esi,eax             ;ESI = pointer to last byte
                mov al,BYTE PTR [D2]
                mov [esi],al

                cmp op,3
                je __bump_ptr           ;inc ptr and exit if not final byte

__send_queue:   movzx eax,queue_ptrs[ebx]
                inc eax                 ;else send message
                mov esi,eax             ;ESI = len
                xchg edi,ebx            ;EDI = queue #, EBX = base address

                invoke send_sysex_msg,0,DWORD PTR M_ad[edi],\
                   DWORD PTR K_ad[edi],DWORD PTR L_ad[edi],ebx,esi,0

                cmp op,3                ;was this an overflow dump?
                jne __address_byte      

                invoke sysex_wait,4     ;yes, do an automatic wait of 4
                jmp __overflowed        ;VBL intervals

__address_byte: dec esi                 ;SI=len-1
__overflowed:   invoke add_sysex_addr,esi,DWORD PTR M_ad[edi],\
                   DWORD PTR K_ad[edi],DWORD PTR L_ad[edi]
               
                mov L_ad[edi],al        ;add (qlen-1) to sysex start address,
                mov K_ad[edi],ah        ;so future Final Byte controllers will
                mov M_ad[edi],dl        ;address same byte (unless overflowed)

                mov queue_ptrs[edi],0   ;flush queue
                jmp __exit

__bump_ptr:     cmp queue_ptrs[ebx],SYSEX_SIZE-1
                jae __send_queue	;dump queue if overflow occurs
                inc queue_ptrs[ebx]
                jmp __exit

__set_MSB:      mov M_ad[ebx],al
                jmp __exit
__set_KSB:      mov K_ad[ebx],al
                jmp __exit
__set_LSB:      mov L_ad[ebx],al
                jmp __exit

send_MIDI_message ENDP

;****************************************************************************
index_timbre    PROC\               ;Get global timbre's local index 0-63
                USES ebx esi edi\
                ,GNum

                mov esi,0
                movzx eax,WORD PTR [GNum]   
__find_gnum:    test timb_attribs[esi],10000000b
                jz __find_next          ;(timbre unused)
                cmp timb_bank[esi],ah
                jne __find_next
                cmp timb_num[esi],al
                je __found
__find_next:    inc esi
                cmp esi,NUM_TIMBS
                jb __find_gnum

                mov esi,-1              ;return -1 if timbre not loaded

__found:        mov eax,esi
                ret
index_timbre    ENDP

;****************************************************************************
setup_patch     PROC\
                USES ebx esi edi\           ;Establish patch's timbre setting
                ,Patch,Bank

                movzx ebx,BYTE PTR [Patch]
                mov eax,0
                mov ah,BYTE PTR [Bank]
                mov patch_bank[ebx],ah  ;log timbre bank # for patch

                cmp ah,0
                je __bank_0             ;bank 0 = built-in Roland timbres

                mov al,bl
                invoke index_timbre,eax ;see if timbre loaded
                cmp ax,-1
                je __bank_0             ;timbre not loaded, use default

                mov ah,al               ;select TIMBRE NUMBER 0-63
                mov al,2                ;select MEMORY (timbre group 2)
                jmp __send

__bank_0:       mov ah,BYTE PTR [Patch] ;restore default Roland timbre...
                mov al,0
                cmp ah,64               ;set GROUP A if patch # < 64
                sbb al,-1               ;set GROUP B if patch # >= 64
                and ah,63               ;set TIMBRE NUMBER 0-63
__send:         invoke write_patch,[Patch],0,eax,2

__exit:         ret                   
setup_patch     ENDP

;****************************************************************************
;*                                                                          *
;*  Public (API-accessible) procedures                                      *
;*                                                                          *
;****************************************************************************

describe_driver PROC\
                USES ebx esi edi\          ;Return ptr to DDT
                ,H,IntRateProc:PTR
                pushfd
                cli

                mov eax,OFFSET DDT

                POP_F
                ret
describe_driver ENDP

;****************************************************************************
send_sysex_msg  PROC\
                USES ebx esi edi\
                ,H,AddrA,AddrB,AddrC,Data:PTR,C_Size,C_Wait
                pushfd
                cli

                invoke send_byte,0f0h   ;transmit Roland MT-32 header
                invoke send_byte,041h
                invoke send_byte,010h
                invoke send_byte,016h
                invoke send_byte,012h

                invoke send_byte,[AddrA]
                invoke send_byte,[AddrB]
                invoke send_byte,[AddrC]

                mov eax,0
                mov al,BYTE PTR [AddrA] ;init checksum
                add al,BYTE PTR [AddrB]
                adc ah,0
                add al,BYTE PTR [AddrC]
                adc ah,0
                mov chksum,ax

                mov edi,[Data]          ;send each message byte
                mov esi,[C_Size]
__send_data:    or esi,esi
                jz __send_chksum
                dec esi
                mov eax,0
                mov al,[edi]
                inc edi
                add chksum,ax
                invoke send_byte,eax
                jmp __send_data

__send_chksum:  movzx eax,chksum           
                and eax,7fh
                sub al,80h
                neg al
                and eax,7fh
                invoke send_byte,eax    ;send the checksum byte

                invoke send_byte,0f7h   ;send EOX terminator

                invoke sysex_wait,[C_Wait]

                POP_F
                ret
send_sysex_msg  ENDP

;****************************************************************************
write_display   PROC\
                USES ebx esi edi es\
                ,H,String:PTR           ;Write string to display (unless NULL)
                pushfd
                cli

                push ds
                pop es

                cld
                mov esi,[String]        ;bail out if null string
                mov eax,esi
                cmp eax,0
                je __end_write

                mov edi,OFFSET string_buf      ;pad string with spaces
                mov ecx,display_size
                mov al,' '
                rep stosb

                mov edi,OFFSET string_buf      ;write up to /display_size/ characters
                mov ecx,display_size

__pad_string:   lodsb
                cmp al,0
                je __show_it
                stosb
                loop __pad_string

__show_it:      invoke send_sysex_msg,0,20h,00h,00h,\
                   OFFSET string_buf,display_size,4

__end_write:    POP_F
                ret
write_display   ENDP

;****************************************************************************
send_cv_msg     PROC\
                USES ebx esi edi\
                ,H,Stat,D1,D2       ;Send an explicit Channel Voice msg
                pushfd
                cli

                invoke send_MIDI_message,[Stat],[D1],[D2]

                POP_F
                ret
send_cv_msg     ENDP

;****************************************************************************
protect_timbre  PROC\
                USES ebx esi edi\          ;Protect a timbre from replacement
                ,H,Bank,Num
                pushfd
                cli

                mov eax,0
                mov al,BYTE PTR [Num]
                mov ah,BYTE PTR [Bank]

                cmp ax,-1
                je __prot_all

                invoke index_timbre,eax
                cmp eax,-1
                je __exit               ;timbre not loaded, can't protect it

                mov ebx,eax
                or timb_attribs[ebx],01000000b
                jmp __exit

__prot_all:     mov ebx,0
__prot_timb:    or timb_attribs[ebx],01000000b
                inc ebx
                cmp ebx,NUM_TIMBS
                jb __prot_timb

__exit:         POP_F
                ret
protect_timbre  ENDP

;****************************************************************************
unprotect_timbre PROC\
                USES ebx esi edi\          ;Allow a timbre to be replaced
                ,H,Bank,Num
                pushfd 
                cli

                mov eax,0
                mov al,BYTE PTR [Num]
                mov ah,BYTE PTR [Bank]

                cmp ax,-1
                je __unprot_all

                invoke index_timbre,eax
                cmp eax,-1
                je __exit               ;timbre not loaded, can't unprotect it

                mov ebx,eax
                and timb_attribs[ebx],10111111b
                jmp __exit

__unprot_all:   mov ebx,0
__unprot_timb:  and timb_attribs[ebx],10111111b
                inc ebx
                cmp ebx,NUM_TIMBS
                jb __unprot_timb

__exit:         POP_F
                ret
unprotect_timbre ENDP

;****************************************************************************
timbre_status   PROC\
                USES ebx esi edi\           ;Return 0 if timbre not resident
                ,H,Bank,Num
                pushfd 
                cli

                mov eax,0
                mov al,BYTE PTR [Num]
                mov ah,BYTE PTR [Bank]

                cmp ah,0                ;Roland driver reports all bank 0
                je __OK                 ;(default) and bank 127 (rhythm) 
                cmp ah,127              ;timbres present
                je __OK                 

                invoke index_timbre,eax

__OK:           inc eax                 ;0 if -1, else local cache index

                POP_F
                ret
timbre_status   ENDP

;****************************************************************************
get_cache_size  PROC\
                USES ebx esi edi\
                ,H
                pushfd
                cli

                mov eax,0               ;no resident cache for MT-32

                POP_F
                ret
get_cache_size  ENDP

;****************************************************************************
define_cache    PROC\
                USES ebx esi edi\
                ,H,Address:PTR,C_Size

                ret
define_cache    ENDP

;****************************************************************************
get_request     PROC\
                USES ebx esi edi\
                ,H,Sequence
                pushfd
                cli

                mov esi,[Sequence]
                cmp esi,-1
                je __no_request
                mov esi,sequence_state[esi]

                cmp [esi].state_table.TIMB,0
                je __no_request         ;no requested timbres, exit

                                        ;make sure TIMB chunk is present
                mov esi,[esi].state_table.TIMB   
                cmp [esi],DWORD PTR 'BMIT'
                jne __no_request        ;if not, no requests are possible

                add esi,8
                movzx edi,WORD PTR [esi];get TIMB.cnt
__chk_index:    add esi,2
                movzx eax,WORD PTR [esi]
                cmp ah,0                ;bank 0 reserved for Roland built-in
                je __next_index         ;timbres.... don't request
                cmp ah,127              ;bank 127 reserved for melodic-mode 
                je __next_index         ;rhythm sounds.... don't request
                invoke index_timbre,[esi]
                cmp eax,-1              ;timbre in local cache?
                je __request            ;no, request it
__next_index:   dec edi
                jne __chk_index
                jmp __no_request        ;all requested timbres loaded, exit

__request:      mov eax,[esi]           ;else return request: AL=num, AH=bank
                jmp __exit

__no_request:   mov eax,-1
                
__exit:         POP_F
                ret
get_request     ENDP

;****************************************************************************
install_timbre  PROC\
                USES ebx esi edi\
                ,H,Bank,Num,Address:PTR
                LOCAL sys_dest
                pushfd
                cli

                mov eax,0
                mov al,BYTE PTR [Num]
                mov ah,BYTE PTR [Bank]

                cmp ah,0                ;bank 0 reserved for built-in default
                je __set_patch          ;timbres -- restore normal patch
                cmp ah,127              ;bank 127 reserved for melodic mode
                je __exit               ;rhythm sounds -- don't install

                invoke index_timbre,eax
                cmp eax,-1
                jne __set_patch         ;timbre already resident, index it

                mov eax,[Address]
                cmp eax,0
                jz __exit

                mov esi,0               ;else look for an empty timbre slot
__find_free:    test timb_attribs[esi],10000000b
                jz __found
                inc esi
                cmp esi,NUM_TIMBS
                jb __find_free

                mov esi,0               ;no free timbre slots, replace least-
                mov ebx,0               ;recently-used unprotected timbre
                mov ecx,-1
                mov eax,-1
                mov edx,-1
__find_LRU:     test timb_attribs[esi],01000000b
                jnz __next_LRU          ;(timbre protected)
                cmp timb_hist[ebx],eax
                ja __next_LRU
__log_LRU:      mov eax,timb_hist[ebx]
                mov ecx,esi
__next_LRU:     add ebx,4
                inc esi
                cmp esi,NUM_TIMBS
                jb __find_LRU

                cmp ecx,-1              ;if no unprotected timbre slots, exit 
                je __exit               ;without installing new timbre

                mov esi,ecx             ;else SI = LRU timbre index

__found:        mov ebx,esi             ;found free/LRU timbre, update its
                shl ebx,1               ;timestamp and replace it
                mov eax,note_event      
                add note_event,1      
                mov timb_hist[ebx*2],eax  

                mov eax,0
                mov al,BYTE PTR [Num]   ;AL=num, AH=bank
                mov ah,BYTE PTR [Bank]
                mov timb_num[esi],al    ;record global # in slot
                mov timb_bank[esi],ah   ;mark timbre "in use/unprotected"
                mov timb_attribs[esi],10000000b                  

                cmp [Address],0
                je __exit               ;(NULL pointer = placeholder)

                mov edi,[Address]
                add edi,2               ;skip timbre length prefix (norm. $F6)

                mov eax,esi             ;set up to send timbre dump to Roland
                shl ax,1                ;8,AX,00 = MT-32 address MSB,KSB,LSB
                mov sys_dest,eax
                invoke send_sysex_msg,0,8,sys_dest,0,edi,0eh,3

                add edi,0eh             ;send partial parameter #1
                invoke send_sysex_msg,0,8,sys_dest,0eh,edi,3ah,3

                add edi,3ah             ;send partial parameter #2
                invoke send_sysex_msg,0,8,sys_dest,48h,edi,3ah,3

                add edi,3ah             ;send partial parameter #3
                inc sys_dest
                invoke send_sysex_msg,0,8,sys_dest,2,edi,3ah,3

                add edi,3ah             ;send partial parameter #4
                invoke send_sysex_msg,0,8,sys_dest,3ch,edi,3ah,3

__set_patch:    invoke setup_patch,[Num],[Bank]

__exit:         POP_F
                ret
install_timbre  ENDP
