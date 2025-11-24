;ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
;лл                                                                         лл
;лл  YAMAHA32.INC                                                           лл
;лл                                                                         лл
;лл  IBM Audio Interface Library XMIDI interpreter for Yamaha OPL2 & OPL3   лл
;лл                                                                         лл
;лл  Version 1.00 of 29-Jul-92: 32-bit conversion (Rational Systems DOS/4G) лл
;лл          1.01 of  1-May-93: Flashtek X32 compatibility added            лл
;лл          1.02 of 17-Jun-93: PAS detection restored                      лл
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


release_voice   PROTO C,Slot:DWORD	;Release slot's voice
update_voice    PROTO C,Slot:DWORD	;Update hardware regs for slot
BNK_phase       PROTO C,Slot:DWORD	;Set up BNK slot parameters

                ;
                ;Driver-specific configuration equates
                ;

OSI_ALE         equ FALSE               ;TRUE to assemble OSI TVFX code

MAX_REC_CHAN    equ 10                  ;Max channel recognized by synths
MAX_TRUE_CHAN   equ 9                   ;Max channel available for locking
MIN_TRUE_CHAN   equ 2                   ;Min channel # (1-based)

DEF_SYNTH_VOL   equ 100                 ;Init vol=100%
MAX_TIMBS       equ 192                 ;Max # of timbres in local cache
DEF_TC_SIZE     equ 3584                ;Room for 256 14-byte .BNK timbres

                IFDEF YM3812
NUM_VOICES      equ 9                   ;# of physical voices available
NUM_SLOTS       equ 16                  ;# of virtual voices available
                ELSEIFDEF YMF262
NUM_VOICES      equ 18                  ;# of physical voices available
NUM_4OP_VOICES  equ 6
NUM_SLOTS       equ 20                  ;# of virtual voices available
                ENDIF

VEL_SENS        equ 1                   ;Velocity sensitivity disabled if 0

VEL_TRUE        equ 0                   ;Full velocity sensitivity range if 1
                                        ;(set to 0 to reduce playback noise)
DEF_PITCH_RANGE equ 12                  ;Default pitch wheel range (semitones)

DEF_AV_DEPTH    equ 11000000b           ;Default AM/Vibrato depth
                                        ;bit 7: AM depth 4.8dB if 1, else 1dB
                                        ;bit 6: VIB depth 14c. if 1, else 7c.

                IFDEF YMF262            ;Panpot thresholds for OPL3 voices

R_PAN_THRESH    equ 27                  ;Force right channel if pan <= n
L_PAN_THRESH    equ 100                 ;Force left channel if pan >= n

                IFDEF SBPRO2
LEFT_MASK       equ 11011111b
RIGHT_MASK      equ 11101111b
                ELSEIFDEF PASOPL
LEFT_MASK       equ 11011111b
RIGHT_MASK      equ 11101111b
                ELSE
LEFT_MASK       equ 11101111b
RIGHT_MASK      equ 11011111b
                ENDIF

                ENDIF

CIRC_ASSIGN     equ TRUE                ;FALSE for "old" AIL voice assignment

                ;
                ;Internal equates
                ;

                IFDEF SBPRO1
SBPRO           equ 1
                ENDIF

                IFDEF SBPRO2
SBPRO           equ 1
                ENDIF

                IFDEF PAS
PROAUDIO        equ 1
                ENDIF

                IFDEF PASOPL
PROAUDIO        equ 1
                ENDIF

                ;
                ;Driver Description Table (DDT)
                ;Returned by describe_driver() proc
                ;

DDT             LABEL WORD
min_API_version dd 200                  ;Minimum API version required = 2.00
drvr_type       dd 3                    ;Type 3: XMIDI emulation
data_suffix     LABEL BYTE
                IFDEF YMF262
                db 'OPL',0              ;Supports .OPL Global Timbre file
                ELSE                    ;(backward-compatible w/.AD format)
                db 'AD',0,0             ;Needs .AD Global Timbre file
                ENDIF
device_name_o   dd OFFSET devnames      ;Pointer to list of supported devices
default_IO      LABEL WORD              ;Factory default I/O parameters
                IFDEF ADLIBSTD
                dd 388h
                ELSEIFDEF SBSTD
                dd 220h
                ELSEIFDEF PROAUDIO
                dd -1
                ELSEIFDEF SBPRO1
                dd 220h
                ELSEIFDEF SBPRO2
                dd 220h
                ELSEIFDEF ADLIBG
                dd 388h
                ENDIF
default_IRQ     dd -1
default_DMA     dd -1
default_DRQ     dd -1
service_rate    dd QUANT_RATE           ;Request 120 calls/second 
display_size    dd 0                    ;No display

devnames        LABEL BYTE
                IFDEF ADLIBSTD
                db 'Ad Lib(R) Music Synthesizer Card',0
                ELSEIFDEF SBSTD
                db 'Creative Labs Sound Blaster(TM) FM Sound',0
                db 'Media Vision Thunderboard(TM) FM Sound',0
                ELSEIFDEF PAS
                db 'Media Vision Pro Audio Spectrum(TM) 8 FM Sound',0
                ELSEIFDEF PASOPL
                db 'Media Vision Pro Audio Spectrum(TM) Plus/16 FM Sound',0
                ELSEIFDEF SBPRO
                db 'Creative Labs Sound Blaster Pro(TM) FM Sound',0
                ELSEIFDEF ADLIBG
                db 'Ad Lib(R) Gold Music Synthesizer Card',0
                ENDIF
                db 0                    ;0 to end list of device names

                IFDEF YM3812
IOWVAL          equ 42
                IFDEF STEREO
STEREO_3812     equ 1
                ENDIF
                ELSEIFDEF YMF262
IOWVAL          equ 8
                ENDIF

                IF OSI_ALE
                INCLUDE ale32.inc
                ENDIF

                ;
                ;Default setup values & internal constants
                ;

freq_table	dw 02b2h,02b4h,02b7h,02b9h,02bch,02beh,02c1h,02c3h,02c6h,02c9h
	dw 02cbh,02ceh,02d0h,02d3h,02d6h,02d8h,02dbh,02ddh,02e0h,02e3h 
	dw 02e5h,02e8h,02ebh,02edh,02f0h,02f3h,02f6h,02f8h,02fbh,02feh 
	dw 0301h,0303h,0306h,0309h,030ch,030fh,0311h,0314h,0317h,031ah 
	dw 031dh,0320h,0323h,0326h,0329h,032bh,032eh,0331h,0334h,0337h 
	dw 033ah,033dh,0340h,0343h,0346h,0349h,034ch,034fh,0352h,0356h 
	dw 0359h,035ch,035fh,0362h,0365h,0368h,036bh,036fh,0372h,0375h 
	dw 0378h,037bh,037fh,0382h,0385h,0388h,038ch,038fh,0392h,0395h 
	dw 0399h,039ch,039fh,03a3h,03a6h,03a9h,03adh,03b0h,03b4h,03b7h 
	dw 03bbh,03beh,03c1h,03c5h,03c8h,03cch,03cfh,03d3h,03d7h,03dah 
	dw 03deh,03e1h,03e5h,03e8h,03ech,03f0h,03f3h,03f7h,03fbh,03feh 
	dw 0fe01h,0fe03h,0fe05h,0fe07h,0fe08h,0fe0ah,0fe0ch,0fe0eh,0fe10h,0fe12h
	dw 0fe14h,0fe16h,0fe18h,0fe1ah,0fe1ch,0fe1eh,0fe20h,0fe21h,0fe23h,0fe25h 
	dw 0fe27h,0fe29h,0fe2bh,0fe2dh,0fe2fh,0fe31h,0fe34h,0fe36h,0fe38h,0fe3ah 
	dw 0fe3ch,0fe3eh,0fe40h,0fe42h,0fe44h,0fe46h,0fe48h,0fe4ah,0fe4ch,0fe4fh 
	dw 0fe51h,0fe53h,0fe55h,0fe57h,0fe59h,0fe5ch,0fe5eh,0fe60h,0fe62h,0fe64h 
	dw 0fe67h,0fe69h,0fe6bh,0fe6dh,0fe6fh,0fe72h,0fe74h,0fe76h,0fe79h,0fe7bh 
	dw 0fe7dh,0fe7fh,0fe82h,0fe84h,0fe86h,0fe89h,0fe8bh,0fe8dh,0fe90h,0fe92h 
	dw 0fe95h,0fe97h,0fe99h,0fe9ch,0fe9eh,0fea1h,0fea3h,0fea5h,0fea8h,0feaah 
	dw 0feadh,0feafh 

note_octave	db 0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1
	db 1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2 
	db 2,2,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4 
	db 4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5 
	db 5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,7 
	db 7,7,7,7,7,7,7,7,7,7,7

note_halftone	db 0,1,2,3,4,5,6,7,8,9,10,11,0,1,2,3,4
	db 5,6,7,8,9,10,11,0,1,2,3,4,5,6,7,8,9
	db 10,11,0,1,2,3,4,5,6,7,8,9,10,11,0,1,2
	db 3,4,5,6,7,8,9,10,11,0,1,2,3,4,5,6,7
	db 8,9,10,11,0,1,2,3,4,5,6,7,8,9,10,11,0
	db 1,2,3,4,5,6,7,8,9,10,11

array0_init     db 00100000b,0,0,01100000b,0,0,0,0,0,0,0,0,0,0,0   ;01-0f
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;10-1f
                db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1                 ;20-2f
                db 1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0                 ;30-3f
                db 63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63 ;40-4f
                db 63,63,63,63,63,63,0,0,0,0,0,0,0,0,0,0           ;50-5f
                db 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
                db 255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0     ;70-7f
                db 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 ;80-8f
                db 15,15,15,15,15,15,0,0,0,0,0,0,0,0,0,0           ;90-9f
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;a0-af
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,DEF_AV_DEPTH,0,0      ;b0-bf
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;c0-cf
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;d0-df
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;e0-ef
                db 0,0,0,0,0,0                                     ;f0-f5

