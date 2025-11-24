;ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
;лл                                                                         лл
;лл  SPKR32.INC                                                             лл
;лл                                                                         лл
;лл  IBM Audio Interface Library -- IBM PC/Tandy XMIDI interpreter          лл
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

update_voice    PROTO C,Slot:DWORD      ;Update hardware regs for slot

                ;
                ;Driver-specific configuration equates
                ;

                IFDEF IBMPC
MAX_REC_CHAN    equ 2                   ;Max channel recognized by synths
MAX_TRUE_CHAN   equ 2                   ;Max channel available for locking
NUM_VOICES      equ 1                   ;# of physical voices available
NUM_SLOTS       equ 2                   ;# of virtual voices available
                ELSEIFDEF TANDY
MAX_REC_CHAN    equ 4 
MAX_TRUE_CHAN   equ 4
NUM_VOICES      equ 3
NUM_SLOTS       equ 4
                ENDIF

MIN_TRUE_CHAN   equ 2                   ;Min channel # (1-based)

DEF_SYNTH_VOL   equ 100                 ;Init vol=100%
DEF_TC_SIZE     equ 0                   ;No timbres used
MAX_TIMBS       equ 0                  

VEL_SENS        equ 1                   ;Velocity sensitivity disabled if 0

VEL_TRUE        equ 0                   ;Full velocity sensitivity range if 1
                                        ;(set to 0 to reduce playback noise)
DEF_PITCH_RANGE equ 12                  ;Default pitch wheel range (semitones)

LINEAR_PITCH    equ 1                   ;0 for semitonal pitch wheel response

EMULATE_MOD     equ 1                   ;(Active only w/LINEAR_PITCH = 1)
MOD_RATE        equ 20                  ; FM simulated at 20 Hz
MOD_DEPTH       equ 5                   ; 5 pitch wheel units/modulation unit

                ;
                ;Driver Description Table (DDT)
                ;Returned by describe_driver() proc
                ;

DDT             LABEL WORD
min_API_version dd 200                  ;Minimum API version required = 2.00
drvr_type       dd 3                    ;Type 3: XMIDI emulation
data_suffix     db 4 dup (0)            ;No Global Timbre Library used
device_name_o   dd OFFSET devnames      ;Pointer to list of supported devices
default_IO      dd -1                   ;Factory default I/O parameters
default_IRQ     dd -1
default_DMA     dd -1
default_DRQ     dd -1
service_rate    dd QUANT_RATE           ;Request QUANT_RATE calls/second 
display_size    dd 0                    ;No display

devnames        LABEL BYTE
                IFDEF IBMPC
                db 'IBM PC or compatible internal speaker',0
                ELSEIFDEF TANDY
                db 'Tandy 3-voice internal sound',0
                ENDIF
                db 0                    ;0 to end list of device names

                ;
                ;Default setup values & internal constants
                ;

                IFDEF IBMPC

value_l         db 136,181,78,64,65,205,196,61,67,124,42,214
                db 136,181,255,209,32,167,226,30,206,190,242,138
                db 68,65,127,232,144,99,99,143,231,95,1,189
                db 162,160,191,244,72,177,49,199,112,47,254,224
                db 209,208,222,251,36,88,152,227,57,151,255,111
                db 232,104,239,125,17,172,76,241,156,75,255,183
                db 116,52,247,190,136,86,38,248,206,165,127,91
                db 58,26,251,223,196,171,147,124,103,82,63,45

value_h         db 142,134,253,240,226,213,201,190,179,169,160,150
                db 142,134,126,119,113,106,100,95,89,84,79,75
                db 71,67,63,59,56,53,50,47,44,42,40,37
                db 35,33,31,29,28,26,25,23,22,21,19,18
                db 17,16,15,14,14,13,12,11,11,10,9,9
                db 8,8,7,7,7,6,6,5,5,5,4,4
                db 4,4,3,3,3,3,3,2,2,2,2,2
                db 2,2,1,1,1,1,1,1,1,1,1,1
                
                ELSEIFDEF TANDY

