;ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
;лл                                                                         лл
;лл  XMIDI32.ASM                                                            лл
;лл                                                                         лл
;лл  IBM Audio Interface Library -- Extended MIDI sound driver shell        лл
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

 	OPTION SCOPED		 ;Enable local labels
                .386                     ;Enable 386 instruction set
 	.MODEL FLAT,C            ;Flat memory model, C calls

release_channel PROTO C,H:DWORD,Chan:DWORD
lock_channel    PROTO C,H:DWORD
XMIDI_control   PROTO C,State:PTR,Chan:DWORD,Con:DWORD,Val:DWORD
release_seq     PROTO C,H:DWORD,Sequence:DWORD
stop_seq        PROTO C,H:DWORD,Sequence:DWORD

                ;
                ;Configuration equates
                ;

FALSE           equ 0
TRUE            equ -1

QUANT_RATE      equ 120                 ;Quantization (def=120 intervals/sec.)
QUANT_TIME      equ 8333                ;(uS/interval = 1000000/QUANT_RATE)

QUANT_TIME_16   equ 0208d5h             ;set to 16,000,000/QUANT_RATE --
                                        ;normally 133333 (208D5H)

MAX_NOTES       equ 32                  ;Max # of notes "on" simultaneously
FOR_NEST        equ 4                   ;# of FOR loop nesting levels
NSEQS           equ 8                   ;# of sequence handles available
QUANT_ADVANCE   equ 1                   ;Beat/bar counts += n intervals

DEF_PITCH_L     equ 00h
DEF_PITCH_H     equ 40h                 ;Default pitch bend = 4000h (mid-way)

BRANCH_EXIT     equ TRUE                ;TRUE to allow branches out of loops

                ;
                ;Macros, non-configurable equates
                ;

                INCLUDE 386.mac         ;DOS extender macros
                INCLUDE ail32.inc

                IFDEF SBSTD
YM3812          equ 1
                ENDIF

                IFDEF ADLIBSTD
YM3812          equ 1
                ENDIF

                IFDEF PAS
YM3812          equ 1
STEREO          equ 1
                ENDIF

                IFDEF SBPRO1
YM3812          equ 1
STEREO          equ 1
                ENDIF

                IFDEF SBPRO2
YMF262          equ 1
STEREO          equ 1
                ENDIF

                IFDEF PASOPL
YMF262          equ 1
STEREO          equ 1
                ENDIF

                IFDEF ADLIBG
YMF262	equ 1
STEREO          equ 1

                ENDIF

                IFDEF TANDY
SPKR            equ 1
                ENDIF

                IFDEF IBMPC
SPKR            equ 1
                ENDIF

NUM_CHANS       equ 16                  ;# of MIDI channels

                .CODE

                PUBLIC driver_start

driver_start    dd OFFSET driver_index
                db 'Copyright (C) 1991,1992 Miles Design, Inc.',01ah

driver_index    LABEL DWORD
                dd AIL_DESC_DRVR,OFFSET describe_driver
                dd AIL_DET_DEV,OFFSET detect_device
                dd AIL_INIT_DRVR,OFFSET init_driver
                dd AIL_SERVE_DRVR,OFFSET serve_driver
                dd AIL_SHUTDOWN_DRVR,OFFSET shutdown_driver

                dd AIL_STATE_TAB_SIZE,OFFSET get_state_size
                dd AIL_INSTALL_CB,OFFSET install_callback
                dd AIL_CANCEL_CB,OFFSET cancel_callback
                dd AIL_REG_SEQ,OFFSET register_seq
                dd AIL_REL_SEQ_HND,OFFSET release_seq

                dd AIL_START_SEQ,OFFSET start_seq
                dd AIL_STOP_SEQ,OFFSET stop_seq
                dd AIL_RESUME_SEQ,OFFSET resume_seq
                dd AIL_SEQ_STAT,OFFSET get_seq_status
                dd AIL_REL_VOL,OFFSET get_rel_volume
                dd AIL_SET_REL_VOL,OFFSET set_rel_volume
                dd AIL_REL_TEMPO,OFFSET get_rel_tempo
                dd AIL_SET_REL_TEMPO,OFFSET set_rel_tempo
                dd AIL_CON_VAL,OFFSET get_control_val
                dd AIL_SET_CON_VAL,OFFSET set_control_val
                dd AIL_CHAN_NOTES,OFFSET get_chan_notes
                dd AIL_MAP_SEQ_CHAN,OFFSET map_seq_channel
                dd AIL_TRUE_SEQ_CHAN,OFFSET true_seq_channel
                dd AIL_BEAT_CNT,OFFSET get_beat_count
                dd AIL_BAR_CNT,OFFSET get_bar_count
                dd AIL_BRA_INDEX,OFFSET branch_index

                dd AIL_SEND_CV_MSG,OFFSET send_cv_msg
                dd AIL_SEND_SYSEX_MSG,OFFSET send_sysex_msg
                dd AIL_WRITE_DISP,OFFSET write_display

                dd AIL_LOCK_CHAN,OFFSET lock_channel
                dd AIL_RELEASE_CHAN,OFFSET release_channel

                dd AIL_T_CACHE_SIZE,OFFSET get_cache_size
                dd AIL_DEFINE_T_CACHE,OFFSET define_cache
                dd AIL_T_REQ,OFFSET get_request
                dd AIL_INSTALL_T,OFFSET install_timbre
                dd AIL_PROTECT_T,OFFSET protect_timbre
                dd AIL_UNPROTECT_T,OFFSET unprotect_timbre
                dd AIL_T_STATUS,OFFSET timbre_status
                dd -1

                ;
                ;Synthesizer- and interface-specific routines
                ;

                IFDEF MT32
                INCLUDE mt3232.inc      ;Roland MT-32-compatible synthesizer
                ENDIF

                IFDEF YM3812
                INCLUDE yamaha32.inc    ;Standard Ad Lib-style chipset
                ENDIF

                IFDEF YMF262
                INCLUDE yamaha32.inc    ;YMF262 support for Ad Lib Gold et al
                ENDIF

                IFDEF MMASTER
                INCLUDE mmaster.inc     ;ASC MediaMaster and 100% compatibles
                ENDIF

                IFDEF SPKR              
                INCLUDE spkr32.inc      ;Internal speaker support for PC/Tandy
                ENDIF

                ;
                ;Misc. data
                ;

ctrl_log        STRUC                   ;XMIDI sequence/global controller log
PV              db NUM_CHANS dup (?)    
MODUL           db NUM_CHANS dup (?)    
PAN             db NUM_CHANS dup (?)    
EXP             db NUM_CHANS dup (?)    
SUS             db NUM_CHANS dup (?)    
PBS             db NUM_CHANS dup (?)
C_LOCK          db NUM_CHANS dup (?)    
C_PROT          db NUM_CHANS dup (?)    
V_PROT          db NUM_CHANS dup (?)    
ctrl_log        ENDS

logged_ctrls    LABEL BYTE              ;Controllers saved in state table
                db PART_VOLUME,MODULATION,PANPOT,EXPRESSION,SUSTAIN
                db PATCH_BANK_SEL
                db CHAN_LOCK,CHAN_PROTECT,VOICE_PROTECT
NUM_CONTROLS    equ ($-logged_ctrls)    ;room for 16 max. w/8-bit hash

ctrl_default    LABEL BYTE              ;default controller/program change
                db 127,0,64,127,0       ;values for startup initialization
                db 0
                db 0,0,0

prg_default     db 68,48,95,78          ;(Roland defaults)
                db 41,3,110,122,-1

ctrl_hash       db 256 dup (-1)         ;Controller offsets indexed for speed
                