array1_init     db 0,0,0,0,00000001b,0,0,0,0,0,0,0,0,0,0           ;01-0f
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;10-1f
                db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1                 ;20-2f
                db 1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0                 ;30-3f
                db 63,63,63,63,63,63,63,63,63,63,63,63,63,63,63,63 ;40-4f
                db 63,63,63,63,63,63,0,0,0,0,0,0,0,0,0,0           ;50-5f
                db 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
                db 255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0     ;70-7f
                db 15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15 ;80-8f
                db 15,15,15,15,15,15,0,0,0,0,0,0,0,0,0,0           ;90-9f
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;a0-af
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;b0-bf
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;c0-cf
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;d0-df
                db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0                 ;e0-ef
                db 0,0,0,0,0,0                                     ;f0-f5
                             
vel_graph       db 82,85,88,91,94,97,100,103,106,109,112,115,118,121,124,127

                IFDEF STEREO_3812
pan_graph       db 0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30                      
                db 32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62                
                db 64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94                
                db 96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,127
                db 127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127
                db 127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127
                db 127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127
                db 127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127
                ENDIF

op_0            db 0,1,2,6,7,8,12,13,14,18,19,20,24,25,26,30,31,32
op_1            db 3,4,5,9,10,11,15,16,17,21,22,23,27,28,29,33,34,35

op_index        db 0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21
                db 0,1,2,3,4,5,8,9,10,11,12,13,16,17,18,19,20,21

op_array        db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
                db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1

voice_num       db 0,1,2,3,4,5,6,7,8,0,1,2,3,4,5,6,7,8

voice_array     db 0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1

op4_base        db 1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0
alt_voice       db 3,4,5,0,1,2,-1,-1,-1,12,13,14,9,10,11,-1,-1,-1

                IFDEF YMF262
alt_op_0        db 6,7,8,0,1,2,-1,-1,-1,24,25,26,18,19,20,-1,-1,-1
alt_op_1        db 9,10,11,3,4,5,-1,-1,-1,27,28,29,21,22,23,-1,-1,-1
conn_sel        db 1,2,4,1,2,4,0,0,0,8,16,32,8,16,32,0,0,0

op4_voice       db 0,1,2,9,10,11

carrier_01      db 00b,01b,10b,01b
carrier_23      db 10b,10b,10b,11b
                ENDIF

                ;
                ;Misc. data
                ;

                IFDEF STEREO_3812
LFMADDR 	dd ?	
LFMDATA 	dd ?	
RFMADDR 	dd ?	
RFMDATA 	dd ?	
                ENDIF

                IFDEF SBPRO
MIXADDR         dd ?
MIXDATA         dd ?
                ENDIF

DATA_OUT        dd ?                            ;IO_addr+1
ADDR_STAT       dd ?                            ;IO_addr

                IFDEF ADLIBG                    ;(used during detection)
CTRL_ADDR       dd ?
CTRL_DATA       dd ?
                ENDIF

                IFDEF PROAUDIO
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

note_event      dd ?                            ;used for LRU counting
timb_hist       dd MAX_TIMBS dup (?)            ;last note event count for LRU
timb_offsets    dd MAX_TIMBS dup (?)            ;offsets of resident timbres
timb_bank       db MAX_TIMBS dup (?)            ;GTR bank
timb_num        db MAX_TIMBS dup (?)            ;GTR #
timb_attribs    db MAX_TIMBS dup (?)            ;bit 7=in use 6=protected

cache_base      dd ?                            ;timbre cache base addr
cache_size      dd ?                            ;total cache size in bytes
cache_end       dd ?                            ;offset of first free byte

TV_accum        dw ?                            ;DDA accum for refresh timing
pri_accum       dw ?                            ;DDA accum for priority watch
vol_update      db ?                            ;0 | U_KSLTL

rover_2op       dd ?                            ;circular voice # counters
rover_4op       dd ?

conn_shadow     db ?                            ;OPL3 Connection Select copy

BNK             STRUC           ;.BNK-style timbre definition
B_length        dw ?
B_transpose     db ?
B_mod_AVEKM     db ?            ;op_0 = FM modulator
B_mod_KSLTL     db ?                
B_mod_AD        db ?
B_mod_SR        db ?
B_mod_WS        db ?
B_fb_c          db ?
B_car_AVEKM     db ?            ;op_1 = FM carrier
B_car_KSLTL     db ?                
B_car_AD        db ?
B_car_SR        db ?
B_car_WS        db ?
BNK             ENDS

OPL3BNK         STRUC           ;.BNK-style OPL3 timbre definition
                BNK <>
O_mod_AVEKM     db ?            ;op_2
O_mod_KSLTL     db ?                
O_mod_AD        db ?
O_mod_SR        db ?
O_mod_WS        db ?
O_fb_c          db ?
O_car_AVEKM     db ?            ;op_3
O_car_KSLTL     db ?                
O_car_AD        db ?
O_car_SR        db ?
O_car_WS        db ?
OPL3BNK         ENDS

S_timbre_off    dd NUM_SLOTS dup (?)  ;pointer to timbre in local cache
S_duration      dw NUM_SLOTS dup (?)  ;# of TV intervals left in keyon
S_status        db NUM_SLOTS dup (?)  ;2=key off, 1=key on, 0=slot free
S_type          db NUM_SLOTS dup (?)  ;0=BNK, 1=TV inst, 2=TV effect, 3=OPL3
S_voice         db NUM_SLOTS dup (?)  ;YM3812 voice 0-8 or -1 assigned to slot
S_channel       db NUM_SLOTS dup (?)  ;MIDI channel owning slot
S_note          db NUM_SLOTS dup (?)  ;MIDI note # for slot's voice
S_keynum        db NUM_SLOTS dup (?)  ;MIDI key # before RBS translation
S_transpose     db NUM_SLOTS dup (?)  ;MIDI note transposition for slot
S_velocity      db NUM_SLOTS dup (?)  ;keyon velocity for note
S_sustain       db NUM_SLOTS dup (?)  ;note sustained if nonzero
S_update        db NUM_SLOTS dup (?)  ;bit mask for YM3812 register updates

S_KBF_shadow    db NUM_SLOTS dup (?)  ;shadowed KON-BLOCK-FNUM(H) registers
S_BLOCK         db NUM_SLOTS dup (?)  ;KON/BLOCK values
S_FBC           db NUM_SLOTS dup (?)  ;YM3812 multi-purpose registers
S_KSLTL_0       db NUM_SLOTS dup (?)  
S_KSLTL_1       db NUM_SLOTS dup (?)  
S_AVEKM_0       db NUM_SLOTS dup (?)  
S_AVEKM_1       db NUM_SLOTS dup (?)  
S_AD_0          db NUM_SLOTS dup (?)  ;YM3812 envelope registers
S_AD_1          db NUM_SLOTS dup (?)
S_SR_0          db NUM_SLOTS dup (?)
S_SR_1          db NUM_SLOTS dup (?)

S_scale_01      db NUM_SLOTS dup (?)  ;level scaling flags for ops 0-1

                IF NOT OSI_ALE
S_ws_val        dw NUM_SLOTS dup (?)  ;YM3812 registers
S_m1_val        dw NUM_SLOTS dup (?)  ;(declared in ALE.INC if TVFX enabled)
S_m0_val        dw NUM_SLOTS dup (?)
S_fb_val        dw NUM_SLOTS dup (?)
S_p_val         dw NUM_SLOTS dup (?)
S_v1_val        dw NUM_SLOTS dup (?)
S_v0_val        dw NUM_SLOTS dup (?)
S_f_val         dw NUM_SLOTS dup (?)  
                ENDIF

                IFDEF YMF262
S_KSLTL_2       db NUM_SLOTS dup (?)  ;YMF262 registers for operators 3-4
S_KSLTL_3       db NUM_SLOTS dup (?)  
S_AVEKM_2       db NUM_SLOTS dup (?)  
S_AVEKM_3       db NUM_SLOTS dup (?)  
S_AD_2          db NUM_SLOTS dup (?)
S_AD_3          db NUM_SLOTS dup (?)
S_SR_2          db NUM_SLOTS dup (?)
S_SR_3          db NUM_SLOTS dup (?)

S_ws_val_2      dw NUM_SLOTS dup (?)
S_m3_val        dw NUM_SLOTS dup (?)
S_m2_val        dw NUM_SLOTS dup (?)
S_v3_val        dw NUM_SLOTS dup (?)
S_v2_val        dw NUM_SLOTS dup (?)

S_scale_23      db NUM_SLOTS dup (?)  ;level scaling flags for ops 2-3
                ENDIF

FREE            equ 0                 ;S_status[] phase equates
KEYON           equ 1
KEYOFF          equ 2

BNK_INST        equ 0                 ;S_type[] equates
TV_INST         equ 1
TV_EFFECT       equ 2
OPL3_INST       equ 3

U_ALL_REGS      equ 11111001b         ;Bit mask equates for S_update
U_AVEKM         equ 10000000b           
U_KSLTL         equ 01000000b
U_ADSR          equ 00100000b
U_WS            equ 00010000b
U_FBC           equ 00001000b
U_FREQ          equ 00000001b

MIDI_vol        db NUM_CHANS dup (?)  ;volume 
MIDI_pan        db NUM_CHANS dup (?)  ;panpot
MIDI_pitch_l    db NUM_CHANS dup (?)  ;pitchwheel LSB
MIDI_pitch_h    db NUM_CHANS dup (?)  ;pitchwheel MSB
MIDI_express    db NUM_CHANS dup (?)  ;expression 
MIDI_mod        db NUM_CHANS dup (?)  ;modulation 
MIDI_sus        db NUM_CHANS dup (?)  ;HOLD1 pedal 
MIDI_vprot      db NUM_CHANS dup (?)  ;voice protection
MIDI_timbre     db NUM_CHANS dup (?)  ;timbre cache index for each channel
MIDI_bank       db NUM_CHANS dup (?)  ;Patch Bank Select values
MIDI_program    db NUM_CHANS dup (?)  ;program change # / channel