value_l         db 7,7,9,14,6,0,12,10,10,8,0,9
                db 7,7,9,14,6,0,12,10,10,8,0,9
                db 7,7,9,14,6,0,12,10,10,8,0,9
                db 7,7,9,14,6,0,12,10,10,12,15,5
                db 11,3,12,7,3,0,14,13,13,14,15,2
                db 5,9,14,3,9,0,7,14,6,15,7,1
                db 10,4,15,9,4,0,11,7,3,15,11,8
                db 5,2,15,12,10,8,5,3,1,15,13,12

value_h         db 53,50,47,44,42,40,37,35,33,63,60,56
                db 53,50,47,44,42,40,37,35,33,63,60,56
                db 53,50,47,44,42,40,37,35,33,63,60,56
                db 53,50,47,44,42,40,37,35,33,31,29,28
                db 26,25,23,22,21,20,18,17,16,15,14,14
                db 13,12,11,11,10,10,9,8,8,7,7,7
                db 6,6,5,5,5,5,4,4,4,3,3,3
                db 3,3,2,2,2,2,2,2,2,1,1,1

atten           db 15,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13
                db 12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11
                db 10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9
                db 8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7
                db 6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5
                db 4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3
                db 2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

atten_reg       db 090h,0b0h,0d0h
part_reg        db 080h,0a0h,0c0h

                ENDIF
                             
vel_graph       db 82,85,88,91,94,97,100,103,106,109,112,115,118,121,124,127

                ;
                ;Misc. data
                ;

S_status        db NUM_SLOTS dup (?)  ;2=key on, 1=key off, 0=slot free
S_voice         db NUM_SLOTS dup (?)  ;Voice # or -1 assigned to slot
S_channel       db NUM_SLOTS dup (?)  ;MIDI channel owning slot
S_note          db NUM_SLOTS dup (?)  ;MIDI note # for slot's voice
S_keynum        db NUM_SLOTS dup (?)  ;MIDI key # before RBS translation
S_transpose     db NUM_SLOTS dup (?)  ;MIDI note transposition for slot
S_velocity      db NUM_SLOTS dup (?)  ;keyon velocity for note
S_sustain       db NUM_SLOTS dup (?)  ;note sustained if nonzero
S_update        db NUM_SLOTS dup (?)  ;bit mask for register updates

S_BLOCK         db NUM_SLOTS dup (?)  ;KON/BLOCK values
S_atten         db NUM_SLOTS dup (?)  ;Tandy attenuation register setup

S_p_val         dw NUM_SLOTS dup (?)
S_mod_value     dw NUM_SLOTS dup (?)  ;instantaneous pitch  for mod emulation

FREE            equ 0                 ;S_status[] phase equates
KEYON           equ 1
KEYOFF          equ 2

U_ALL_REGS      equ 01000001b         ;Bit mask equates for S_update
U_KSLTL         equ 01000000b
U_FREQ          equ 00000001b

MIDI_vol        db NUM_CHANS dup (?)  ;volume 
MIDI_pitch_l    db NUM_CHANS dup (?)  ;pitchwheel LSB
MIDI_pitch_h    db NUM_CHANS dup (?)  ;pitchwheel MSB
MIDI_express    db NUM_CHANS dup (?)  ;expression 
MIDI_sus        db NUM_CHANS dup (?)  ;HOLD1 pedal 
MIDI_mod        db NUM_CHANS dup (?)  ;modulation
MIDI_vprot      db NUM_CHANS dup (?)  ;voice protection

MIDI_voices     db NUM_CHANS dup (?)  ;# of voices assigned to channel

V_channel       db NUM_VOICES dup (?) ;voice assigned to MIDI channel n or -1

S_V_priority    dw NUM_SLOTS dup (?)  ;adjusted voice priorities

MOD_accum       dw ?                  ;DDA accumulator for modulation timing
MOD_sign        dw ?                  ;sign (+/-) for modulation

last_val_l      db ?
last_val_h      db ?

;****************************************************************************
;*                                                                          *
;* I/O routines                                                             *
;*                                                                          *
;****************************************************************************