state_table     STRUC                   ;XMIDI sequence state table layout
TIMB            dd ?
RBRN            dd ?
EVNT            dd ?
EVNT_ptr        dd ?
cur_callback    dd ?
ctrl_ptr        dd ?
seq_handle      dd ?
seq_started     dw ?
status          dw ?
post_release    dw ?
interval_cnt    dw ?
note_count      dw ?
vol_error       dd ?
vol_percent     dd ?
vol_target      dd ?
vol_accum       dd ?
vol_period      dd ?
tempo_error     dd ?
tempo_percent   dd ?
tempo_target    dd ?
tempo_accum     dd ?
tempo_period    dd ?
beat_count      dw ?
measure_count   dw ?
time_numerator  dw ?
time_fraction   dd ?
beat_fraction   dd ?
time_per_beat   dd ?
FOR_ptrs        dd FOR_NEST dup (?)
FOR_loop_cnt    dw FOR_NEST dup (?)
chan_map        db NUM_CHANS dup (?)
chan_program    db NUM_CHANS dup (?)
chan_pitch_l    db NUM_CHANS dup (?)
chan_pitch_h    db NUM_CHANS dup (?)
chan_indirect   db NUM_CHANS dup (?)
chan_controls   ctrl_log <>
note_chan       db MAX_NOTES dup (?)
note_num        db MAX_NOTES dup (?)
note_time       dd MAX_NOTES dup (?)
state_table     ENDS

sequence_state  dd NSEQS dup (?)
sequence_count  dd ?
current_handle  dd ?

service_active  dw ?
trigger_fn      dd ?

global_shadow   LABEL BYTE            
global_controls ctrl_log <>
global_program  db NUM_CHANS dup (?)
global_pitch_l  db NUM_CHANS dup (?)
global_pitch_h  db NUM_CHANS dup (?)
GLOBAL_SIZE     equ ($-global_shadow)

active_notes    db NUM_CHANS dup (?)    
lock_status     db NUM_CHANS dup (?)    ;bit 7: locked
                                        ;    6: lock-protected           
init_OK         dw 0

;****************************************************************************
;*                                                                          *
;*  XMIDI interpreter and related procedures                                *
;*                                                                          *
;****************************************************************************

find_seq        PROC C USES ebx esi edi\	        ;Find FORM SeqNum in IFF
                ,XMID:PTR,SeqNum
                LOCAL end_addr                  ;CAT/FORM XMID file

                mov ecx,[SeqNum]
                inc ecx                         ;look for CXth FORM XMID chunk

                mov esi,[XMID]
__find_XMID:    cmp DWORD PTR [esi],' TAC'
                je __found_IFF
__chk_FORM:     cmp DWORD PTR [esi],'MROF'      ;return failure if not an IFF
                jne __not_found                 ;image

__found_IFF:    cmp DWORD PTR [esi+8],'DIMX'
                je __found_XMID

__next_XMID:    mov ax,[esi+4]                   ;find first XMID chunk
                xchg al,ah
                shl eax,16
                mov ax,[esi+6]
                xchg al,ah
                add eax,8
                add eax,esi
                mov esi,eax
                jmp __find_XMID
                
__found_XMID:   mov ax,[esi+4]
                xchg al,ah
                shl eax,16
                mov ax,[esi+6]
                xchg al,ah
                sub eax,5                        
                mov end_addr,eax

                cmp DWORD PTR [esi],'MROF'      ;if outer header was a FORM,
                jne __scan_CAT                  ;return successfully if CX=1
                cmp ecx,1
                je __seq_found
                jmp __not_found           

__scan_CAT:     add esi,12                      ;index first FORM chunk

__check_FORM:   cmp DWORD PTR [esi+8],'DIMX'    ;is this a FORM XMID?
                je __next_seq                   ;yes, dec the loop counter...

__next_FORM:    mov ax,[esi+4]                  ;else add length of FORM + 8
                xchg al,ah
                shl eax,16
                mov ax,[esi+6]                  ;and keep looking...
                xchg al,ah
                add eax,8
                sub end_addr,eax                ;...unless EOF reached
                jl __not_found
                add eax,esi
                mov esi,eax
                jmp __check_FORM

__next_seq:     loop __next_FORM                ;look for CXth sequence chunk

__seq_found:    mov eax,esi                       
                jmp __exit                      ;return pointer to first chunk

__not_found:    mov eax,0                       ;return NULL if not found
__exit:         ret
find_seq        ENDP

;****************************************************************************
rewind_seq      PROC C USES ebx esi edi \   ;Reset sequence pointer and invalidate
                ,Sequence		;all state table entries
                                                     
                mov esi,[Sequence]
                mov esi,sequence_state[esi]

                mov ecx,FOR_NEST
                mov ebx,0
__init_FOR:     mov [esi].state_table.FOR_loop_cnt[ebx],-1
                add ebx,2
                loop __init_FOR

                mov ebx,NUM_CHANS-1
__init_chans:   mov [esi].state_table.chan_map[ebx],bl
                mov [esi].state_table.chan_program[ebx],-1
                mov [esi].state_table.chan_pitch_l[ebx],-1
                mov [esi].state_table.chan_pitch_h[ebx],-1
                mov [esi].state_table.chan_indirect[ebx],-1
                dec ebx
                jge __init_chans

                mov ebx,SIZE state_table.chan_controls-1
__init_ctrls:   mov BYTE PTR [esi].state_table.chan_controls[ebx],-1
                dec ebx
                jge __init_ctrls

                mov ebx,MAX_NOTES-1
__init_notes:   mov [esi].state_table.note_chan[ebx],-1
                dec ebx
                jge __init_notes

                mov [esi].state_table.cur_callback,-1
                mov [esi].state_table.interval_cnt,0
                mov [esi].state_table.note_count,0
                mov [esi].state_table.vol_percent,DEF_SYNTH_VOL
                mov [esi].state_table.vol_target,DEF_SYNTH_VOL
                mov [esi].state_table.tempo_percent,100
                mov [esi].state_table.tempo_target,100
                mov [esi].state_table.tempo_error,0

                mov [esi].state_table.beat_count,0
                mov [esi].state_table.measure_count,-1

                mov [esi].state_table.beat_fraction,0
                mov [esi].state_table.time_fraction,0

                mov [esi].state_table.time_numerator,4       ;default to 4/4 time

                mov [esi].state_table.time_per_beat,07a1200h ;default to 500000 us/beat*16
                                                             ;(120 beats/min)
                ret
rewind_seq      ENDP

;****************************************************************************
flush_channel_notes PROC C \ 	       ;Turn all sequences' notes off in a
                USES ebx esi edi \         ;given channel
                ,Chan
                LOCAL handle,seqcnt

                mov handle,0            ;for all sequences....

                mov ecx,sequence_count
                mov seqcnt,ecx
                jecxz __exit

__for_seq:      mov edi,handle
                add handle,4
                cmp sequence_state[edi],0
                je __for_seq            ;(sequence not registered)

                mov esi,sequence_state[edi]

                cmp [esi].state_table.note_count,0
                je __next_seq           ;no notes on, don't bother looking

                mov ebx,0               ;check note queue for active notes
__for_entry:    mov al,[esi].state_table.note_chan[ebx]
                cmp al,BYTE PTR [Chan]
                jne __next_entry
                mov [esi].state_table.note_chan[ebx],-1
                mov cl,[esi].state_table.note_num[ebx]
                mov edi,ebx
                mov bl,al
                mov bh,0                ;translate logical to physical channel
                mov bl,[esi].state_table.chan_map[ebx]
                dec active_notes[ebx]   ;dec # of active notes in channel
                or bl,80h               ;send MIDI Note Off message
                invoke send_MIDI_message,ebx,ecx,0
                dec [esi].state_table.note_count
                mov ebx,edi
__next_entry:   inc ebx
                cmp ebx,MAX_NOTES
                jb __for_entry

__next_seq:     dec seqcnt
                jne __for_seq

__exit:         ret
flush_channel_notes ENDP

;****************************************************************************
flush_note_queue PROC C \      		;Turn all queued notes off
                USES ebx esi edi \
                ,State:PTR

                mov esi,[State]

                mov ebx,0               ;check note queue for active notes
__for_entry:    mov al,[esi].state_table.note_chan[ebx]
                cmp al,-1
                je __next_entry
                mov [esi].state_table.note_chan[ebx],-1
                mov cl,[esi].state_table.note_num[ebx]
                mov edi,ebx
                mov ebx,0               ;translate logical to physical channel
                mov bl,al
                mov bl,[esi].state_table.chan_map[ebx]
                dec active_notes[ebx]   ;dec # of active notes in channel
                or bl,80h               ;send MIDI Note Off message
                invoke send_MIDI_message,ebx,ecx,0
                mov ebx,edi