RBS_timbres     db 128 dup (?)        ;RBS timbre offset cache

MIDI_voices     db NUM_CHANS dup (?)  ;# of voices assigned to channel

V_channel       db NUM_VOICES dup (?) ;voice assigned to MIDI channel n or -1

S_V_priority    dw NUM_SLOTS dup (?)  ;adjusted voice priorities

;****************************************************************************
;*                                                                          *
;* I/O routines                                                             *
;*                                                                          *
;****************************************************************************

set_IO_parms    PROC\			;Set I/O address parms for adapter
                USES ebx esi edi\
                ,IO_ADDR,IRQ,DMA,DRQ

                IFDEF PAS

                mov LFMADDR,388h          
                mov LFMDATA,389h          
                mov RFMADDR,38ah          
                mov RFMDATA,38bh          
                mov eax,LFMADDR 

                ELSEIFDEF PASOPL

                mov eax,388h

                ELSE

                mov eax,[IO_ADDR]

                ENDIF

                IFDEF SBSTD

                add eax,8             

                ELSEIFDEF SBPRO1

                mov LFMADDR,eax
                inc eax
                mov LFMDATA,eax
                inc eax
                mov RFMADDR,eax
                inc eax
                mov RFMDATA,eax
                inc eax
                mov MIXADDR,eax
                inc eax
                mov MIXDATA,eax
                add eax,3

                ELSEIFDEF SBPRO2

                push eax
                add eax,4
                mov MIXADDR,eax
                inc eax
                mov MIXDATA,eax
                pop eax

                ENDIF

                mov ADDR_STAT,eax
                inc eax
                mov DATA_OUT,eax

                ret
set_IO_parms    ENDP

;****************************************************************************

;
;YM3812/YMF262 register access routines must preserve DS, SI, DI!
;

write_register  PROC Operator,Base,Value

                movzx ebx,BYTE PTR [Operator]
                mov ah,op_array[ebx]
                mov bl,op_index[ebx]
                add bl,BYTE PTR [Base]
                mov bh,ah
                mov cl,BYTE PTR [Value]
                jmp do_update
write_register  ENDP

send_byte       PROC Voice,Base,Value

                movzx ebx,BYTE PTR [Voice]
                mov ah,voice_array[ebx]
                mov bl,voice_num[ebx]
                add bl,BYTE PTR [Base]
                mov bh,ah
                mov cl,BYTE PTR [Value]
                jmp do_update
send_byte       ENDP

do_update:      pop ebp                 ;discard stack frame

update_reg      PROC                    ;Write value CL to register BX
                                        ;(preserves BX)
                IFDEF STEREO_3812

                mov al,bl               ;AL=address, CL=data
                mov edx,LFMADDR         ;select left register address
                out dx,al               
                mov edx,RFMADDR         ;select right register address
                out dx,al

                mov edx,LFMADDR         ;delay 3.3 uS
                mov ah,6
__rept_1:       in al,dx
                dec ah
                jne __rept_1

                mov al,cl
                mov edx,LFMDATA         ;write left data
                out dx,al
                mov edx,RFMDATA         ;write right data (same as left)
                out dx,al

                mov edx,LFMADDR         ;delay 23 uS (3.3 for YMF262)
                mov ah,IOWVAL
__rept_2:       in al,dx
                dec ah
                jne __rept_2

                ELSE

                mov edx,ADDR_STAT

                IFDEF YMF262            ;index 2nd array if addr > 256
                add dl,bh               
                adc dh,0
                add dl,bh               
                adc dh,0
                ENDIF

                mov al,bl               ;AL=address, CL=data
                out dx,al               ;select register address

                mov ah,6
__rept_3:       in al,dx
                dec ah
                jne __rept_3

                mov al,cl
                inc edx
                out dx,al
                dec edx

                mov ah,IOWVAL
__rept_4:       in al,dx
                dec ah
                jne __rept_4

                ENDIF
                ret
update_reg      ENDP
                
                IFDEF STEREO_3812       ;Access 3812 stereo registers
stereo_register PROC Part,Base,RLValues

                movzx ebx,BYTE PTR [Part]
                movzx eax,BYTE PTR [Base]
                add al,op_index[ebx]
                mov edx,LFMADDR         ;select left register address
                out dx,al
                mov edx,RFMADDR         ;select right register address
                out dx,al
                mov edx,LFMADDR         ;delay 3.3 uS

                mov ah,6
__rept_1:       in al,dx
                dec ah
                jne __rept_1

                mov edx,LFMDATA         ;write left data
                mov al,BYTE PTR [RLValues+1]
                out dx,al
                mov edx,RFMDATA         ;write right data
                mov al,BYTE PTR [RLValues]
                out dx,al
                mov edx,LFMADDR         ;delay 23 uS

                mov ah,42
__rept_2:       in al,dx
                dec ah
                jne __rept_2

                ret
stereo_register ENDP
                ENDIF

read_status     PROC                    ;Read YM3812 status register
                mov edx,ADDR_STAT        
                mov eax,0
                in al,dx
                ret
read_status     ENDP

;****************************************************************************
                IFDEF ADLIBG            

IO_wait         PROC                    ;Wait for clear SB bit (Ad Lib Gold)
                mov ecx,500
                mov edx,CTRL_ADDR
__wait:         in al,dx
                and al,01000000b
                loopnz __wait
                ret
IO_wait         ENDP

get_FM_vol      PROC RegNum             ;Get FM VOLUME register value
                call IO_wait
                mov edx,CTRL_ADDR
                mov eax,[RegNum]
                out dx,al
                call IO_wait
                mov edx,CTRL_DATA
                in al,dx
                ret
get_FM_vol      ENDP

set_FM_vol      PROC RegNum,Val         ;Set FM VOLUME register value
                call IO_wait
                mov edx,CTRL_ADDR
                mov eax,[RegNum]
                out dx,al
                call IO_wait
                mov edx,CTRL_DATA
                mov eax,[Val]
                out dx,al
                ret
set_FM_vol      ENDP
                ENDIF

;****************************************************************************
                IFDEF PROAUDIO
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
detect_device   PROC\
                USES ebx esi edi\		;Attempt to detect card
                ,H,IO_ADDR,IRQ,DMA,DRQ

                pushfd
                cli

                IFDEF ADLIBG              ;if Ad Lib Gold, look for control
                                          ;chip
                mov edx,IO_ADDR
                add edx,2
                mov CTRL_ADDR,edx
                inc edx
                mov CTRL_DATA,edx

                mov edx,CTRL_ADDR         ;attempt to enable control chip
                mov al,0ffh               
                out dx,al

                invoke get_FM_vol,9       ;get left volume
                mov esi,eax
                invoke get_FM_vol,10      ;get right volume
                mov edi,eax

                xor esi,0101b             ;tweak a few bits
                xor edi,1010b

                invoke set_FM_vol,9,esi   ;write the tweaked values back
                invoke set_FM_vol,10,edi

                invoke get_FM_vol,9       ;see if changes took effect
                cmp eax,esi
                mov eax,0                 ;(return failure)
                jne __return
                invoke get_FM_vol,10
                cmp eax,edi
                mov eax,0                 ;(return failure)
                jne __return

                xor esi,0101b             ;control chip found: restore old
                xor edi,1010b             ;values & re-enable FM sound

                invoke set_FM_vol,9,esi
                invoke set_FM_vol,10,edi

                call IO_wait
                mov edx,CTRL_ADDR          
                mov al,0feh               
                out dx,al

                mov eax,1                 ;return success
                jmp __return

                ELSE		          ;(not Ad Lib Gold card...)

                push DATA_OUT             ;preserve current I/O addresses
                push ADDR_STAT
                IFDEF STEREO_3812
                push LFMADDR
                push RFMADDR
                push LFMDATA
                push RFMDATA
                ENDIF
                IFDEF SBPRO
                push MIXDATA
                push MIXADDR
                ENDIF

                invoke set_IO_parms,[IO_ADDR],[IRQ],[DMA],[DRQ]

                IFDEF SBPRO2              ;do Ad Lib detection at 2x8, NOT 2x0
                add DATA_OUT,8            ;(avoids hangups on standard SB)
                add ADDR_STAT,8
                ENDIF                     

                call detect_Adlib         ;do Ad Lib compatibility test first
                cmp eax,0
                je __exit

                IFDEF SBPRO               ;then look for CT-1345A mixer chip
                mov edx,MIXADDR
                mov al,0ah                ;select Mic Vol control
                out dx,ax
                jmp $+2
                mov edx,MIXDATA
                in al,dx                  ;get original value
                jmp $+2
                mov ah,al                 ;save it
                xor al,110b               ;toggle its bits
                out dx,al                 ;write it back
                jmp $+2
                in al,dx                  ;read/verify changed value
                xor al,110b              
                cmp al,ah
                mov al,ah                 ;put the old value back
                out dx,al
                mov eax,0
                jne __exit 
                mov eax,1                 ;signal card found -- it's a SB PRO
                ENDIF

                IFDEF PROAUDIO            ;if Pro Audio Spectrum, look for 

                mov eax,0bc00h            ;MVSOUND.SYS device driver
                mov ebx,03f3fh
                invoke INT2F,eax,ebx
                xor ebx,ecx
                xor ebx,edx
                cmp bx,'MV'               ;MediaVision flag
                mov eax,0
                jne __exit

                IFDEF PAS                 ;(dual-3812 card)
                mov ax,0bc07h             ;function 7: get state table entries
                invoke INT2F,eax,eax
                and bl,01111111b          ;set FM split (bit 7)
                mov al,bl                 ;...and write to mixer port
                mov dx,0b88h
                out dx,al
                ENDIF

                mov eax,1                 ;signal PAS card found

                ENDIF