;****************************************************************************
detect_device   PROC\
                USES ebx esi edi\		;Attempt to detect hardware
                ,H,IO_ADDR,IRQ,DMA,DRQ

                pushfd
                cli
                cld

                IFDEF IBMPC

                mov eax,1               ;IBM speaker always present

                ELSEIFDEF TANDY

                mov eax,1

                ENDIF

__exit:         POP_F
                ret
detect_device   ENDP

;****************************************************************************
reset_synth     PROC\                   ;Init hardware regs
                USES ebx esi edi

                pushfd                  
                cli

                IFDEF IBMPC

                in al,61h
                and al,11111100b
                out 61h,al              ;turn the speaker off

                ELSEIFDEF TANDY
                        
                mov al,9fh              ;turn off any active sounds
                out 0c0h,al
                mov al,0bfh
                out 0c0h,al
                mov al,0dfh
                out 0c0h,al

                mov al,68h              ;enable internal speaker
                out 61h,al

                ENDIF

                POP_F
                ret
reset_synth     ENDP

;****************************************************************************
;*                                                                          *
;*  Timbre cache management / related API calls                             *
;*                                                                          *
;****************************************************************************

;****************************************************************************
protect_timbre  PROC H,Bank,Num

                ret
protect_timbre  ENDP

;****************************************************************************
unprotect_timbre PROC H,Bank,Num            

                ret
unprotect_timbre ENDP

;****************************************************************************
timbre_status   PROC\
                USES ebx esi edi\          ;Return 0 if timbre not resident
                ,H,Bank,Num

                pushfd 
                cli

                mov eax,1               ;(no timbres needed)

                POP_F
                ret
timbre_status   ENDP

;****************************************************************************
get_cache_size  PROC\
                USES ebx esi edi\
                ,H

                pushfd
                cli

                mov eax,DEF_TC_SIZE

                POP_F
                ret
get_cache_size  ENDP

;****************************************************************************
define_cache    PROC H,Address:PTR,C_Size

                ret
define_cache    ENDP

;****************************************************************************
get_request     PROC\
                USES ebx esi edi\
                ,H,Sequence

                pushfd
                cli

                mov eax,-1
                
                POP_F
                ret
get_request     ENDP

;****************************************************************************
install_timbre  PROC H,Bank,Num,Address:PTR

                ret
install_timbre  ENDP

;****************************************************************************
;*                                                                          *
;*  MIDI interpreter and related procedures                                 *
;*                                                                          *
;****************************************************************************

                IF EMULATE_MOD
serve_synth     PROC\                   ;Update simulated modulation
                USES ebx esi edi           ;Called once per quantization interval

                add MOD_accum,MOD_RATE
                cmp MOD_accum,QUANT_RATE
                jae __do_M_period
                ret

__do_M_period:  sub MOD_accum,QUANT_RATE

                xor MOD_sign,1

                mov esi,0               ;mark appropriate voice parameters
__flag_slot:    cmp S_status[esi],FREE  ;as "changed"
                je __flag_next
                movzx ebx,S_channel[esi]
                mov al,MOD_DEPTH
                mul MIDI_mod[ebx]
                cmp MOD_sign,0
                je __store
                neg ax

__store:        mov ebx,esi
                mov S_mod_value[esi][ebx],ax
                or S_update[esi],U_FREQ
                invoke update_voice,esi ;update the hardware registers
__flag_next:    inc esi
                cmp esi,NUM_SLOTS
                jne __flag_slot

                ret
serve_synth     ENDP
                ENDIF

;****************************************************************************
init_synth      PROC\                   ;Init MIDI synthesizer emulation
                USES ebx esi edi
                pushfd           
                cli
                cld

                mov edi,0
__init_chans:   mov MIDI_voices[edi],0
                inc edi
                cmp edi,NUM_CHANS
                jne __init_chans

                mov edi,0
__init_slots:   mov S_status[edi],FREE
                mov S_mod_value[edi*2],0
                inc edi
                cmp edi,NUM_SLOTS
                jne __init_slots

                mov edi,0