__next_entry:   inc ebx
                cmp ebx,MAX_NOTES
                jb __for_entry

                mov [esi].state_table.note_count,0
                ret
flush_note_queue ENDP

;****************************************************************************
reset_sequence  PROC C \     		;Abandon all sequence-owned resources
                USES ebx esi edi \
                ,State:PTR

                mov esi,[State]

                mov edi,0
__for_chan:     mov ebx,edi
                mov al,[esi].state_table.chan_controls.SUS[ebx]
                cmp al,64
                jl __chk_lock
                mov global_controls.SUS[ebx],0
                or ebx,0b0h
                invoke send_MIDI_message,ebx,SUSTAIN,0
                mov ebx,edi

__chk_lock:     mov al,[esi].state_table.chan_controls.C_LOCK[ebx]
                cmp al,64
                jl __chk_cprot
                invoke flush_channel_notes,edi
                mov ebx,edi
                movzx ebx,[esi].state_table.chan_map[ebx]
                inc ebx
                invoke release_channel,0,ebx
                mov ebx,edi
                mov [esi].state_table.chan_map[ebx],bl

__chk_cprot:    mov al,[esi].state_table.chan_controls.C_PROT[ebx]
                cmp al,64
                jl __chk_vprot
                and lock_status[ebx],10111111b

__chk_vprot:    mov al,[esi].state_table.chan_controls.V_PROT[ebx]
                cmp al,64
                jl __next_chan
                or ebx,0b0h
                invoke send_MIDI_message,ebx,VOICE_PROTECT,0
                mov ebx,edi

__next_chan:    inc edi
                cmp edi,NUM_CHANS
                jne __for_chan

                ret
reset_sequence  ENDP

;****************************************************************************
restore_sequence PROC C \     		;Reassert all "owned" controls
                USES ebx esi edi\
                ,State:PTR
                LOCAL con,ctrl,index

                mov esi,[State]

                mov edi,0               ;re-lock any channels formerly locked
__for_lock:     mov ebx,edi             ;by CHAN_LOCK controllers
                mov al,[esi].state_table.chan_controls.C_LOCK[ebx]
                cmp al,-1
                je __next_lock
                cmp al,64
                jl __next_lock
                invoke lock_channel,0   ;lock new channel and map to current
                dec eax                 ;channel in sequence
                cmp eax,-1
                jne __locked
                mov eax,edi
__locked:       mov ebx,edi
                mov [esi].state_table.chan_map[ebx],al
__next_lock:    inc edi
                cmp edi,NUM_CHANS
                jne __for_lock

                mov con,0               ;re-establish all logged controller
__for_control:  mov ebx,con             ;values
                mov bl,logged_ctrls[ebx]
                cmp bl,CHAN_LOCK        ;(except channel locks, which were
                je __next_control       ;done above)
                mov ctrl,ebx
                mov bl,ctrl_hash[ebx]
                mov index,ebx            
                mov edi,0
__for_channel:  mov ebx,edi
                add ebx,index
                mov al,BYTE PTR [esi].state_table.chan_controls[ebx]
                cmp al,-1
                je __next_channel
                invoke XMIDI_control,esi,edi,ctrl,eax
__next_channel: inc edi
                cmp edi,NUM_CHANS
                jne __for_channel
__next_control: inc con
                cmp con,NUM_CONTROLS
                jne __for_control

                mov edi,0               ;restore pitch/program # values
__for_p_p:      mov ebx,edi
                mov al,[esi].state_table.chan_pitch_l[ebx]
                cmp al,-1
                je __set_prg
                mov dl,[esi].state_table.chan_pitch_h[ebx]
                cmp dl,-1
                je __set_prg
                mov bl,[esi].state_table.chan_map[ebx]
                or bx,0e0h
                invoke send_MIDI_message,ebx,eax,edx
                mov ebx,edi
__set_prg:      mov dl,[esi].state_table.chan_program[ebx]
                cmp dl,-1
                je __next_p_p
                mov bl,[esi].state_table.chan_map[ebx]
                or ebx,0c0h
                invoke send_MIDI_message,ebx,edx,0
__next_p_p:     inc edi
                cmp edi,NUM_CHANS
                jne __for_p_p

                ret
restore_sequence ENDP

;****************************************************************************
XMIDI_volume    PROC C \      		;Send updated volume control messages
                USES ebx esi edi \
                ,State:PTR

                mov esi,[State]       
                                       
                mov ebx,0                
__for_chan:     movzx eax,[esi].state_table.chan_controls.PV[ebx]
                cmp al,-1
                je __next_chan

                movzx ecx,WORD PTR [esi].state_table.vol_percent
                mul ecx                 ;else get scaled volume value
                mov ecx,100
                div ecx
                cmp eax,127
                jb __send
                mov eax,127

__send:         mov edi,ebx             ;update global controller shadow
                mov global_controls.PV[ebx],al
                test lock_status[ebx],10000000b
                jnz __next_chan         ;(logical channel locked)
                mov edi,ebx
                mov bl,[esi].state_table.chan_map[ebx]
                or bl,0b0h              ;send the new volume value
                invoke send_MIDI_message,ebx,PART_VOLUME,eax
                mov ebx,edi             ;recover channel

__next_chan:    inc ebx
                cmp ebx,NUM_CHANS
                jne __for_chan

                ret
XMIDI_volume    ENDP

;****************************************************************************
XMIDI_control   PROC\
                USES ebx esi edi\           ;Process XMIDI Control Change message
                ,State:PTR,Chan,Con,Val
                LOCAL prg_esp
                OPTION NOLJMP

                cld

                mov esi,[State]

                movzx ebx,BYTE PTR [Chan]       ;EBX=channel #
                movzx edx,BYTE PTR [Val]        ;EDX=value

                mov al,[esi].state_table.chan_indirect[ebx]
                cmp al,-1
                je __value                      ;no indirection pending, continue

                mov [esi].state_table.chan_indirect[ebx],-1
                mov bl,al
                mov edi,[esi].state_table.ctrl_ptr      ;else get value from controller table
                mov dl,[edi][ebx]      

__value:        movzx ebx,BYTE PTR [Con]        ;BX=controller #

                mov bl,ctrl_hash[ebx]
                cmp bl,-1
                je __interpret          ;(not loggable)

                add bl,BYTE PTR [Chan]	;copy controller value to state tables
                mov BYTE PTR global_controls[ebx],dl
                mov BYTE PTR [esi].state_table.chan_controls[ebx],dl

__interpret:    mov al,BYTE PTR [Con]	;handle sequence-specific controllers
                mov bl,BYTE PTR [Chan]
                cmp al,PART_VOLUME      ;BX=channel, AL=controller #, DX=value
                je __scale_volume       
                cmp al,CLEAR_BEAT_BAR   
                je __go_cc
                cmp al,CALLBACK_TRIG
                je __go_cb
                cmp al,FOR_LOOP
                je __go_for
                cmp al,NEXT_LOOP
                je __go_next
                cmp al,CHAN_PROTECT
                je __go_cp
                cmp al,CHAN_LOCK
                je __go_cl
                cmp al,INDIRECT_C_PFX
                je __go_indirect

__send:         test lock_status[ebx],10000000b
                jnz __exit              ;(logical channel locked)
                mov bl,[esi].state_table.chan_map[ebx]
                or bl,0b0h              ;send the controller value
                invoke send_MIDI_message,ebx,eax,edx
              
__exit:         mov eax,3               ;return total event size
                ret

__scale_volume: mov ecx,[esi].state_table.vol_percent
                cmp ecx,100
                je __send               ;100% volume, don't scale
                mov eax,edx
                mul ecx                 ;else get scaled volume value
                mov ecx,100
                div ecx
                mov edx,eax
                mov eax,PART_VOLUME
                cmp edx,127
                jb __send_volume
                mov edx,127
__send_volume:  mov global_controls.PV[ebx],dl
                jmp __send