__exit:
       	IFDEF SBPRO
                pop MIXADDR
                pop MIXDATA
                ENDIF
                IFDEF STEREO_3812
                pop RFMDATA
                pop LFMDATA
                pop RFMADDR
                pop LFMADDR
                ENDIF
                pop ADDR_STAT
                pop DATA_OUT

                ENDIF                     ;IFDEF ADLIBG

__return:       POP_F                     ;return AX=0 if not found
                ret
detect_device   ENDP

                IFNDEF ADLIBG
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
                ENDIF

;****************************************************************************
reset_synth     PROC\                     ;Init regs & register map
                USES ebx esi edi

                pushfd                  
                cli

                IFDEF YMF262
                mov ebx,105h
                mov cl,00000001b          ;set OPL3 NEW bit
                call update_reg

                mov ebx,104h              ;init OPL3 to 18 2-op voices
                mov cl,0
                call update_reg
                mov conn_shadow,0
                ENDIF

                mov ebx,1h
__init_0:       mov cl,array0_init[ebx-1]
                call update_reg
                inc ebx
                cmp ebx,0f5h
                jbe __init_0

                IFDEF YMF262
                mov ebx,101h
__init_1:       mov cl,array1_init[ebx-101h]
                call update_reg
                inc ebx
                cmp ebx,1f5h
                jbe __init_1
                ENDIF

                POP_F
                ret
reset_synth     ENDP

;****************************************************************************
shutdown_synth  PROC\                     ;Establish Ad Lib compatibility mode
                USES ebx esi edi

                pushfd
                cli

                IFDEF YMF262
                mov ebx,105h
                mov cl,0                  ;clear OPL3 NEW bit
                call update_reg
                ENDIF

                IFDEF PAS                 ;(dual-3812 card)
                mov ax,0bc07h             ;function 7: get state table entries
                invoke INT2F,eax,eax
                or bl,10000000b           ;clear FM split (bit 7)
                mov al,bl                 ;...and write to mixer port
                mov dx,0b88h
                out dx,al
                ENDIF

                POP_F
                ret
shutdown_synth  ENDP

;****************************************************************************
;*                                                                          *
;*  Timbre cache management / related API calls                             *
;*                                                                          *
;****************************************************************************

index_timbre    PROC \         		;Get global timbre's local index
                USES ebx esi edi\
                ,GNum:DWORD

                mov esi,0
                movzx eax,WORD PTR [GNum]   
__find_gnum:    test timb_attribs[esi],10000000b
                jz __find_next          ;(timbre unused)
                cmp timb_bank[esi],ah
                jne __find_next
                cmp timb_num[esi],al
                je __found
__find_next:    inc esi
                cmp esi,MAX_TIMBS
                jb __find_gnum

                mov esi,-1              ;return -1 if timbre not loaded

__found:        mov eax,esi
                ret
index_timbre    ENDP

;****************************************************************************
protect_timbre  PROC\
                USES ebx esi edi\           ;Protect a timbre from replacement
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
                cmp ebx,MAX_TIMBS
                jb __prot_timb

__exit:         POP_F
                ret
protect_timbre  ENDP

;****************************************************************************
unprotect_timbre PROC\
                USES ebx esi edi\		;Allow a timbre to be replaced
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
                cmp ebx,MAX_TIMBS
                jb __unprot_timb

__exit:         POP_F
                ret
unprotect_timbre ENDP

;****************************************************************************
timbre_status   PROC\
                USES ebx esi edi\		;Return 0 if timbre not resident
                ,H,Bank,Num
                pushfd 
                cli

                mov eax,0
                mov al,BYTE PTR [Num]
                mov ah,BYTE PTR [Bank]

                invoke index_timbre,eax
                cmp eax,-1
                je __exit               ;not resident, inc AX to return 0

                mov esi,eax             ;else return offset+1 in local cache
                shl esi,1
                shl esi,1
                mov eax,timb_offsets[esi] ;(for diagnostic purposes)

__exit:         inc eax

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
define_cache    PROC\
                USES ebx esi edi\
                ,H,Address:PTR,C_Size
                pushfd
                cli

                mov edi,[Address]
                mov cache_base,edi

                mov eax,[C_Size]
                mov cache_size,eax

                mov cache_end,0

                POP_F
                ret
define_cache 	ENDP

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

                mov esi,[esi].state_table.TIMB   ;make sure TIMB chunk is present
                cmp [esi],DWORD PTR 'BMIT'
                jne __no_request        ;if not, no requests are possible

                add esi,8
                movzx edi,WORD PTR [esi];get TIMB.cnt
__chk_index:    add esi,2
                movzx eax,WORD PTR [esi]
                invoke index_timbre,[esi]
                cmp eax,-1              ;timbre in local cache?
                je __request            ;no, request it
__next_index:   dec edi
                jne __chk_index
                jmp __no_request        ;all requested timbres loaded, exit

__request:      movzx eax,WORD PTR [esi];else return request: AL=num, AH=bank
                jmp __exit

__no_request:   mov eax,-1
                
__exit:         POP_F
                ret
get_request ENDP

;****************************************************************************
delete_LRU      PROC\                   ;Excise LRU timbre from cache, adjust
                USES ebx esi edi es
                LOCAL index,tsize,toff  ;links & references
                cld

                push ds
                pop es

                mov index,-1
                mov eax,-1           
                mov ebx,0                
                mov edx,-1
                mov esi,0                
__find_LRU:     test timb_attribs[esi],10000000b
                jz __next_LRU           ;(timbre not installed)
                test timb_attribs[esi],01000000b
                jnz __next_LRU          ;(timbre protected)

                cmp timb_hist[ebx],eax  ;(timbre not least-recently-used)
                ja __next_LRU

__log_LRU:      mov eax,timb_hist[ebx]
                mov index,esi
__next_LRU:     add ebx,4
                inc esi
                cmp esi,MAX_TIMBS
                jb __find_LRU

                cmp index,-1        
                je __exit

                mov esi,index           ;SI = LRU timbre index
                shl esi,1
                shl esi,1
                mov eax,timb_offsets[esi]
                mov toff,eax
                mov edi,cache_base
                add edi,eax             ;EDI -> timbre to delete
                movzx esi,[edi].BNK.B_length
                mov tsize,esi
                add esi,edi             ;ESI -> next timbre in cache

                mov ecx,cache_base
                add ecx,cache_end
                sub ecx,esi             ;ECX = size of area to move down 

                rep movsb

                mov edi,index           ;clear attributes of deleted timbre
                mov timb_attribs[edi],00000000b
                mov eax,tsize           ;adjust end-of-cache pointer
                sub cache_end,eax                

                mov edi,0                ;invalidate channel references to
                mov ebx,0                ;the deleted timbre
__for_chan:     mov bl,MIDI_timbre[edi]  
                cmp bl,-1
                je __next_chan
                cmp ebx,index
                jne __next_chan
                mov MIDI_timbre[edi],-1
__next_chan:    inc edi
                cmp edi,NUM_CHANS
                jne __for_chan
                mov edi,0
__for_RBS:      mov bl,RBS_timbres[edi]
                cmp ebx,index
                jne __next_RBS
                mov RBS_timbres[edi],-1
__next_RBS:     inc edi
                cmp edi,SIZE RBS_timbres
                jne __for_RBS

                mov edi,0               ;adjust offset pointers for all
                mov ebx,0               ;timbres after the deleted timbre
__for_index:    test timb_attribs[ebx],10000000b
                jz __next_index
                mov eax,timb_offsets[edi] 
                cmp eax,toff
                jbe __next_index
                sub eax,tsize
                mov timb_offsets[edi],eax
__next_index:   add di,4
                inc ebx
                cmp ebx,MAX_TIMBS
                jb __for_index

                mov eax,cache_base
                add toff,eax
                mov edi,0               ;adjust pointers to any active timbres
                mov esi,0                
__for_slot:     cmp S_status[esi],FREE
                je __next_slot
                mov eax,S_timbre_off[edi]
                cmp eax,toff
                jb __next_slot
                jne __move_down
                invoke release_voice,esi
                mov S_status[esi],FREE
                jmp __next_slot
__move_down:    sub eax,tsize
                mov S_timbre_off[edi],eax
__next_slot:    add edi,2
                inc esi
                cmp esi,NUM_SLOTS
                jne __for_slot

__exit:         ret
delete_LRU 	ENDP

;****************************************************************************
install_timbre  PROC\
                USES ebx esi edi es\
                ,H,Bank,Num,Address:PTR
                LOCAL index
                pushfd
                cli
                cld

                push ds
                pop es

                mov eax,0
                mov al,BYTE PTR [Num]
                mov ah,BYTE PTR [Bank]

                invoke index_timbre,eax
                mov index,eax
                cmp eax,-1
                jne __set_patch         ;if timbre already resident, index it

                mov eax,[Address]
                cmp eax,0
                jz __exit

__get_space:    mov edi,0               ;find first available timbre slot
__find_index:   test timb_attribs[edi],10000000b
                jz __get_size           ;found it....
                inc edi
                cmp edi,MAX_TIMBS
                jne __find_index
                jmp __kill_LRU          ;else kill a timbre to free up a slot

__get_size:     mov esi,[Address]       ;get new timbre size
                movzx ebx,[esi].BNK.B_length     
                add ebx,cache_end
                cmp ebx,cache_size      ;enough free space in local cache?
                jbe __install           ;yes, install it

__kill_LRU:     call delete_LRU         ;get rid of old timbres until the
                jmp __get_space         ;new timbre can be installed