__init_voices:  mov V_channel[edi],-1
                inc edi
                cmp edi,NUM_VOICES
                jne __init_voices

                mov MOD_accum,0
                mov MOD_sign,0

                mov last_val_l,-1
                mov last_val_h,-1

                POP_F
                ret
init_synth      ENDP

;****************************************************************************
assign_voice    PROC\
                USES ebx esi edi\
                ,Slot                  ;Allocate hardware voice to slot
                
                mov ebx,-1             ;try to find an unassigned voice
__search_free:  inc ebx
                cmp ebx,NUM_VOICES
                je __seize_voice       ;(# of active slots > # of voices)
                cmp V_channel[ebx],-1   
                jne __search_free

                mov esi,[Slot]         ;found free voice, assign it to slot
                mov S_voice[esi],bl
                mov edi,ebx
                mov bl,S_channel[esi]
                inc MIDI_voices[ebx]
                mov V_channel[edi],bl

                mov S_update[esi],U_ALL_REGS
                invoke update_voice,esi;update the hardware
                ret

__seize_voice:  call update_priority   ;assign voice based on priority search
                ret                     
assign_voice 	ENDP

;****************************************************************************
release_voice   PROC\
                USES ebx esi edi\
                ,Slot                       ;Release slot's voice

                mov esi,[Slot]

                cmp S_voice[esi],-1   
                je __exit            

                and S_BLOCK[esi],11011111b
                or S_update[esi],U_FREQ;force KON = 0...

                invoke update_voice,esi     ;...silence any note...

                movzx ebx,S_channel[esi]    ;...and deallocate the voice
                dec MIDI_voices[ebx]
                mov bl,S_voice[esi]
                mov V_channel[ebx],-1

                mov S_voice[esi],-1

__exit:         ret
release_voice   ENDP
                    
;****************************************************************************
VAL_SHIFT       MACRO
                IFDEF TANDY

                movzx ecx,ah
                shl ecx,4
                or cl,al
                mov eax,ecx

                ENDIF
                ENDM

RECOMBINE       MACRO
                IFDEF TANDY

                mov ecx,eax
                shr ecx,4
                mov ah,cl
                and al,0fh

                ENDIF
                ENDM

update_voice    PROC\
                USES ebx esi edi\
                ,Slot                  ;Update hardware regs for slot
                LOCAL voice,vol:BYTE
                LOCAL f_num
                LOCAL val_l:BYTE,val_h:BYTE
                LOCAL pitch:WORD,notenum:WORD,mid_val:WORD
                LOCAL MOD_value:WORD
                OPTION NOLJMP

                mov esi,[Slot]         ;update only requested parameters for
                                       ;speed
                movzx ebx,S_voice[esi]
                cmp bl,-1               
                je __exit              ;no hardware voice assigned, exit

                and eax,0ffh
                mov voice,ebx          ;voice #

                test S_update[esi],U_KSLTL
                jnz __go_KSLTL
__KSLTL_done:   test S_update[esi],U_FREQ
                jnz __go_FREQ           ;(update FREQ's KeyOn bit last)
__FREQ_done:
__exit:         ret

__go_KSLTL:     jmp __KSLTL
__go_FREQ:      jmp __FREQ

;----------------------------------------------------------------------------
__KSLTL:        IFDEF IBMPC

                ELSEIFDEF TANDY

                movzx edi,S_channel[esi]
                and edi,0fh             ;DI = MIDI channel

                mov al,MIDI_vol[edi]
                mul MIDI_express[edi]
                shl eax,1               ;(AX*2)/256 = AX/128 ї AX/127
                mov al,ah
                cmp al,1
                sbb al,-1               ;(error = 1/127 units; round up if !0)
                mul S_velocity[esi]
                shl eax,1
                mov al,ah
                cmp al,1
                sbb al,-1               ;(error = 1/127 units; round up if !0)

                and eax,0ffh
                mov ebx,eax             ;BX=composite (vol+expression) volume

                mov al,atten[ebx]
                mov S_atten[esi],al

                or S_update[esi],U_FREQ ;signal need to update keyon volume

                ENDIF

                and S_update[esi],NOT U_KSLTL
                jmp __KSLTL_done

;----------------------------------------------------------------------------
__FREQ:         test S_BLOCK[esi],00100000b
                jnz __key_on            ;KON bit = 1; turn note on

                IFDEF IBMPC

                in al,61h               ;turn IBM speaker off
                and al,11111100b
                out 61h,al

                ELSEIFDEF TANDY

                mov ebx,voice
                mov al,atten_reg[ebx]
                or al,0fh               ;set maximum Tandy attenuation
                out 0c0h,al

                ENDIF
                jmp __end_freq

__key_on:       movzx ebx,S_channel[esi];get pitch bend word value
                movzx eax,MIDI_pitch_h[ebx]
                shl ax,7
                or al,MIDI_pitch_l[ebx]
                mov pitch,ax            ;store it

                movzx ebx,S_note[esi]   ;get key # 12-108
                mov al,S_transpose[esi]
                cbw
                add bx,ax
                sub bx,24              ;normalize to 0-95
__norm_1:       add bx,12
                cmp bx,0
                jl __norm_1
                add bx,12
__norm_2:       sub bx,12
                cmp bx,95
                jg __norm_2
                mov notenum,bx         ;store it

                IF LINEAR_PITCH

                push esi
                push edi

                shl esi,1
                movzx eax,S_mod_value[esi]
                mov MOD_value,ax

                movzx ebx,notenum       ;get upper and lower note value bounds
                sub ebx,DEF_PITCH_RANGE
                jg __lin_1
                mov ebx,0
__lin_1:        movzx eax,value_l[ebx]
                mov ah,value_h[ebx]
                VAL_SHIFT
                mov esi,eax             ;SI=low value

                movzx ebx,notenum
                add bx,DEF_PITCH_RANGE
                cmp bx,95
                jb __lin_2
                mov bx,95
__lin_2:        movzx eax,value_l[ebx]
                mov ah,value_h[ebx]
                VAL_SHIFT
                mov edi,eax             ;DI=high value

                movzx ebx,notenum
                movzx eax,value_l[ebx]
                mov ah,value_h[ebx]      
                VAL_SHIFT
                mov edx,eax             ;DX=0 value
                mov mid_val,dx          

                movzx eax,pitch
                sub ax,2000h            ;E000=min 0=none 1FFF=max
                add ax,MOD_value
                cmp ax,0
                jl __neg
                sub di,dx
                mov dx,di               ;positive bend: DX = maximum 
                jmp __set
__neg:          sub dx,si               ;negative bend: DX = maximum 

__set:          imul dx
                mov ecx,2000h
                idiv cx

                add ax,mid_val
                RECOMBINE
                mov val_l,al
                mov val_h,ah

                pop edi
                pop esi

                ELSE
                movzx eax,pitch
                sub ax,1fffh

                sar ax,5                ;divide by 0x20, preserving sign
                                        ;(range now +0x100 to -0x0FF)

                movzx ecx,DEF_PITCH_RANGE
                imul cx                 ;normally 12 (+0xc00 to -0xBF4)

                add ah,BYTE PTR notenum ;add computed note offset * 256
                add ax,8                ;add 1/32 to round to 1/16

                sar ax,4                ;derive true note #
                                        ;(expressed in 1/16 halftones)

                sub ax,(12*16)
__norm_3:       add ax,(12*16)
                cmp ax,0
                jl __norm_3
                add ax,(12*16)
__norm_4:       sub ax,(12*16)
                cmp ax,(96*16)-1
                jg __norm_4

                shr ax,4
                movzx ebx,ax          

                mov al,value_l[ebx]
                mov val_l,al
                mov al,value_h[ebx]
                mov val_h,al

                ENDIF                   ;LINEAR_PITCH

                IFDEF IBMPC

                movzx ebx,val_l         ;avoid redundant timer resets to
                mov bh,val_h            ;reduce clicks
                cmp bl,last_val_l
                jne __set_freq
                cmp bh,last_val_h
                je __on

__set_freq:     mov al,0b6h             ;program speaker frequency on Timer 2
                out 43h,al
                jmp $+2
                mov al,bl
                out 42h,al
                jmp $+2
                mov al,bh
                out 42h,al
                jmp $+2
                mov last_val_l,bl
                mov last_val_h,bh
__on:           in al,61h
                or al,00000011b
                out 61h,al              ;turn the PC speaker on

                ELSEIFDEF TANDY

                mov edi,voice           ;set Tandy freq, keyon level
                mov al,part_reg[edi]
                or al,val_l
                out 0c0h,al
                jmp $+2
                mov al,val_h
                out 0c0h,al
                jmp $+2
                mov al,atten_reg[edi]
                or al,S_atten[esi]
                out 0c0h,al

                ENDIF                   ;TANDY

__end_freq:     and S_update[esi],NOT U_FREQ
                jmp __FREQ_done

                OPTION LJMP
update_voice    ENDP

;****************************************************************************
update_priority PROC\                   ;Maintain synthesizer voice priority
                USES ebx esi edi
                LOCAL vcount,low_p,high_p

                mov edx,0               ;zero active slot count

                mov esi,-1              
__get_priority: inc esi                 ;build adjusted priority table and
                cmp esi,NUM_SLOTS       ;reallocate voices if necessary
                je __chk_demand
                cmp S_status[esi],FREE
                je __get_priority

                inc edx                 ;slot active, bump count
                movzx edi,BYTE PTR S_channel[esi]
                and edi,0fh             ;DI = slot's MIDI channel
                mov eax,0ffffh           
                cmp MIDI_vprot[edi],64  ;priority = max if voice protection on
                jge __adj_priority       
                mov ax,S_p_val[esi*2]  
__adj_priority: movzx ecx,MIDI_voices[edi]  ;AX = slot's base priority        
                sub eax,ecx             ;priority -= # of voices in channel
                jnc __set_priority
                mov eax,0
__set_priority: mov S_V_priority[esi*2],ax
                jmp __get_priority

__chk_demand:   cmp edx,NUM_VOICES      ;more active slots than voices?
                jbe __exit              ;no, exit

                mov vcount,edx

__sort_p_list:  mov eax,0               ;set AX = unvoiced highest priority                
                mov edx,0ffffh          ;set DX = voiced lowest priority
                mov esi,-1
__for_slot:     inc esi                  
                cmp esi,NUM_SLOTS
                je __reassign
                cmp S_status[esi],FREE
                je __for_slot
                movzx edi,WORD PTR S_V_priority[esi*2]
                cmp S_voice[esi],-1      
                jne __chk_low           
                cmp edi,eax               
                jb __for_slot         
                mov eax,edi              
                mov high_p,esi          ;highest-priority unvoiced slot index
                jmp __for_slot
__chk_low:      cmp edi,edx               
                ja __for_slot
                mov edx,edi               
                mov low_p,esi           ;lowest-priority voiced slot index
                jmp __for_slot

__reassign:     cmp eax,edx             ;highest unvoiced < lowest voiced?
                jb __exit               ;yes, we're done

                mov esi,low_p
                movzx ebx,S_voice[esi]    

                push ebx
                invoke release_voice,esi
                mov S_status[esi],FREE  ;release any slots deallocated...
                pop ebx

                mov esi,high_p
                mov S_voice[esi],bl
                mov edi,ebx
                mov bl,S_channel[esi]
                inc MIDI_voices[ebx]
                mov V_channel[edi],bl

                mov S_update[esi],U_ALL_REGS
                invoke update_voice,esi  ;update the hardware

                dec vcount
                jnz __sort_p_list       ;keep sorting until priorities valid

__exit:         ret
update_priority ENDP


;****************************************************************************
slot_phase      PROC\
                USES ebx esi edi\
                ,Slot                       ;Set up slot parameters

                mov esi,[Slot]
                
                mov S_BLOCK[esi],00100000b  ;set KON, clear BLOCK mask

                mov S_p_val[esi*2],32767    ;instrument priority = average

                mov S_update[esi],U_ALL_REGS;flag all registers "dirty"

                ret
slot_phase 	  ENDP

;****************************************************************************
note_off        PROC\
                USES ebx esi edi\               ;Turn MIDI note off
                ,Chan,Note

                mov esi,-1                  ;find all slots in which note is
__next_slot:    mov al,BYTE PTR [Note]      ;playing
                mov bl,BYTE PTR [Chan]
__find_note:    inc esi
                cmp esi,NUM_SLOTS
                je __exit
                cmp S_status[esi],KEYON  
                jne __find_note
                cmp S_keynum[esi],al
                jne __find_note
                cmp S_channel[esi],bl
                jne __find_note

                and ebx,0ffh
                cmp MIDI_sus[ebx],64
                jge __sustained
                
                invoke release_voice,esi    ;release the slot's voice
                mov S_status[esi],FREE
                jmp __next_slot

__sustained:    mov S_sustain[esi],1
                jmp __next_slot

__exit:         ret

note_off 	  ENDP

;****************************************************************************
note_on         PROC\
                USES ebx esi edi\
                ,Chan,Note,Velocity ;Turn MIDI note on

                mov edi,[Chan]     

                mov esi,0
__find_slot:    cmp S_status[esi],FREE
                je __slot_found
                inc esi                 ;find a free virtual voice slot
                cmp esi,NUM_SLOTS
                jne __find_slot
                jmp __exit              ;exit if no virtual voice available

__slot_found:   mov eax,[Chan]          ;establish MIDI channel
                mov S_channel[esi],al    

                mov edx,[Note]
                mov S_keynum[esi],dl    ;save MIDI key #

                mov S_note[esi],dl
                mov S_transpose[esi],0  ;(no transposition)

                IF VEL_SENS             
                mov eax,[Velocity]      ;establish note velocity
                IF NOT VEL_TRUE              
                shr al,3
                mov ebx,OFFSET vel_graph       ;scale back velocity sensitivity to 
                xlat cs:[ebx]           ;reduce perceived playback noise
                ENDIF
                ELSE
                mov al,127              ;default velocity = 127
                ENDIF
                mov S_velocity[esi],al

                mov S_status[esi],KEYON ;flag note "on" in slot

                mov S_sustain[esi],0    ;init sustained flag

                invoke slot_phase,esi   ;set up BNK timbre in slot

                mov S_voice[esi],-1

                invoke assign_voice,esi ;assign hardware voice to slot

__exit:         ret
note_on 	ENDP

;****************************************************************************
release_sustain PROC\
                USES ebx esi edi\
                ,Chan

                mov esi,0                
__release_sus:  cmp S_status[esi],FREE   
                je __next_sus
                mov al,BYTE PTR [Chan]
                cmp S_channel[esi],al
                jne __next_sus
                cmp S_sustain[esi],0
                je __next_sus
                invoke note_off,edi,DWORD PTR S_note[esi]
__next_sus:     inc esi
                cmp esi,NUM_SLOTS
                jne __release_sus

                ret
release_sustain ENDP

;****************************************************************************
send_MIDI_message PROC\
                USES ebx esi edi\          ;Send MIDI Channel Voice message
                Stat,D1,D2

                mov esi,[D1]
                and esi,0ffh            ;SI=data 1 / controller #
                mov edi,[Stat]
                mov eax,edi               
                and edi,00fh            ;DI=channel
                and eax,0f0h            ;AX=status
                movzx ecx,BYTE PTR [D2] ;CX=data byte 2

                cmp eax,0b0h             
                je __ctrl_change
                cmp eax,0e0h
                je __pitch
                cmp eax,080h
                je __note_off
                cmp eax,090h
                jne __exit

                cmp edi,MIN_TRUE_CHAN-1
                jb __exit
                cmp edi,MAX_REC_CHAN-1
                ja __exit

                jecxz __note_off        ;implicit Note Off if velocity==0

                invoke note_on,edi,esi,ecx
                ret

__note_off:     invoke note_off,edi,esi
__exit:         ret

__pitch:        mov eax,esi
                mov MIDI_pitch_l[edi],al
                mov MIDI_pitch_h[edi],cl
                mov al,U_FREQ
                jmp __flag_updates

__ctrl_change:  cmp esi,VOICE_PROTECT
                je __vprot
                jmp __MIDI

__vprot:        mov MIDI_vprot[edi],cl
                jmp __exit

__MIDI:         mov al,0
                mov ebx,OFFSET MIDI_mod
                cmp esi,MODULATION
                je __MIDI_set
                mov al,U_KSLTL          ;Emulate MIDI controllers
                mov ebx,OFFSET MIDI_vol
                cmp esi,PART_VOLUME
                je __MIDI_set
                mov ebx,OFFSET MIDI_express
                cmp esi,EXPRESSION
                je __MIDI_set

                cmp esi,SUSTAIN
                je __MIDI_sus
                cmp esi,RESET_ALL_CTRLS
                je __MIDI_rac
                cmp esi,ALL_NOTES_OFF
                je __MIDI_ano
                jmp __exit             

__MIDI_set:     mov [ebx][edi],cl       ;save shadowed controller value
                
__flag_updates: mov ebx,edi
                mov esi,0               ;mark appropriate voice parameters
__flag_slot:    cmp S_status[esi],FREE  ;as "changed"
                je __flag_next
                cmp S_channel[esi],bl
                jne __flag_next
                or S_update[esi],al
                push eax
                push ebx
                invoke update_voice,esi ;update the hardware registers
                pop ebx
                pop eax
__flag_next:    inc esi
                cmp esi,NUM_SLOTS
                jne __flag_slot
                jmp __exit

__MIDI_sus:     mov MIDI_sus[edi],cl    ;log sustain value
                cmp cl,64               ;releasing sustain controller?
                jge __exit
                                          
                invoke release_sustain,edi
                jmp __exit              ;yes, turn off any sustained notes 

__MIDI_ano:     mov esi,0               ;turn off all notes playing in channel
__chk_note:     cmp S_status[esi],KEYON
                jne __next_ano
                mov ebx,edi
                cmp S_channel[esi],bl
                jne __next_ano
                invoke note_off,edi,DWORD PTR S_note[esi]
__next_ano:     inc esi
                cmp esi,NUM_SLOTS
                jne __chk_note
                jmp __exit

__MIDI_rac:     mov MIDI_sus[edi],0
                invoke release_sustain,edi
                mov MIDI_mod[edi],0
                mov MIDI_express[edi],127   ;emulate Roland LAPC-1 RAC message
                mov MIDI_pitch_l[edi],DEF_PITCH_L
                mov MIDI_pitch_h[edi],DEF_PITCH_H
                mov al,U_KSLTL OR U_FREQ
                jmp __flag_updates

send_MIDI_message ENDP

;****************************************************************************
;*                                                                          *
;*  Miscellaneous public (API-accessible) procedures                        *
;*                                                                          *
;****************************************************************************

describe_driver PROC\
                USES ebx esi edi\           ;Return ptr to DDT
                ,H,IntRateProc:PTR
                pushfd
                cli

                mov eax,OFFSET DDT

                POP_F
                ret
describe_driver ENDP

;****************************************************************************
send_cv_msg     PROC\
                USES ebx esi edi\
                ,H,Stat,D1,D2       ;Send an explicit Channel Voice msg
                pushfd
                cli

                invoke send_MIDI_message,[Stat],[D1],[D2]

                POP_F
                ret
send_cv_msg 	ENDP

;****************************************************************************
send_sysex_msg  PROC H,AddrA,AddrB,AddrC,Data:PTR,C_Size,C_Wait

                ret

send_sysex_msg  ENDP

;****************************************************************************
write_display   PROC H,String:PTR      ;Write string to display (unless NULL)

                ret

write_display   ENDP