__go_cc:        jmp __clear_cntrs
__go_cb:        jmp __callback
__go_for:       jmp __for_loop
__go_next:      jmp __next_loop
__go_cp:        jmp __chan_prot
__go_cl:        jmp __chan_lock
__go_indirect:  jmp __indirect

                OPTION LJMP

__indirect:     mov [esi].state_table.chan_indirect[ebx],dl
                jmp __exit

__clear_cntrs:  mov [esi].state_table.beat_count,0
                mov [esi].state_table.measure_count,0
                mov [esi].state_table.beat_fraction,0
                mov eax,[esi].state_table.time_fraction
                sub [esi].state_table.beat_fraction,eax
                jmp __exit

__callback:     mov [esi].state_table.cur_callback,edx
                mov eax,trigger_fn
                cmp eax,0
                je __exit               ;(callback functions disabled)
                pushfd

                mov prg_esp,esp         ;save SP before parameters pushed
                                        ;(use C calling convention)
                push edx                ;push sequence handle, ctrl value
                push [esi].state_table.seq_handle    
                call [trigger_fn]       ;and call the callback function

                mov esp,prg_esp         ;restore old SP value

                POP_F
                jmp __exit

__for_loop:     mov ebx,0               ;get index of available loop counter
                mov ecx,FOR_NEST
__for_find:     cmp [esi].state_table.FOR_loop_cnt[ebx],-1
                je __for_found
                add ebx,2
                loop __for_find
                jmp __exit
__for_found:    mov [esi].state_table.FOR_loop_cnt[ebx],dx
                shl ebx,1
                mov edi,[esi].state_table.EVNT_ptr      ;(NEXT controller will skip FOR)
                mov [esi].state_table.FOR_ptrs[ebx],edi
                jmp __exit

__next_loop:    cmp dl,64               ;BREAK controller (value < 64)?
                jl __exit               ;yes, ignore and continue

                mov ebx,(FOR_NEST*2)-2   
                mov ecx,FOR_NEST        ;else get index of inner loop counter
__next_find:    cmp [esi].state_table.FOR_loop_cnt[ebx],-1
                jne __next_found
                sub ebx,2
                loop __next_find
                jmp __exit
__next_found:   cmp [esi].state_table.FOR_loop_cnt[ebx],0
                je __do_loop            ;FOR value 0 = infinite loop
                dec [esi].state_table.FOR_loop_cnt[ebx]
                jnz __do_loop
                mov [esi].state_table.FOR_loop_cnt[ebx],-1
                jmp __exit              ;remove loop from list if dec'd to 0
__do_loop:      shl ebx,1
                mov edi,[esi].state_table.FOR_ptrs[ebx]
                mov [esi].state_table.EVNT_ptr,edi
                jmp __exit

__chan_prot:    or lock_status[ebx],01000000b
                cmp dl,64
                jge __exit
                and lock_status[ebx],10111111b
                jmp __exit

__chan_lock:    mov edi,ebx
                cmp dl,64
                jl __unlock
                invoke lock_channel,0   ;lock new channel and map to current
                dec eax                 ;channel in sequence
                cmp eax,-1
                jne __set_chan
                mov eax,edi
__set_chan:     mov ebx,edi
                mov [esi].state_table.chan_map[ebx],al
                jmp __exit

__unlock:       invoke flush_channel_notes,edi
                mov ebx,edi
                mov bl,[esi].state_table.chan_map[ebx]
                inc ebx
                invoke release_channel,0,ebx
                mov ebx,edi
                mov [esi].state_table.chan_map[ebx],bl
                jmp __exit              ;release and unmap locked channel

XMIDI_control   ENDP

;****************************************************************************
XMIDI_note_on   PROC\       		;Turn XMIDI note on, add to note queue
                USES ebx esi edi es\
                ,State:PTR
                LOCAL chan_note,vel,len ;Returns AX=size of Note On event

                push ds
                pop es

                mov esi,[State]          
                mov edi,[esi].state_table.EVNT_ptr;retrieve event data pointer
                movzx eax,WORD PTR [edi]         
                and al,0fh                      ;AL=channel, AH=note #
                mov chan_note,eax
                movzx eax,BYTE PTR [edi+2]      ;AL=velocity
                mov vel,eax

                mov eax,edi             ;get VLN duration value in EAX
                add edi,3
                mov ebx,0                
                mov edx,0
                jmp __calc_VLN
__shift_VLN:    mov ecx,7
__mul_128:      shl ebx,1
                loop __mul_128
__calc_VLN:     mov cl,[edi]
                inc edi
                mov ch,cl
                and cl,7fh
                or bl,cl
                or ch,ch
                js __shift_VLN

                sub edi,eax             ;get length of entire event
                mov len,edi             ;EBX = duration in q-intervals

                mov edi,chan_note
                and edi,0fh
                test lock_status[edi],10000000b
                jnz __exit              ;(logical channel locked)

                lea edi,[esi].state_table.note_chan
                mov ecx,MAX_NOTES       ;set up to scan the note queue for
                mov al,0ffh             ;an empty slot
                repne scasb             
                mov eax,edi
                jne __overflow          ;overwrite entry 0 if queue full
                inc [esi].state_table.note_count    ;else bump note counter...
                lea eax,[esi].state_table.note_chan+1   ;and index the empty slot
__overflow:     sub edi,eax             ;DI=queue slot [0,MAX_NOTES-1]

                mov eax,ebx             ;EAX = duration
                mov ebx,edi             ;EBX = queue slot
                sub eax,1               ;predecrement (note queue watches for
                                        ;negative durations)

                mov ecx,chan_note       ;log the note's channel and key #
                mov [esi].state_table.note_chan[ebx],cl
                mov [esi].state_table.note_num[ebx],ch
                mov [esi].state_table.note_time[ebx*4],eax

                mov ebx,0               ;translate logical to physical channel
                mov bl,cl
                mov bl,[esi].state_table.chan_map[ebx]
                inc active_notes[ebx]   ;inc # of notes in channel
                or bl,90h               ;turn the note on
                mov cl,ch
                invoke send_MIDI_message,ebx,ecx,vel

__exit:         mov eax,len             ;return event length
                ret
XMIDI_note_on   ENDP

;****************************************************************************
XMIDI_meta      PROC\      		;XMIDI meta-event interpreter
                USES ebx esi edi\
                ,State:PTR
                LOCAL event_len,event_type:BYTE

                mov esi,[State]         ;get pointers to state table and event
                mov edi,[esi].state_table.EVNT_ptr
                mov al,[edi+1]
                mov event_type,al

                mov ebx,edi             ;get offset of status byte
                add edi,2               ;adjust for type and status bytes
                mov eax,0               ;get variable-length number
                mov edx,0
                jmp __calc_VLN
__shift_VLN:    mov ecx,7
__mul_128:      shl eax,1
                loop __mul_128
__calc_VLN:     mov cl,[edi]
                inc edi
                mov ch,cl
                and cl,7fh
                or al,cl
                or ch,ch
                js __shift_VLN
                mov ecx,edi
                sub ecx,ebx             ;EBX = size of meta-event header
                add eax,ecx             ;add size of header to data length
                mov event_len,eax       ;to determine overall event length

                mov al,event_type
                cmp al,2fh
                je __end_sequence
                cmp al,58h
                je __time_sig
                cmp al,51h
                je __set_tempo

__exit:         mov eax,event_len       ;return total event length
                ret

__end_sequence: invoke reset_sequence,esi
                mov [esi].state_table.status,SEQ_DONE
                cmp [esi].state_table.post_release,0 ;release-on-completion pending?
                je __exit              
                invoke release_seq,0,current_handle
                jmp __exit

__time_sig:     movzx ecx,BYTE PTR [edi]
                mov [esi].state_table.time_numerator,cx

                mov cl,[edi+1]
                sub ecx,2
                jae __do_mult
                neg ecx

                mov eax,QUANT_TIME_16
__div_quant:    shr eax,1
                loop __div_quant
                jmp __end_calc

__do_mult:      mov eax,1 
                shl eax,cl
                mov ecx,eax
                mov eax,0
__mul_quant:    add eax,QUANT_TIME_16
                loop __mul_quant