__install:      xchg cache_end,ebx

                mov index,edi           ;save index of new timbre

                mov eax,0
                mov al,BYTE PTR [Num]   ;AL=num, AH=bank
                mov ah,BYTE PTR [Bank]
                mov timb_num[edi],al    ;record global # in slot
                mov timb_bank[edi],ah   ;mark timbre "in use/unprotected"
                mov timb_attribs[edi],10000000b                  
                shl edi,1               ;update timbre's timestamp
                shl edi,1
                mov eax,note_event     
                add note_event,1      
                mov timb_hist[edi],eax  
                mov timb_offsets[edi],ebx

                mov edi,cache_base      ;ESI -> new timbre source addr
                add edi,ebx             ;EDI -> cache address for new timbre
                movzx ecx,[esi].BNK.B_length
                rep movsb

__set_patch:    mov al,BYTE PTR [Num]                                     
                mov ah,BYTE PTR [Bank]
                mov ebx,index
                mov edi,0               ; for all channels...
__set_requests: cmp MIDI_program[edi],al; if timbre was requested in channel,
                jne __next_req          ; update channel's timbre index
                cmp MIDI_bank[edi],ah
                jne __next_req
                mov MIDI_timbre[edi],bl
__next_req:     inc edi
                cmp edi,NUM_CHANS
                jne __set_requests

__exit:         POP_F
                ret
install_timbre  ENDP

;****************************************************************************
;*                                                                          *
;*  MIDI interpreter and related procedures                                 *
;*                                                                          *
;****************************************************************************

init_synth      PROC\                   ;Init MIDI synthesizer emulation
                USES ebx esi edi es
                pushfd           
                cli
                cld

                push ds
                pop es

                mov note_event,0

                mov edi,0
__init_timbres: mov timb_attribs[edi],00000000b
                inc edi
                cmp edi,MAX_TIMBS
                jne __init_timbres

                mov edi,0
__init_chans:   mov MIDI_timbre[edi],-1
                mov MIDI_voices[edi],0
                mov MIDI_program[edi],-1
                mov MIDI_bank[edi],0
                inc edi
                cmp edi,NUM_CHANS
                jne __init_chans

                mov edi,0
__init_slots:   mov S_status[edi],FREE
                inc edi
                cmp edi,NUM_SLOTS
                jne __init_slots

                mov edi,0
__init_voices:  mov V_channel[edi],-1
                inc edi
                cmp edi,NUM_VOICES
                jne __init_voices

                mov edi,OFFSET RBS_timbres
                mov ecx,SIZE RBS_timbres
                mov al,-1
                rep stosb 

                mov TV_accum,0
                mov pri_accum,0
                mov vol_update,0
                mov rover_2op,-1
                mov rover_4op,-1

                POP_F
                ret
init_synth 	ENDP

;****************************************************************************
assign_voice    PROC\
                USES ebx esi edi\
                ,Slot               ;Allocate hardware voice to slot
                
                IFDEF YMF262
                mov esi,[Slot]           
                cmp S_type[esi],OPL3_INST
                je __OPL3
                ENDIF

                mov edx,-1              ;try to find an unassigned voice
                mov ebx,rover_2op