__end_calc:     mov [esi].state_table.time_fraction,eax
                
                mov [esi].state_table.beat_fraction,0
                sub [esi].state_table.beat_fraction,eax

                mov [esi].state_table.beat_count,0
                inc [esi].state_table.measure_count
                jmp __exit

__set_tempo:    mov ah,0
                mov al,[edi]
                shl eax,16
                mov ah,[edi+1]
                mov al,[edi+2]

                shl eax,4

                mov [esi].state_table.time_per_beat,eax
                jmp __exit

XMIDI_meta      ENDP

;****************************************************************************
XMIDI_sysex     PROC\       		;XMIDI System Exclusive interpreter
                USES ebx esi edi\
                ,State:PTR
                LOCAL event_len,event_type

                mov esi,[State]         ;get pointers to state table and event
                mov edi,[esi].state_table.EVNT_ptr
                movzx eax,BYTE PTR [edi]
                mov event_type,eax

                mov ebx,edi             ;get offset of type byte
                inc edi                 ;adjust for type (F0 | F7) byte
                mov eax,0               ;get variable-length number
                jmp __calc_VLN
__shift_VLN:    shl ax,7                ;multiply by 128
__calc_VLN:     mov cl,[edi]
                inc edi
                mov ch,cl
                and cl,7fh
                or al,cl
                or ch,ch
                js __shift_VLN
                mov ecx,edi
                sub ecx,ebx             ;BX = size of header (type + len)
                add ecx,eax             ;add size of header to data length
                mov event_len,ecx       ;to determine overall event length

                IFDEF send_MIDI_sysex
                invoke send_MIDI_sysex,edi,event_type,eax
                ENDIF

                mov eax,event_len       ;return total event length
                ret
XMIDI_sysex     ENDP

;*****************************************************************************
ul_divide       PROC\			;Unsigned long division
                USES ebx esi edi\
                ,Num,Den

                mov eax,[Num]
                mov ebx,[Den]
                div ebx
                ret

ul_divide       ENDP

;****************************************************************************
advance_count   PROC\                   ;Fetch EDX=bar, EAX=beat "advanced" by
                USES ebx esi edi\           ;QUANT_ADVANCE intervals
                ,Sequence

                mov esi,[Sequence]
                mov esi,sequence_state[esi]

                movzx eax,[esi].state_table.beat_count
                movzx edx,[esi].state_table.measure_count

                IF QUANT_ADVANCE        ;anticipate future changes to beat/bar
                                        ;count (assuming current MIDI meter!)
                mov edi,QUANT_ADVANCE
                mov ebx,[esi].state_table.beat_fraction

__advance_loop: add ebx,[esi].state_table.time_fraction

                cmp ebx,[esi].state_table.time_per_beat
                jl __advance_next

                sub ebx,[esi].state_table.time_per_beat
                
                inc eax                 ;bump beat if ticks > ticks per beat

                cmp ax,[esi].state_table.time_numerator
                jb __advance_next

                mov eax,0               ;bump measure if beat > time numerator
                inc edx

__advance_next: dec edi
                jnz __advance_loop

                ENDIF

                ret                     
advance_count   ENDP                    

;****************************************************************************
;*                                                                          *
;*  Public (API-accessible) procedures                                      *
;*                                                                          *
;****************************************************************************
serve_driver    PROC\                   ;Periodic driver service routine
                USES ebx esi edi
                LOCAL seqcnt
                OPTION NOLJMP		;disable jump fixups for speed

                cld

                cmp service_active,0    ;OK to interrupt foreground process?
                je __do_service         
                jmp __served            ;no

__new_beat:     sub eax,[esi].state_table.time_per_beat
                inc [esi].state_table.beat_count
                movzx ecx,[esi].state_table.beat_count
                cmp cx,[esi].state_table.time_numerator
                jb __same_beat
                mov [esi].state_table.beat_count,0
                inc [esi].state_table.measure_count
                jmp __same_beat

__do_service:   inc service_active

                mov current_handle,-4   ;for all sequences....

                mov ecx,sequence_count
                mov seqcnt,ecx
                jecxz __end_seqs

__for_seq:      add current_handle,4
                mov edi,current_handle
                cmp sequence_state[edi],0
                je __for_seq            ;(sequence not registered)

                mov esi,sequence_state[edi]
                cmp [esi].state_table.status,SEQ_PLAYING
                jne __next_seq          ;(sequence not playing)

                mov eax,[esi].state_table.tempo_error
                add eax,[esi].state_table.tempo_percent
                mov [esi].state_table.tempo_error,eax
                sub eax,100
                jl __chk_t_grad
__rep_interval: mov [esi].state_table.tempo_error,eax

                cmp [esi].state_table.note_count,0  ;any notes on in sequence?
                jne __do_queue          ;yes, turn off any expired notes

__end_queue:    dec [esi].state_table.interval_cnt
                jle __do_events         ;next interval ready, play it

__end_events:   mov eax,[esi].state_table.beat_fraction
                add eax,[esi].state_table.time_fraction
                cmp eax,[esi].state_table.time_per_beat
                jge __new_beat
__same_beat:    mov [esi].state_table.beat_fraction,eax
                
                mov eax,[esi].state_table.tempo_error
                sub eax,100
                jge __rep_interval

__chk_t_grad:   mov eax,[esi].state_table.tempo_percent
                cmp eax,[esi].state_table.tempo_target
                jne __do_temp_grad

__chk_v_grad:   mov eax,[esi].state_table.vol_percent
                cmp eax,[esi].state_table.vol_target
                jne __do_vol_grad

__next_seq:     dec seqcnt
                jne __for_seq

__end_seqs:     IFDEF serve_synth       ;update synthesizer hardware regs and
                call serve_synth        ;time-variant effects, if applicable
                ENDIF

                dec service_active
__served:       ret

__do_temp_grad: jmp __tempo_grad
__do_vol_grad:  jmp __volume_grad

__do_events:    jmp __get_event   

__do_queue:     lea edi,[esi].state_table.note_chan ;check note queue for expired notes
                mov edx,edi
                mov ecx,MAX_NOTES        
__scan_queue:   mov ax,ds
                mov es,ax
                mov al,0ffh             
__next_scan:    repe scasb             
                je __end_queue          ;no active notes left, return to loop
                mov ebx,edi
                sub ebx,edx
                dec ebx                 ;get offset of note in queue
                sub [esi].state_table.note_time[ebx*4],1
                jge __next_scan         ;not yet expired, keep searching
                push ecx
                mov al,-1               ;else mark note entry as "free"
                xchg al,[esi].state_table.note_chan[ebx]
                mov cl,[esi].state_table.note_num[ebx]
                movzx ebx,al            ;translate logical to physical channel
                movzx ebx,[esi].state_table.chan_map[ebx]
                dec active_notes[ebx]   ;dec # of active notes in channel
                or bl,80h               ;send MIDI Note Off message
                invoke send_MIDI_message,ebx,ecx,0
                pop ecx
                lea edx,[esi].state_table.note_chan     ;restore queue length & base pointer
                dec [esi].state_table.note_count
                jnz __scan_queue        ;keep searching if any notes left
                jmp __end_queue         ;else return to interval loop

__new_interval: add [esi].state_table.EVNT_ptr,1
                mov [esi].state_table.interval_cnt,ax
                jmp __end_events

__get_event:    mov edi,[esi].state_table.EVNT_ptr      ;get next event status
                movzx eax,BYTE PTR [edi];AL = channel/status
                cmp eax,128             ;XMIDI interval count?
                jb __new_interval       ;yes, store it and continue   

                mov ebx,eax             ;EDI->EVNT_ptr; ESI->state table
                and eax,0f0h            ;AX = status
                and ebx,00fh            ;BX = logical channel
                mov cl,[edi+1]          ;CL = data byte 1
                mov dl,[edi+2]          ;DL = data byte 2

                cmp eax,0f0h            ;branch to XMIDI event handler for
                jae __sys               ;current MIDI status byte value
                cmp eax,0e0h
                jae __pitch_wheel
                mov edi,2
                cmp eax,0d0h
                jae __send_event
                cmp eax,0c0h
                jae __prg_change
                cmp eax,0b0h
                jae __ctrl_change
                mov edi,3
                cmp eax,0a0h
                jae __send_event

                invoke XMIDI_note_on,esi
                mov edi,eax
                jmp __end_event

__sys:          cmp bl,0fh         
                je __meta
                invoke XMIDI_sysex,esi
                mov edi,eax
                jmp __end_event

__meta:         invoke XMIDI_meta,esi
                mov edi,eax
                jmp __end_event

__ctrl_change:  invoke XMIDI_control,esi,ebx,ecx,edx
                mov edi,3
                jmp __end_event

__pitch_wheel:  mov [esi].state_table.chan_pitch_l[ebx],cl
                mov [esi].state_table.chan_pitch_h[ebx],dl
                mov global_pitch_l[ebx],cl
                mov global_pitch_h[ebx],dl
                mov edi,3
                jmp __send_event

__prg_change:   mov [esi].state_table.chan_program[ebx],cl
                mov global_program[ebx],cl
                mov edi,2

__send_event:   test lock_status[ebx],10000000b
                jnz __end_event         ;(logical channel locked)
                or al,[esi].state_table.chan_map[ebx]
                invoke send_MIDI_message,eax,ecx,edx

__end_event:    add [esi].state_table.EVNT_ptr,edi
                cmp [esi].state_table.status,SEQ_PLAYING
                jne __end_sequence
                jmp __get_event
__end_sequence: jmp __next_seq          

__tempo_grad:   pushfd
                mov eax,[esi].state_table.tempo_accum
                add eax,(QUANT_TIME / 100)
                mov ecx,-1              ;CX=total tempo change/tick
__for_tempo:    inc ecx
                mov [esi].state_table.tempo_accum,eax
__next_tempo:   sub eax,[esi].state_table.tempo_period
                jge __for_tempo
                POP_F                   ;restore result of comparison
                jecxz __end_t_grad      ;(no change this tick)
                mov ebx,[esi].state_table.tempo_target
                mov eax,[esi].state_table.tempo_percent
                jl __add_tempo            
                sub eax,ecx
                cmp eax,ebx
                jge __set_tempo
                jmp __end_tempo
__add_tempo:    add eax,ecx
                cmp eax,ebx
                jle __set_tempo
__end_tempo:    mov eax,ebx
__set_tempo:    mov [esi].state_table.tempo_percent,eax
__end_t_grad:   jmp __chk_v_grad

__volume_grad:  pushfd
                mov eax,[esi].state_table.vol_accum
                add eax,(QUANT_TIME / 100)
                mov ecx,-1               ;ECX=total vol change/tick
__for_vol:      inc ecx
                mov [esi].state_table.vol_accum,eax
__next_vol:     sub eax,[esi].state_table.vol_period
                jge __for_vol
                POP_F                   ;restore result of comparison
                jcxz __end_v_grad       ;(no change this tick)
                mov ebx,[esi].state_table.vol_target
                mov eax,[esi].state_table.vol_percent
                jl __add_vol            
                sub eax,ecx
                cmp eax,ebx
                jge __set_vol
                jmp __end_vol
__add_vol:      add eax,ecx
                cmp eax,ebx
                jle __set_vol
__end_vol:      mov eax,ebx
__set_vol:      mov [esi].state_table.vol_percent,eax
                invoke XMIDI_volume,esi
__end_v_grad:   jmp __next_seq

                OPTION LJMP		;(resume automatic jump calculation)
serve_driver    ENDP

;****************************************************************************
init_driver     PROC\
                USES ebx esi edi es\
                ,H,IO_ADDR,IRQ,DMA,DRQ
                pushfd
                cli
                cld

                push ds
                pop es

                mov service_active,0
                mov sequence_count,0

                mov eax,-1
                mov edi,OFFSET global_shadow
                mov ecx,GLOBAL_SIZE/2
                rep stosw
                mov edi,OFFSET ctrl_hash
                mov ecx,(SIZE ctrl_hash)/2
                rep stosw

                mov eax,0
                mov edi,OFFSET sequence_state
                mov ecx,(SIZE sequence_state)/2
                rep stosw
                mov edi,OFFSET lock_status
                mov ecx,(SIZE lock_status)/2
                rep stosw
                mov edi,OFFSET active_notes
                mov ecx,(SIZE active_notes)/2
                rep stosw

                mov esi,0               ;create fast lookup table for 
                mov eax,0               ;XMIDI controller address offsets
                mov ebx,0
__create_hash:  mov bl,logged_ctrls[esi]
                mov ctrl_hash[ebx],al
                add eax,NUM_CHANS
                inc esi
                cmp esi,NUM_CONTROLS
                jne __create_hash

                IFDEF set_IO_parms
                invoke set_IO_parms,[IO_ADDR],[IRQ],[DMA],[DRQ]
                ENDIF
                IFDEF reset_interface
                call reset_interface
                ENDIF
                IFDEF init_interface
                call init_interface
                ENDIF
                call reset_synth
                call init_synth
                call cancel_callback

                mov esi,0                       ;init MIDI/XMIDI controllers
__for_ctrl:     mov edi,MIN_TRUE_CHAN-1         ;to nominal default values
__for_chan:     mov eax,edi
                or eax,0b0h
                movzx ebx,logged_ctrls[esi]
                mov cl,ctrl_default[esi]
                cmp cl,-1
                je __next_ctrl
                mov edx,ebx
                mov bl,ctrl_hash[ebx]
                add ebx,edi
                mov BYTE PTR global_controls[ebx],cl
                invoke send_MIDI_message,eax,edx,ecx
                inc edi
                cmp edi,MAX_REC_CHAN-1
                jbe __for_chan
__next_ctrl:    inc esi
                cmp esi,NUM_CONTROLS-1
                jbe __for_ctrl

                IFDEF sysex_wait
                invoke sysex_wait,10            ;wait for FIFO to empty if
                ENDIF                           ;necessary

                mov edi,MIN_TRUE_CHAN-1         ;init pitch/programs
__for_p_p:      mov global_pitch_l[edi],DEF_PITCH_L
                mov global_pitch_h[edi],DEF_PITCH_H
                mov eax,edi
                or eax,0e0h
                invoke send_MIDI_message,eax,DEF_PITCH_L,DEF_PITCH_H
                mov bl,prg_default[di-(MIN_TRUE_CHAN-1)]
                cmp bl,-1
                je __next_p_p
                mov global_program[edi],bl
                mov eax,edi
                or eax,0c0h
                invoke send_MIDI_message,eax,ebx,0
__next_p_p:     inc edi
                cmp edi,MAX_REC_CHAN-1
                jbe __for_p_p

                IFDEF sysex_wait
                invoke sysex_wait,10            ;wait for FIFO to empty if
                ENDIF                           ;necessary

                mov init_OK,1

                POP_F
                ret
init_driver     ENDP

;****************************************************************************
shutdown_driver PROC\
                USES ebx esi edi\
                ,H,SignOff:PTR
                LOCAL handle,seqcnt

                pushfd
                cli

                cmp init_OK,0
                je __exit

                mov handle,0            ;for all sequences....
                mov ecx,sequence_count
                mov seqcnt,ecx
                jecxz __shutdown
__for_seq:      mov edi,handle
                add handle,4
                cmp sequence_state[edi],0
                je __for_seq            ;(sequence not registered)
                invoke stop_seq,0,edi
                invoke release_seq,0,edi
__next_seq:     dec seqcnt
                jne __for_seq

__shutdown:     call reset_synth

                IFDEF write_display
                invoke write_display,0,[SignOff]
                ENDIF

                IFDEF reset_interface
                call reset_interface
                ENDIF

                IFDEF shutdown_synth
                call shutdown_synth
                ENDIF

                mov init_OK,0

__exit:         POP_F
                ret
shutdown_driver ENDP

;****************************************************************************
get_state_size  PROC\
                USES ebx esi edi\
                ,H

                pushfd
                cli

                mov eax,SIZE state_table

                POP_F
                ret
get_state_size  ENDP