__search_free:  inc edx
                cmp edx,NUM_VOICES
                je __seize_voice        ;(# of active slots > # of voices)
                inc ebx
                IF CIRC_ASSIGN
                cmp ebx,NUM_VOICES
                jne __chk_free
                mov ebx,0
__chk_free:     mov rover_2op,ebx
                ENDIF
                cmp V_channel[ebx],-1   
                jne __search_free

                mov S_voice[esi],bl     ;found free voice, assign it to slot
                mov edi,ebx
                mov bl,S_channel[esi]
                inc MIDI_voices[ebx]
                mov V_channel[edi],bl

__update:       mov S_update[esi],U_ALL_REGS
                invoke update_voice,esi  ;update the hardware
                ret

__seize_voice:  call update_priority    ;assign voice based on priority search
                ret                     

                IFDEF YMF262
__OPL3:         mov edx,-1              ;assigning 4-op voice...
                mov edi,rover_4op
                mov bh,0
__OPL3_search:  inc edx                 ;try to find unassigned eligible pair
                cmp edx,NUM_4OP_VOICES
                je __seize_voice
                inc edi
                IF CIRC_ASSIGN
                cmp edi,NUM_4OP_VOICES
                jne __OPL3_free
                mov edi,0
__OPL3_free:    mov rover_4op,edi
                ENDIF
                mov bl,op4_voice[edi]
                cmp V_channel[ebx],-1
                jne __OPL3_search
                cmp V_channel[ebx+3],-1
                jne __OPL3_search

                mov S_voice[esi],bl      
                mov edi,ebx
                mov bl,S_channel[esi]
                inc MIDI_voices[ebx]
                mov V_channel[edi],bl
                mov V_channel[edi+3],bl
                jmp __update
                ENDIF

assign_voice 	ENDP

;****************************************************************************
release_voice   PROC\
                USES ebx esi edi\
                ,Slot               ;Release slot's voice

                mov esi,[Slot]

                cmp S_voice[esi],-1   
                je __exit            

                and S_BLOCK[esi],11011111b
                or S_update[esi],U_FREQ ;force KON = 0...

                invoke update_voice,esi ;...silence any note...

                mov ebx,0               ;...and deallocate the voice
                mov bl,S_channel[esi]     
                dec MIDI_voices[ebx]
                mov bl,S_voice[esi]

                cmp S_type[esi],OPL3_INST
                jne __free_chan
                mov V_channel[ebx+3],-1 ;free both "halves" of 4-op OPL3
__free_chan:    mov V_channel[ebx],-1   ;voice

                mov S_voice[esi],-1

                cmp S_type[esi],OPL3_INST
                je __release            ;release any .BNK slots deallocated...
                cmp S_type[esi],BNK_INST
                jne __exit              
__release:      mov S_status[esi],FREE  ;otherwise, slot remains active

__exit:         ret
release_voice 	ENDP
                    
;****************************************************************************
update_voice    PROC\
                USES ebx esi edi\
                ,Slot                   ;Update hardware regs for slot
                LOCAL voice,voice0,voice1,vol:BYTE,lvol:BYTE,rvol:BYTE
                LOCAL f_num:WORD
                LOCAL array,update_save:BYTE
                LOCAL scale_flags:BYTE
                LOCAL m0_val,m1_val,AVEKM_0:BYTE,AVEKM_1:BYTE
                LOCAL v0_val,v1_val,KSLTL_0:BYTE,KSLTL_1:BYTE
                LOCAL AD_0:BYTE,AD_1:BYTE,SR_0:BYTE,SR_1:BYTE
                LOCAL ws_val,fb_val,FBC:BYTE

                mov esi,[Slot]          ;update only requested parameters for
                                        ;speed
                cmp S_voice[esi],-1
                je __exit               ;no hardware voice assigned, exit

                test S_update[esi],U_KSLTL
                jz __chk_type           ;skip level calculations if not needed

                mov edi,DWORD PTR S_channel[esi]
                and edi,0fh             ;DI = MIDI channel

                movzx eax,MIDI_vol[edi]
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
                mov vol,al              ;AX=composite (vol+expression) volume

                IFDEF STEREO_3812       ;interpret panpot for stereo cards
                mov ecx,eax
                mov ebx,0
                mov bl,MIDI_pan[edi]
                mov al,pan_graph[ebx]
                mul cl                  ;calculate left-channel volume
                shl eax,1               ;(AX*2)/256 = AX/128 ї AX/127
                mov al,ah
                cmp al,1
                sbb al,-1               ;(error = 1/127 units; round up if !0)
                mov lvol,al             ;set left volume
                sub ebx,127
                neg ebx
                mov al,pan_graph[ebx]
                mul cl                  ;calculate right-channel volume
                shl eax,1               ;(AX*2)/256 = AX/128 ї AX/127
                mov al,ah
                cmp al,1
                sbb al,-1               ;(error = 1/127 units; round up if !0)
                mov rvol,al             ;set right volume
                ENDIF

__chk_type:     mov eax,0
                cmp S_type[esi],OPL3_INST
                jne __set_array
                mov eax,1               ;write 2nd array first if OPL3 4-op
__set_array:    mov array,eax

                IFDEF YMF262            ;update OPL3 CONN SEL register
                mov edi,DWORD PTR S_voice[esi]
                and edi,0ffh             ;DI = voice
                mov cl,conn_shadow
                mov dl,conn_sel[edi]
                or eax,eax
                jz __2_op

                or cl,dl               ;turning on 4-op voice...
                cmp cl,conn_shadow     ;CS changed (replacing 2 2-op voices?)
                je __do_arrays         ;no, 4-op connection is already valid
                mov conn_shadow,cl     ;else enable 4-op connection select
                mov ebx,104h
                call update_reg
                jmp __do_arrays
                
__2_op:         not dl                 ;turning on 2-op voice...
                and cl,dl
                cmp cl,conn_shadow     ;CS changed (replacing 1 4-op voice?)
                je __do_arrays         ;no, voice already in 2-op mode
                mov conn_shadow,cl     ;else disable 4-op connection select
                mov ebx,104h
                call update_reg         
                                       ;mute "other half" of old 4-op voice
                mov al,alt_op_0[edi]   ;set modulator release = immed
                invoke write_register,eax,080h,0fh

                mov al,alt_op_1[edi]   ;set carrier release = immed
                invoke write_register,eax,080h,0fh

                movzx eax,alt_voice[edi]    ;set KON = 0 to force note off
                invoke send_byte,eax,0b0h,0
                ENDIF

__do_arrays:    cmp array,0
                je __array_0

                IFDEF YMF262
                mov eax,0
                mov ebx,0
                mov bl,S_voice[esi]
                add ebx,3               ;index 2nd operator pair at v+3
                mov voice,ebx
                mov al,op_0[ebx]
                mov voice0,eax          ;"modulator" operator cell (2nd pair)
                mov al,op_1[ebx]
                mov voice1,eax          ;"carrier" operator cell (2nd pair)

                mov ebx,esi             ;set op0 and op1 parms for 2nd pair...
                movzx eax,S_m2_val[esi*2]
                mov m0_val,eax
                movzx eax,S_m3_val[esi*2]
                mov m1_val,eax
                mov al,S_AVEKM_2[esi]
                mov AVEKM_0,al
                mov al,S_AVEKM_3[esi]
                mov AVEKM_1,al
                movzx eax,S_v2_val[esi*2]
                mov v0_val,eax
                movzx eax,S_v3_val[esi*2]
                mov v1_val,eax
                mov al,S_KSLTL_2[esi]
                mov KSLTL_0,al
                mov al,S_KSLTL_3[esi]
                mov KSLTL_1,al
                mov al,S_AD_2[esi]
                mov AD_0,al
                mov al,S_AD_3[esi]
                mov AD_1,al
                mov al,S_SR_2[esi]
                mov SR_0,al
                mov al,S_SR_3[esi]
                mov SR_1,al
                mov fb_val,0
                movzx eax,S_ws_val_2[esi*2]
                mov ws_val,eax
                mov al,S_FBC[esi]
                shr al,1
                mov FBC,al
                mov al,S_scale_23[esi]
                mov scale_flags,al

                mov al,S_update[esi]
                mov update_save,al      ;save update flags for 2nd op pair
                ENDIF

__do_array:     test S_update[esi],U_AVEKM
                jnz __go_AVEKM
__AVEKM_done:   test S_update[esi],U_KSLTL
                jnz __go_KSLTL
__KSLTL_done:   test S_update[esi],U_ADSR
                jnz __go_ADSR
__ADSR_done:    test S_update[esi],U_WS
                jnz __go_WS
__WS_done:      test S_update[esi],U_FBC
                jnz __go_FBC
__FBC_done:     test S_update[esi],U_FREQ
                jnz __go_FREQ           ;(update FREQ's KeyOn bit last)
__FREQ_done:
                cmp array,0
                je __exit
                mov array,0

                mov al,update_save
                mov S_update[esi],al

__array_0:      movzx ebx,S_voice[esi]
                mov voice,ebx           ;voice #
                movzx eax,op_0[ebx]
                mov voice0,eax          ;"modulator" operator cell
                mov al,op_1[ebx]
                mov voice1,eax          ;"carrier" operator cell

                mov ebx,esi
                movzx eax,WORD PTR S_m0_val[ebx][esi]
                mov m0_val,eax
                movzx eax,WORD PTR S_m1_val[ebx][esi]
                mov m1_val,eax
                mov al,S_AVEKM_0[esi]
                mov AVEKM_0,al
                mov al,S_AVEKM_1[esi]
                mov AVEKM_1,al
                movzx eax,WORD PTR S_v0_val[ebx][esi]
                mov v0_val,eax
                movzx eax,WORD PTR S_v1_val[ebx][esi]
                mov v1_val,eax
                mov al,S_KSLTL_0[esi]
                mov KSLTL_0,al
                mov al,S_KSLTL_1[esi]
                mov KSLTL_1,al
                mov al,S_AD_0[esi]
                mov AD_0,al
                mov al,S_AD_1[esi]
                mov AD_1,al
                mov al,S_SR_0[esi]
                mov SR_0,al
                mov al,S_SR_1[esi]
                mov SR_1,al
                movzx eax,S_fb_val[ebx][esi]
                mov fb_val,eax
                movzx eax,WORD PTR S_ws_val[ebx][esi]
                mov ws_val,eax
                mov al,S_FBC[esi]
                mov FBC,al
                mov al,S_scale_01[esi]
                mov scale_flags,al
                jmp __do_array

__exit:         ret

__go_AVEKM:     jmp __AVEKM
__go_KSLTL:     jmp __KSLTL
__go_ADSR:      jmp __ADSR
__go_WS:        jmp __WS
__go_FBC:       jmp __FBC
__go_FREQ:      jmp __FREQ

;----------------------------------------------------------------------------
__AVEKM:        mov edi,DWORD PTR S_channel[esi]
                and edi,0fh     

                mov al,MIDI_mod[edi]
                mov edi,01000000b       ;set bit 6 (Vibrato) if MIDI
                cmp al,64               ;Modulation controller >= 64
                jge __set_mod
                mov edi,00000000b

__set_mod:      mov eax,m0_val          ;get multiplier 0 value
                mov cl,4
                shr eax,cl
                mov al,ah               ;divide by 4096
                or eax,edi              ;merge with AM modulation bit
                or al,AVEKM_0           ;merge with EG/KSR/VIB bits
                invoke write_register,voice0,20h,eax

                mov eax,m1_val          ;get multiplier 1 value
                mov cl,4
                shr eax,cl
                mov al,ah               ;divide by 4096
                or eax,edi              ;merge with AM modulation bit
                or al,AVEKM_1           ;merge with EG/KSR/VIB bits
                invoke write_register,voice1,20h,eax

                and S_update[esi],NOT U_AVEKM
                jmp __AVEKM_done

;----------------------------------------------------------------------------
__KSLTL:        mov ebx,v0_val          ;get modulator volume level
                shr ebx,1
                shr ebx,1
                mov bl,bh               ;divide by 1024 (BL=BH=0-63)
                
                IFDEF STEREO_3812
                test scale_flags,01b    ;additive operator?
                jz __mod_vol            ;no, don't scale level
                mov cl,127              ;else scale by composite volume factor
                mov al,bh
                mul lvol
                div cl
                mov bh,al
                mov al,bl
                mul rvol
                div cl
                mov bl,al
__mod_vol:      not ebx                 ;BL=right vol, BH=left vol         
                and ebx,3f3fh           ;convert volume to attenuation
                mov al,KSLTL_0          ;merge with KSL bits
                mov ah,al
                or ebx,eax
                invoke stereo_register,voice0,40h,ebx
                ELSE
                test scale_flags,01b    ;additive operator?
                jz __mod_vol            ;no, don't scale level
                mov cl,127              ;else scale by composite volume factor
                mov al,bh
                mul vol
                div cl
                mov bl,al
__mod_vol:      not bl                  ;BL=monaural vol
                and bl,3fh              ;convert volume to attenuation
                or bl,KSLTL_0           ;merge with KSL bits
                invoke write_register,voice0,40h,ebx
                ENDIF

                mov ebx,v1_val          ;get carrier volume level
                shr ebx,1
                shr ebx,1
                mov bl,bh               ;divide by 1024 (BL=BH=0-63)
                
                IFDEF STEREO_3812
                test scale_flags,10b    ;additive operator?
                jz __car_vol            ;no, don't scale level
                mov cl,127              ;else scale by composite volume factor
                mov al,bh
                mul lvol
                div cl
                mov bh,al
                mov al,bl
                mul rvol
                div cl
                mov bl,al
__car_vol:      not ebx                 ;BL=right vol, BH=left vol         
                and ebx,3f3fh           ;convert volume to attenuation
                mov al,KSLTL_1          ;merge with KSL bits
                mov ah,al
                or ebx,eax
                invoke stereo_register,voice1,40h,ebx
                ELSE
                test scale_flags,10b    ;additive operator?
                jz __car_vol            ;no, don't scale level
                mov cl,127              ;else scale by composite volume factor
                mov al,bh
                mul vol
                div cl
                mov bl,al
__car_vol:      not bl                  ;BL=monaural vol
                and bl,3fh              ;convert volume to attenuation
                or bl,KSLTL_1           ;merge with KSL bits
                invoke write_register,voice1,40h,ebx
                ENDIF

                and S_update[esi],NOT U_KSLTL
                jmp __KSLTL_done

;----------------------------------------------------------------------------
__ADSR:         mov al,AD_0
                invoke write_register,voice0,60h,eax

                mov al,AD_1
                invoke write_register,voice1,60h,eax

                mov al,SR_0
                invoke write_register,voice0,80h,eax

                mov al,SR_1
                invoke write_register,voice1,80h,eax

                and S_update[esi],NOT U_ADSR
                jmp __ADSR_done

;----------------------------------------------------------------------------
__WS:           mov al,BYTE PTR ws_val
                invoke write_register,voice1,0e0h,eax

                mov al,BYTE PTR ws_val+1
                invoke write_register,voice0,0e0h,eax

                and S_update[esi],NOT U_WS
                jmp __WS_done

;----------------------------------------------------------------------------
__FBC:          mov eax,fb_val          ;get feedback
                mov cl,4                ;divide by 4096
                shr eax,cl
                and ah,00001110b        ;merge bits 1-3 with C bit
                mov al,FBC
                and al,00000001b
                or al,ah

                IFDEF YMF262
                or al,00110000b         ;assume center channel
                mov edi,DWORD PTR S_channel[esi]
                and edi,0fh             
                mov bl,MIDI_pan[edi]    ;get panpot for voice's MIDI channel
                cmp bl,R_PAN_THRESH  
                jbe __right
                cmp bl,L_PAN_THRESH
                jb __write_FBC
                and al,LEFT_MASK       ;>= left threshold, mute right channel
                jmp __write_FBC
__right:        and al,RIGHT_MASK      ;<= right threshold, mute left channel
                ENDIF

__write_FBC:    invoke send_byte,voice,0c0h,eax

                and S_update[esi],NOT U_FBC
                jmp __FBC_done

;----------------------------------------------------------------------------
__FREQ:         cmp array,1
                je __end_freq           ;(OPL3 freq not used in 2nd array)
                
                mov al,S_type[esi]      ;if BNK instrument or TVFX instrument,
                cmp al,TV_EFFECT        ;note #/pitch bend determine freq...
                jne __MIDI_freq
                jmp __TV_effect         ;else TV freq/blk vals determine freq

__MIDI_freq:    test S_BLOCK[esi],00100000b
                jnz __key_on            ;KON bit = 1; turn note on

                mov al,S_KBF_shadow[esi];retrieve voice's last KBF value
                and al,11011111b        ;set KON = 0
                invoke send_byte,voice,0b0h,eax
                jmp __end_freq          ;turn the note off immediately

__key_on:       movzx ebx,S_channel[esi];get signed pitch bend value
                movzx eax,MIDI_pitch_h[ebx]
                mov ecx,7
                shl eax,cl
                or al,MIDI_pitch_l[ebx]  
                sub ax,2000h            

                mov ecx,5               ;divide by 0x20, preserving sign
                sar ax,cl               ;(range now +0x100 to -0x100)

                mov ecx,0
                mov cl,DEF_PITCH_RANGE  ;normally 12 (+0xc00 to -0xc00)
                imul ecx                 

                movzx ebx,S_note[esi]   ;get key # 12-108

                mov ecx,eax             ;transpose it
                mov al,S_transpose[esi]
                cbw
                cwde
                add ebx,eax
                mov eax,ecx

                sub ebx,24              ;normalize to 0-95

__norm_1:       add ebx,12
                cmp ebx,0
                jl __norm_1
                add ebx,12
__norm_2:       sub ebx,12
                cmp ebx,95
                jg __norm_2

                add ah,bl               ;add computed note offset
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

                mov edi,0
                mov di,ax
                shr edi,4               ;divide by 16 to get physical note #
                mov edx,edi             ;DX = halftone/octave index   
                movzx ebx,note_halftone[edi]
                mov edi,ebx             ;DI = halftone value within octave

                shl edi,5               ;derive table row
                shl ax,1
                and ax,11111b
                cwde
                add edi,eax             ;derive table index  

                movzx eax,WORD PTR freq_table[edi]

                mov edi,edx
                mov bl,note_octave[edi]
                dec bl
                or ax,ax
                jge __bump_block
                inc bl
__bump_block:   or bl,bl
                jge __set_M_freq
                inc bl
                sar ax,1

__set_M_freq:   shl bl,2                ;merge F-NUM(H) with block #
                and ah,00000011b
                or ah,bl
                mov f_num,ax
                jmp __set_freq

__TV_effect:    movzx eax,S_f_val[esi*2];set frequency for TV sound effect
                shr eax,6               ;F-NUMBER = TV freq val = 0-1023
                mov f_num,ax

__set_freq:     mov al,BYTE PTR f_num   ;set F-NUMBER(L)
                invoke send_byte,voice,0a0h,eax

                mov al,BYTE PTR f_num+1 ;get F-NUMBER(H)
                or al,S_BLOCK[esi]      ;merge with KON/BLOCK bits

                mov S_KBF_shadow[esi],al;track value for quick Note Off later
                invoke send_byte,voice,0b0h,eax

__end_freq:     and S_update[esi],NOT U_FREQ
                jmp __FREQ_done

update_voice 	ENDP

;****************************************************************************
update_priority PROC\                   ;Maintain synthesizer voice priority
                USES ebx esi edi
                LOCAL slot_cnt,low_p,high_p,low_4_p
                LOCAL needful

                mov slot_cnt,0          ;zero active slot count

                mov esi,-1              
__get_priority: inc esi                 ;build adjusted priority table and
                cmp esi,NUM_SLOTS       ;reallocate voices if necessary
                je __sort_p_list
                cmp S_status[esi],FREE
                je __get_priority

                inc slot_cnt            ;slot active, bump count

                mov ebx,esi
                mov edi,DWORD PTR S_channel[esi]
                and edi,0fh             ;EDI = slot's MIDI channel
                mov eax,0ffffh           
                cmp MIDI_vprot[edi],64  ;priority = max if voice protection on
                jge __adj_priority       
                movzx eax,WORD PTR S_p_val[ebx][esi]  
__adj_priority: mov ecx,0
                mov cl,MIDI_voices[edi] ;EAX = slot's base priority        
                sub eax,ecx             ;priority -= # of voices in channel
                jnc __set_priority
                mov eax,0
__set_priority: mov S_V_priority[ebx][esi],ax
                jmp __get_priority

__sort_p_list:  mov eax,0               ;set AX = unvoiced highest priority                
                mov edx,0ffffh          ;set DX = voiced lowest priority
                mov ecx,0ffffh          ;set CX = voiced 4-op lowest priority
                mov esi,-1               
__for_slot:     inc esi                  
                cmp esi,NUM_SLOTS
                je __reassign
                cmp S_status[esi],FREE
                je __for_slot
                mov ebx,esi
                movzx edi,WORD PTR S_V_priority[ebx][esi]
                mov bl,S_voice[esi]
                cmp bl,-1      
                jne __chk_low_4
                cmp edi,eax               
                jb __for_slot         
                mov eax,edi              
                mov high_p,esi          ;highest-priority unvoiced slot
                jmp __for_slot
__chk_low_4:    cmp op4_base[ebx],0
                je __chk_low
                cmp edi,ecx
                ja __chk_low
                mov ecx,edi
                mov low_4_p,esi         ;lowest-priority voiced 4-op slot
__chk_low:      cmp edi,edx               
                ja __for_slot
                mov edx,edi               
                mov low_p,esi           ;lowest-priority voiced slot
                jmp __for_slot

__reassign:     cmp eax,edx             ;highest unvoiced < lowest voiced?
                jb __exit               ;yes, we're done

                cmp eax,0               ;if highest unvoiced priority = 0 (or 
                je __exit               ;none), bail out

                mov esi,low_p           ;else steal voice from compatible slot

                IFDEF YMF262
                mov ebx,high_p          ;is new voice a 4-op voice?
                cmp S_type[ebx],OPL3_INST
                jne __rob_voice
                mov esi,low_4_p         ;yes, must rob from 4-op slot
                cmp S_type[esi],OPL3_INST
                je __rob_voice          ;4-op seizes 4-op, continue

                mov al,alt_voice[esi]   ;4-op seizes 2 2-op voices...
                mov edi,-1               
__kill_alt:     inc edi                 ;find slot which owns 2nd half's
                cmp edi,NUM_SLOTS       ;2-op voice (if any) and release it
                je __rob_voice
                cmp S_status[edi],FREE
                je __kill_alt
                cmp S_voice[edi],al
                jne __kill_alt

                invoke release_voice,edi
                ENDIF

__rob_voice:    mov ebx,0
                mov bl,S_voice[esi]    

                push ebx
                invoke release_voice,esi
                pop ebx

                mov esi,high_p
                mov S_voice[esi],bl
                mov edi,ebx
                mov bl,S_channel[esi]
                inc MIDI_voices[ebx]
                mov V_channel[edi],bl
                cmp S_type[esi],OPL3_INST
                jne __do_update
                mov V_channel[edi+3],bl

__do_update:    mov S_update[esi],U_ALL_REGS
                invoke update_voice,esi ;update the hardware

                dec slot_cnt
                jnz __sort_p_list       ;keep sorting until priorities valid

__exit:         ret
update_priority ENDP

;****************************************************************************
                IFDEF YMF262
OPL_phase       PROC\
                USES ebx esi edi\
                ,Slot                       ;Set up 4-op slot parameters

                invoke BNK_phase,[Slot]

                mov esi,[Slot]              ;Get byte ptr
                mov ebx,esi
                shl ebx,1                   ;Convert to word ptr
                mov edi,S_timbre_off[ebx*2] ;Use as dword ptr

                mov S_type[esi],OPL3_INST

                mov al,[edi].BNK.B_fb_c
                and ax,10000000b
                shr eax,6
                or S_FBC[esi],al            ;move 2nd connection bit to S_FBC

                push ebx                    
                movzx ebx,S_FBC[esi]        ;set scaling flags for additive
                mov al,carrier_01[ebx]      ;algorithm (0-3) 
                mov S_scale_01[esi],al      ;operators, based on voice's
                mov al,carrier_23[ebx]
                mov S_scale_23[esi],al
                pop ebx

                mov eax,0                   ;copy key scale/total level values
                mov ah,[edi].OPL3BNK.O_mod_KSLTL
                mov dl,ah
                and dl,11000000b
                mov S_KSLTL_2[esi],dl
                not ah
                and ah,00111111b
                shl eax,2
                mov S_v2_val[ebx],ax

                mov eax,0                    
                mov ah,[edi].OPL3BNK.O_car_KSLTL
                mov dl,ah
                and dl,11000000b
                mov S_KSLTL_3[esi],dl
                not ah
                and ah,00111111b
                shl eax,2
                mov S_v3_val[ebx],ax

                movzx eax,[edi].OPL3BNK.O_mod_AVEKM
                mov ah,al                   ;copy AM/Vib/EG/KSR/Multi bits
                and al,11110000b            
                mov S_AVEKM_2[esi],al
                mov al,0
                shl eax,4
                mov S_m2_val[ebx],ax

                movzx eax,[edi].OPL3BNK.O_car_AVEKM
                mov ah,al
                and al,11110000b            
                mov S_AVEKM_3[esi],al
                mov al,0
                shl eax,4
                mov S_m3_val[ebx],ax

                mov al,[edi].OPL3BNK.O_mod_AD    ;copy envelope parms
                mov S_AD_2[esi],al           
                mov al,[edi].OPL3BNK.O_mod_SR
                mov S_SR_2[esi],al

                mov al,[edi].OPL3BNK.O_car_AD
                mov S_AD_3[esi],al
                mov al,[edi].OPL3BNK.O_car_SR
                mov S_SR_3[esi],al

                mov al,[edi].OPL3BNK.O_car_WS	;copy wave select values
                mov ah,[edi].OPL3BNK.O_mod_WS
                mov S_ws_val_2[ebx],ax

                ret
OPL_phase 	ENDP
                ENDIF

;****************************************************************************
BNK_phase       PROC\
                USES ebx esi edi\
                ,Slot:DWORD		    ;Set up BNK slot parameters

                mov esi,[Slot]
                mov ebx,esi              
                shl ebx,1
                mov edi,S_timbre_off[ebx*2]
                
                mov S_BLOCK[esi],00100000b  ;set KON, clear BLOCK mask

                mov S_type[esi],BNK_INST
                mov S_duration[ebx],-1

                mov S_p_val[ebx],32767      ;BNK instrument priority = average

                mov eax,0
                mov ah,[edi].BNK.B_fb_c     ;copy feedback/connection values
                mov dl,ah
                and dl,00000001b
                mov S_FBC[esi],dl
                mov cl,4
                shl eax,cl
                mov S_fb_val[ebx],ax

                mov eax,0                   ;copy key scale/total level values
                mov ah,[edi].BNK.B_mod_KSLTL
                mov dl,ah
                and dl,11000000b
                mov S_KSLTL_0[esi],dl
                not ah
                and ah,00111111b
                shl eax,1
                shl eax,1
                mov S_v0_val[ebx],ax

                mov eax,0                    
                mov ah,[edi].BNK.B_car_KSLTL
                mov dl,ah
                and dl,11000000b
                mov S_KSLTL_1[esi],dl
                not ah
                and ah,00111111b
                shl eax,1
                shl eax,1
                mov S_v1_val[ebx],ax

                mov al,[edi].BNK.B_mod_AVEKM    ;copy AM/Vib/EG/KSR/Multi bits
                mov ah,al
                and al,11110000b            
                mov S_AVEKM_0[esi],al
                mov al,0
                mov cl,4
                shl eax,cl
                mov S_m0_val[ebx],ax

                mov al,[edi].BNK.B_car_AVEKM
                mov ah,al
                and al,11110000b            
                mov S_AVEKM_1[esi],al
                mov al,0
                mov cl,4
                shl eax,cl
                mov S_m1_val[ebx],ax

                mov al,[edi].BNK.B_mod_AD       ;copy envelope parms
                mov S_AD_0[esi],al           
                mov al,[edi].BNK.B_mod_SR
                mov S_SR_0[esi],al

                mov al,[edi].BNK.B_car_AD
                mov S_AD_1[esi],al
                mov al,[edi].BNK.B_car_SR
                mov S_SR_1[esi],al

                mov al,[edi].BNK.B_car_WS       ;copy wave select values
                mov ah,[edi].BNK.B_mod_WS
                mov S_ws_val[ebx],ax

                mov al,S_FBC[esi]           ;get C-bit (1=additive, 0=FM)
                or al,10b                   ;(2-op carrier always scaled)
                mov S_scale_01[esi],al

                mov S_update[esi],U_ALL_REGS;flag all registers "dirty"

                ret
BNK_phase 	ENDP

;****************************************************************************
note_off        PROC\
                USES ebx esi edi\          ;Turn MIDI note off
                ,Chan:DWORD,Note:DWORD

                mov esi,-1              ;find all slots in which note is
__next_slot:    mov al,BYTE PTR [Note]  ;playing
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

                and ebx,0ffh           ;clear all but bl
                cmp MIDI_sus[ebx],64
                jge __sustained
                
                cmp S_type[esi],OPL3_INST
                je __release_it
                cmp S_type[esi],BNK_INST
                jne __TV_note_off
                
__release_it:   invoke release_voice,esi;release the slot's voice
                mov S_status[esi],FREE

                IFDEF TV_switch_voice   ;if TVFX installed...
                call TV_switch_voice    ;see if a TV slot needs voice
                ENDIF
                jmp __next_slot

__TV_note_off:  mov ebx,esi
                mov S_duration[ebx][esi],1    
                jmp __next_slot         ;set duration to last cycle

__sustained:    mov S_sustain[esi],1
                jmp __next_slot

__exit:         ret

note_off 	ENDP

;****************************************************************************
note_on         PROC\
                USES ebx esi edi\
                ,Chan,Note,Velocity 	;Turn MIDI note on

                mov edi,[Chan]     

                mov ebx,0
                mov bl,MIDI_timbre[edi];get timbre used in channel

                cmp edi,9              ;channel under RBS control?
                jne __set_timbre       ;no, BX=timbre cache index

                mov esi,[Note]         ;else see if RBS entry is valid for
                mov bl,RBS_timbres[esi];this note
                cmp bl,-1
                jne __set_timbre       ;it's valid -- use it

                mov eax,[Note]         ;it's not -- validate RBS entry first
                mov ah,127             ;bank 127 reserved for RBS timbres
                invoke index_timbre,eax
                mov ebx,eax
                mov RBS_timbres[esi],bl

__set_timbre:   cmp bl,-1
                je __exit              ;timbre not loaded, exit
                
                shl ebx,1
                shl ebx,1
                mov edi,cache_base
                add edi,timb_offsets[ebx]   ;else get address of timbre

                add note_event,1            ;update timbre cache LRU counters
                mov eax,note_event
                mov timb_hist[ebx],eax
                    
                mov esi,0               
__find_slot:    cmp S_status[esi],FREE
                je __slot_found
                inc esi                ;find a free virtual voice slot
                cmp esi,NUM_SLOTS
                jne __find_slot
                jmp __exit             ;exit if no virtual voice available

__slot_found:   mov eax,[Chan]         ;establish MIDI channel
                mov S_channel[esi],al   

                mov edx,[Note]
                mov S_keynum[esi],dl   ;save MIDI key #

                mov al,0               ;establish MIDI note/transposition
                mov cl,[edi+2]
                mov ebx,[Chan]
                cmp ebx,9              ;(for all channels except 10)
                je __set_n_txp
                mov al,cl
                mov cl,dl
__set_n_txp:    mov S_note[esi],cl
                mov S_transpose[esi],al

                IF VEL_SENS            
                mov eax,[Velocity]     ;establish note velocity
                IF NOT VEL_TRUE            
                shr al,3
                mov ebx,OFFSET vel_graph      ;scale back velocity sensitivity to 
                xlat [ebx]             ;reduce perceived playback noise
                ENDIF
                ELSE
                mov al,127             ;default velocity = 127
                ENDIF
                mov S_velocity[esi],al

                mov ebx,esi            ;copy timbre address
                shl ebx,1
                shl ebx,1
                mov S_timbre_off[ebx],edi

                mov S_status[esi],KEYON;flag note "on" in slot

                mov S_sustain[esi],0   ;init sustained flag

                movzx eax,[edi].BNK.B_length 
                cmp eax,SIZE OPL3BNK   ;derive timbre type based on size
                je __OPL3BNK
                cmp eax,SIZE BNK
                jne __TVFX_timbre

__BNK:          invoke BNK_phase,esi   ;set up BNK timbre in slot
                jmp __get_voice

__OPL3BNK:      IFDEF YMF262
                invoke OPL_phase,esi
                ENDIF
                jmp __get_voice

__TVFX_timbre:  IFDEF TV_phase         ;if TVFX enabled...
                invoke TV_phase,esi    ;set up TVFX timbre in slot
                ELSE
                jmp __exit
                ENDIF

__get_voice:    mov S_voice[esi],-1
                invoke assign_voice,esi;assign hardware voice to slot

__exit:         ret
note_on 	ENDP

;****************************************************************************
release_sustain PROC\
                USES ebx esi edi\
                ,Chan:DWORD

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
                Stat:DWORD,D1:DWORD,D2:DWORD

                mov esi,[D1]
                and esi,0ffh            ;SI=data 1 / controller #
                mov edi,[Stat]
                mov eax,edi               
                and edi,00fh            ;DI=channel
                and eax,0f0h            ;AX=status
                movzx ecx,BYTE PTR [D2] ;CX=data byte 2

                cmp eax,0b0h             
                je __ctrl_change
                cmp eax,0c0h
                je __program
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

__ctrl_change:  cmp esi,PATCH_BANK_SEL
                je __t_bank
                cmp esi,VOICE_PROTECT
                je __vprot
                cmp esi,TIMBRE_PROTECT
                jne __MIDI

                mov ebx,0
                mov bl,MIDI_timbre[edi]
                cmp bl,-1
                je __exit
                mov al,timb_attribs[ebx]
                and al,10111111b
                cmp cl,64
                jl __tprot
                or al,01000000b
__tprot:        mov timb_attribs[ebx],al
                jmp __exit

__program:      mov eax,esi
                mov MIDI_program[edi],al
                mov ah,MIDI_bank[edi]
                invoke index_timbre,eax
                mov MIDI_timbre[edi],al ;record timbre # used by channel
                jmp __exit              ;(-1 if timbre not in local cache)

__t_bank:       mov MIDI_bank[edi],cl
                jmp __exit

__vprot:        mov MIDI_vprot[edi],cl
                jmp __exit

__MIDI:         mov al,U_AVEKM          ;Emulate MIDI controllers
                mov ebx,OFFSET MIDI_mod         
                cmp esi,MODULATION
                je __MIDI_set

                mov al,U_KSLTL
                mov ebx,OFFSET MIDI_vol
                cmp esi,PART_VOLUME
                je __MIDI_set
                mov ebx,OFFSET MIDI_express
                cmp esi,EXPRESSION
                je __MIDI_set

                IFDEF YMF262            ;FBC registers control stereo on OPL3
                mov al,U_FBC
                ENDIF

                mov ebx,OFFSET MIDI_pan
                cmp esi,PANPOT
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
                mov MIDI_mod[edi],0     ;emulate Roland LAPC-1 RAC message
                mov MIDI_express[edi],127
                mov MIDI_pitch_l[edi],DEF_PITCH_L
                mov MIDI_pitch_h[edi],DEF_PITCH_H
                mov al,U_AVEKM OR U_KSLTL OR U_FREQ
                jmp __flag_updates

send_MIDI_message ENDP

;****************************************************************************
;*                                                                          *
;*  Miscellaneous public (API-accessible) procedures                        *
;*                                                                          *
;****************************************************************************

describe_driver PROC\
                USES ebx esi edi\          ;Return PTR to DDT
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
send_sysex_msg  PROC\
                ,H,AddressA,AddressB,AddressC,Data:PTR,C_Size,C_Wait

                ret
send_sysex_msg 	ENDP

;****************************************************************************
write_display   PROC\               ;Write string to display (unless NULL)
                ,H,String:PTR

                ret
write_display 	ENDP