;****************************************************************************
install_callback PROC\      		;Declare callback trigger handler
                USES ebx esi edi\
                ,H,Fn:PTR

                pushfd
                cli

                mov edi,[Fn]
                mov trigger_fn,edi

                POP_F
                ret
install_callback ENDP

;****************************************************************************
cancel_callback PROC\                  ;Disable callback trigger calls
                USES ebx esi edi\
                ,H

                pushfd
                cli

                mov trigger_fn,0

                POP_F
                ret
cancel_callback ENDP

;****************************************************************************
register_seq    PROC\             	;Initialize sequence state table and
                USES ebx esi edi\
                ,H,XMID:PTR,Num,State:PTR,Ctrl:PTR
                LOCAL handle,chunk_len

                pushfd                   ;return sequence handle
                cli

                mov ebx,0               ;look for an unused sequence handle
                mov ecx,NSEQS
__find_handle:  cmp sequence_state[ebx],0
                je __handle_found
                add ebx,4
                loop __find_handle

                mov eax,-1              ;no sequence handles available...
                jmp __exit              ;return failure

__handle_found: mov handle,ebx

                invoke find_seq,[XMID],[Num]
                cmp eax,0
                je __bad                ;sequence not found, exit...

                mov edi,eax
                mov chunk_len,12        ;else skip FORM <len> XMID header
                
                mov esi,[State]
                mov sequence_state[ebx],esi

                mov [esi].state_table.TIMB,0
                mov [esi].state_table.RBRN,0
                mov [esi].state_table.EVNT,0

__log_chunk:    add edi,chunk_len

                mov ax,[edi+4]
                xchg al,ah
                shl eax,16
                mov ax,[edi+6]
                xchg al,ah
                add eax,8
                mov chunk_len,eax

                cmp [edi],DWORD PTR 'BMIT'	;TIMB = list of required timbres
                jne __not_TIMB          ;(optional)
                mov [esi].state_table.TIMB,edi
                jmp __log_chunk

__not_TIMB:     cmp [edi],DWORD PTR 'NRBR'      ;RBRN = list of branch points
                jne __not_RBRN          ;(optional)
                mov [esi].state_table.RBRN,edi
                jmp __log_chunk

__not_RBRN:     cmp [edi],DWORD PTR 'TNVE'      ;EVNT = MIDI event list
                jne __log_chunk         ;(mandatory; must be last chunk in

                mov eax,handle
                mov [esi].state_table.seq_handle,eax
                mov [esi].state_table.EVNT,edi

                mov edi,[Ctrl]
                mov [esi].state_table.ctrl_ptr,edi

                mov [esi].state_table.post_release,0

                mov [esi].state_table.seq_started,0
                mov [esi].state_table.status,SEQ_STOPPED
                inc sequence_count

                invoke rewind_seq,handle

                mov eax,handle
                jmp __exit

__bad:          mov eax,-1
__exit:         POP_F
                ret
register_seq    ENDP

;****************************************************************************
release_seq     PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli
 
                mov esi,[Sequence]
                cmp esi,-1
                je __exit

                cmp sequence_state[esi],0
                je __exit

                mov edi,sequence_state[esi]
                cmp [edi].state_table.status,SEQ_PLAYING
                jne __release

                mov [edi].state_table.post_release,1
                jmp __exit

__release:      mov sequence_state[esi],0
                dec sequence_count

__exit:         POP_F
                ret
release_seq     ENDP

;****************************************************************************
start_seq       PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli

                mov esi,[Sequence]
                cmp esi,-1
                je __exit
              
                mov esi,sequence_state[esi]
              
                cmp [esi].state_table.status,SEQ_PLAYING
                jne __start
                invoke stop_seq,0,[Sequence]
              
__start:        invoke rewind_seq,[Sequence]
              
                mov eax,[esi].state_table.EVNT
                add eax,8
                mov [esi].state_table.EVNT_ptr,eax
              
                mov [esi].state_table.status,SEQ_PLAYING
                mov [esi].state_table.seq_started,1

__exit:         POP_F
                ret
start_seq	ENDP

;****************************************************************************
stop_seq        PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli

                mov esi,[Sequence]
                cmp esi,-1
                je __exit

                cmp sequence_state[esi],0
                je __exit

                mov esi,sequence_state[esi]

                cmp [esi].state_table.status,SEQ_PLAYING
                jne __exit

                invoke flush_note_queue,esi
                invoke reset_sequence,esi

                mov [esi].state_table.status,SEQ_STOPPED

__exit:         POP_F
                ret
stop_seq        ENDP

;****************************************************************************
resume_seq      PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli

                mov esi,[Sequence]
                cmp esi,-1
                je __exit

                cmp sequence_state[esi],0
                je __exit

                mov esi,sequence_state[esi]

                cmp [esi].state_table.status,SEQ_STOPPED
                jne __exit

                cmp [esi].state_table.seq_started,0
                je __exit

                invoke restore_sequence,esi

                mov [esi].state_table.status,SEQ_PLAYING
                                           
__exit:         POP_F
                ret
resume_seq      ENDP

;****************************************************************************
get_seq_status  PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli

                mov esi,[Sequence]

                mov eax,-1
                cmp esi,eax
                je __exit

                mov esi,sequence_state[esi]
                movzx eax,WORD PTR [esi].state_table.status

__exit:         POP_F
                ret
get_seq_status  ENDP

;****************************************************************************
get_beat_count  PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli

                mov esi,[Sequence]

                mov eax,-1
                cmp esi,eax
                je __exit

                invoke advance_count,esi

__exit:         POP_F
                ret
get_beat_count  ENDP

;****************************************************************************
get_bar_count   PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli

                mov esi,[Sequence]

                mov eax,-1
                cmp esi,eax
                je __exit

                invoke advance_count,esi

                mov eax,edx

__exit:         POP_F
                ret
get_bar_count   ENDP

;****************************************************************************
map_seq_channel PROC\
                USES ebx esi edi\
                ,H,Sequence,SeqChan,PhysChan

                pushfd
                cli

                mov esi,[Sequence]
                cmp esi,-1
                je __exit

                mov esi,sequence_state[esi]
                mov ebx,[SeqChan]
                dec ebx
                mov eax,[PhysChan]
                dec eax
                mov [esi].state_table.chan_map[ebx],al

__exit:         POP_F
                ret
map_seq_channel ENDP

;****************************************************************************
true_seq_channel PROC\
                USES ebx esi edi\
                ,H,Sequence,SeqChan

                pushfd
                cli

                mov esi,[Sequence]
                mov eax,-1
                cmp esi,eax
                je __exit

                mov esi,sequence_state[esi]
                mov ebx,[SeqChan]
                dec ebx
                mov eax,0
                mov al,[esi].state_table.chan_map[ebx]
                inc eax

__exit:         POP_F
                ret
true_seq_channel ENDP

;****************************************************************************
branch_index    PROC\
                USES ebx esi edi\
                ,H,Sequence,Marker

                pushfd
                cli

                mov esi,[Sequence]
                cmp esi,-1
                je __exit

                mov esi,sequence_state[esi]

                cmp [esi].state_table.RBRN,0
                je __exit               ;no branch points, exit               

                                        ;make sure RBRN chunk is still present
                mov edi,[esi].state_table.RBRN  
                cmp [edi],DWORD PTR 'NRBR'
                jne __exit              ;if not, no branching is possible

                movzx ecx,WORD PTR [edi+8]         
                add edi,10              ;get RBRN.cnt
                mov al,BYTE PTR [Marker]         
__find_marker:  cmp [edi],al
                je __marker_found
__find_next:    add edi,6               ;sizeof(RBRN.entry)
                loop __find_marker
                jmp __exit              ;marker not found in RBRN chunk

__marker_found: mov eax,[edi+2]         ;else get offset of target in EVNT
                add eax,8
                mov edi,[esi].state_table.EVNT
                add edi,eax
                mov [esi].state_table.EVNT_ptr,edi
                mov [esi].state_table.interval_cnt,0
                invoke flush_note_queue,esi

                IF BRANCH_EXIT          ;cancel all FOR...NEXT loops if
                mov ecx,FOR_NEST        ;BRANCH_EXIT is TRUE
                mov ebx,0
__init_FOR:     mov [esi].state_table.FOR_loop_cnt[ebx],-1
                add ebx,2
                loop __init_FOR
                ENDIF

__exit:         POP_F
                ret
branch_index    ENDP

;****************************************************************************
get_rel_tempo   PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli

                mov esi,[Sequence]
                mov eax,-1
                cmp esi,eax
                je __exit

                mov esi,sequence_state[esi]
                mov eax,[esi].state_table.tempo_percent

__exit:         POP_F
                ret
get_rel_tempo   ENDP

;****************************************************************************
get_rel_volume  PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli

                mov esi,[Sequence]
                mov eax,-1
                cmp esi,eax
                je __exit

                mov esi,sequence_state[esi]
                mov eax,[esi].state_table.vol_percent

__exit:         POP_F
                ret
get_rel_volume  ENDP


;****************************************************************************
set_rel_tempo   PROC\
                USES ebx esi edi\
                ,H,Sequence,Tempo,Grad
                pushfd
                cli

                mov esi,[Sequence]
                cmp esi,-1
                je __exit

                mov esi,sequence_state[esi]
                mov eax,[Tempo]
                mov [esi].state_table.tempo_target,eax

                cmp [Grad],0
                je __immed

                mov eax,[esi].state_table.tempo_target
                sub eax,[esi].state_table.tempo_percent
                jz __exit               ;(no difference specified)

                cdq
                xor eax,edx             
                sub eax,edx
                mov ecx,eax             ;CX = tempo delta

                mov eax,10              ;get # of 100us periods/step
                mul [Grad]           
                div ecx
                cmp eax,0
                jnz __nonzero
                mov eax,1
__nonzero:      mov [esi].state_table.tempo_period,eax
                mov [esi].state_table.tempo_accum,0
                jmp __exit

__immed:        mov [esi].state_table.tempo_percent,eax

__exit:         POP_F
                ret
set_rel_tempo   ENDP

;****************************************************************************
set_rel_volume  PROC\
                USES ebx esi edi\
                ,H,Sequence,Volume,Grad
                pushfd
                cli

                mov esi,[Sequence]
                cmp esi,-1
                je __exit

                mov esi,sequence_state[esi]
                mov eax,[Volume]
                mov [esi].state_table.vol_target,eax

                cmp [Grad],0
                je __immed

                mov eax,[esi].state_table.vol_target
                sub eax,[esi].state_table.vol_percent
                jz __exit               ;(no difference specified)

                cdq
                xor eax,edx 
                sub eax,edx
                mov ecx,eax             ;CX = vol delta

                mov eax,10              ;get # of 100us periods/step
                mul [Grad]
                div ecx
                cmp eax,0
                jnz __nonzero
                mov eax,1
__nonzero:      mov [esi].state_table.vol_period,eax
                mov [esi].state_table.vol_accum,0
                jmp __exit

__immed:        mov [esi].state_table.vol_percent,eax
                invoke XMIDI_volume,esi

__exit:         POP_F
                ret
set_rel_volume  ENDP

;****************************************************************************
get_control_val PROC\
                USES ebx esi edi\
                ,H,Sequence,Chan,Control

                pushfd
                cli

                mov esi,[Sequence]

                mov eax,-1
                cmp esi,eax
                je __exit

                mov esi,sequence_state[esi]

                mov ebx,[Control]

                cmp ebx,CALLBACK_TRIG   ;allow application to poll last
                jne __not_cb            ;callback trigger controller

                mov eax,[esi].state_table.cur_callback
                jmp __exit

__not_cb:       mov bl,ctrl_hash[ebx]
                cmp bl,-1
                je __exit               ;controller value not maintained, exit

                add ebx,[Chan]
                dec ebx
                mov eax,0               ;else return current controller value
                mov al,BYTE PTR [esi].state_table.chan_controls[ebx]

__exit:         POP_F
                ret
get_control_val ENDP

;****************************************************************************
set_control_val PROC\
                USES ebx esi edi\
                ,H,Sequence,Chan,Control,Val

                pushfd
                cli

                mov esi,[Sequence]
                cmp esi,-1
                je __exit

                mov esi,sequence_state[esi]

                mov eax,[Chan]
                dec eax
                invoke XMIDI_control,esi,eax,[Control],[Val]

__exit:         POP_F
                ret
set_control_val ENDP

;****************************************************************************
get_chan_notes  PROC\
                USES ebx esi edi\
                ,H,Sequence,Chan

                pushfd
                cli

                mov esi,[Sequence]

                mov eax,-1
                cmp esi,eax
                je __exit

                mov esi,sequence_state[esi]

                mov eax,0
                mov ebx,0
                mov ecx,[Chan]
                dec ecx
__count_notes:  cmp [esi].state_table.note_chan[ebx],cl
                jne __next_note
                inc eax
__next_note:    inc ebx
                cmp ebx,MAX_NOTES
                jne __count_notes

__exit:         POP_F
                ret
get_chan_notes  ENDP

;****************************************************************************
lock_channel    PROC\           	;return 0 if no channel available
                USES ebx esi edi\	        ;for locking
                ,H

                pushfd
                cli

                mov ecx,-1              ;find highest channel # w/lowest note
                mov esi,ecx             ;activity
                mov eax,11000000b       ;skip locked and protected channels
__do_search:    mov edi,MAX_TRUE_CHAN-1
__find_channel: test lock_status[edi],al
                jnz __find_next      
                cmp active_notes[edi],cl
                jae __find_next         ;'jae' gives priority to higher chans
                mov cl,active_notes[edi]
                mov esi,edi
__find_next:    dec edi
                cmp edi,MIN_TRUE_CHAN-1 ;(1-based channel #'s)
                jge __find_channel

                cmp esi,-1              
                jne __got_channel

                cmp eax,10000000b
                je __exit
                mov eax,10000000b       ;if no channels available for locking,
                jmp __do_search         ;ignore lock protection & try again

__got_channel:  or esi,0b0h
                invoke send_MIDI_message,esi,SUSTAIN,0
                and esi,0fh
                invoke flush_channel_notes,esi
                mov active_notes[esi],0

                or lock_status[esi],10000000b

__exit:         mov eax,esi
                inc eax
                POP_F
                ret
lock_channel    ENDP

;****************************************************************************
release_channel PROC\
                USES ebx esi edi\
                ,H,Chan
                pushfd
                cli
                
                mov esi,[Chan]
                dec esi
                test lock_status[esi],10000000b
                jz __exit               ;channel not locked, exit
                and lock_status[esi],01111111b

                mov active_notes[esi],0 ;silence the channel
                or esi,0b0h             ;(caller responsible for housekeeping)
                invoke send_MIDI_message,esi,SUSTAIN,0
                invoke send_MIDI_message,esi,ALL_NOTES_OFF,0

                and esi,0fh             ;update channel controller values
                mov ebx,esi
                mov edi,0
__for_ctrl:     mov dl,BYTE PTR global_controls[ebx]
                cmp dl,-1               ;controller value was never set in
                je __next_ctrl          ;channel, skip it
                push ebx
                and bl,0fh              ;else isolate channel #...
                or bl,0b0h              ;and send Control Change message
                mov al,logged_ctrls[edi]
                invoke send_MIDI_message,ebx,eax,edx
                pop ebx
__next_ctrl:    add ebx,NUM_CHANS       ;index next controller value
                inc edi                 ;index next controller type
                cmp edi,NUM_CONTROLS
                jne __for_ctrl

                and esi,0fh
                mov al,global_program[esi]
                cmp al,-1               ;update channel program #
                je __restore_pw
                or esi,0c0h
                invoke send_MIDI_message,esi,eax,0

__restore_pw:   and esi,0fh             ;update channel pitch wheel
                mov al,global_pitch_l[esi]
                cmp al,-1
                je __exit
                mov dl,global_pitch_h[esi]
                cmp dl,-1
                je __exit
                or esi,0e0h
                invoke send_MIDI_message,esi,eax,edx

__exit:         POP_F
                ret
release_channel ENDP

;****************************************************************************
               END
