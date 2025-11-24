;�����������������������������������������������������������������������������
;��                                                                         ��
;��  AIL32.ASM                                                              ��
;��                                                                         ��
;��  IBM Audio Interface Library API module for 32-bit DPMI (AIL/32)        ��
;��                                                                         ��
;��  Version 1.00 of 29-Jul-92: 32-bit conversion (Rational Systems DOS/4G) ��
;��          1.01 of  4-Apr-93: SS: override added in call_driver           ��
;��          1.02 of  1-May-93: Flashtek X32 compatibility added            ��
;��          1.03 of 11-Jul-93: EARLY_EOI option added for use w/interrupt- ��
;��                             critical environments                       ��
;��          1.04 of 27-Aug-93: Watcom debugger support enabled             ��
;��                                                                         ��
;��  80386 ASM source compatible with Microsoft Assembler v6.0 or later     ��
;��  Author: John Miles (32-bit flat model conversion by John Lemberger)    ��                            ��
;��                                                                         ��
;�����������������������������������������������������������������������������
;��                                                                         ��
;��  Copyright (C) 1991-1993 Miles Design, Inc.                             ��
;��                                                                         ��
;��  Miles Design, Inc.                                                     ��
;��  6702 Cat Creek Trail                                                   ��
;��  Austin, TX 78731                                                       ��
;��  (512) 345-2642 / FAX (512) 338-9630 / BBS (512) 454-9990               ��
;��                                                                         ��
;�����������������������������������������������������������������������������

                OPTION SCOPED           ;Enable local labels
                .386                    ;Enable 386 instruction set

                IFDEF ZORTECH

                .MODEL SMALL,C          ;Small 32-bit memory model and C calls
DGROUP          group _TEXT,_DATA       ;(Zortech's BLINK needs this)

                ELSE

                .MODEL FLAT,C

                ENDIF

                ;
                ;Configuration equates
                ;

FALSE           equ 0
TRUE            equ -1

EARLY_EOI       equ TRUE                ;TRUE to send EOI at beginning of IRQ
                                        ;(for interrupt-critical environments)

                ;
                ;Macros, internal equates
                ;
                
HOOK_INT        equ 8                   ;timer tick interrupt
CURRENT_REV     equ 215                 ;current API revision level

                INCLUDE ail32.inc       ;define driver procedure call numbers
                INCLUDE 386.mac         ;DOS extender macros

TIMER_TYPE 	TYPEDEF DWORD

AIL_release_timer_handle PROTO C,Timer:TIMER_TYPE
AIL_set_timer_period	 PROTO C,Timer:TIMER_TYPE,uS:DWORD
AIL_start_timer		 PROTO C,Timer:TIMER_TYPE

                ;
                ;Local data
                ;

                .DATA

BIOS_H          equ 16                  ;Handle to BIOS default timer

active_timers   dw 0                    ;# of timers currently registered
timer_busy      dw 0                    ;Reentry flag for INT 8 handler

timer_callback  dd 17 dup (0)           ;Callback function addrs for timers
timer_status    dw 17 dup (0)           ;Status of timers (0=free 1=off 2=on)
timer_elapsed   dd 17 dup (0)           ;Modified DDA error counts for timers
timer_value     dd 17 dup (0)           ;Modified DDA limit values for timers
timer_period    dd 0                    ;Modified DDA increment for timers

bios_cb         dd 0
bios_cb_cs      dw 0  
bios_cb_real    dd 0
current_timer   dd 0                    
temp_period     dd 0
PIT_divisor     dd 0

index_base      dd 16 dup (0)           ;Driver table base addresses
assigned_timer  dd 16 dup (0)           ;Timers assigned to drivers
driver_active   dd 16 dup (0)

drvproc         dd 0
cur_drvr        dd 0
rtn_off         dw 0
rtn_seg         dw 0
timer_handle    dd 0

local_DS        dw 0

                ALIGN 4
                dd 256 dup (?)         ;1024-byte interrupt stack used by
intstack        LABEL DWORD            ;default -- can be increased if needed

old_ss          dd ?
old_sp          dd ?

drvr_desc       STRUC
min_API_version dd ?
drvr_type       dd ?
data_suffix     db 4 dup (?)
dev_names       dd ?
def_IO          dd ?
def_IRQ         dd ?
def_DMA         dd ?
def_DRQ         dd ?
svc_rate        dd ?
dsp_size        dd ?
drvr_desc	ENDS

                .CODE

                ;
                ;Process services
                ;                

                PUBLIC AIL_startup
                PUBLIC AIL_shutdown
                PUBLIC AIL_register_timer
                PUBLIC AIL_set_timer_period
                PUBLIC AIL_set_timer_frequency
                PUBLIC AIL_set_timer_divisor
                PUBLIC AIL_interrupt_divisor
                PUBLIC AIL_start_timer
                PUBLIC AIL_start_all_timers
                PUBLIC AIL_stop_timer
                PUBLIC AIL_stop_all_timers
                PUBLIC AIL_release_timer_handle
                PUBLIC AIL_release_all_timers

                ;
                ;Installation services
                ;

                PUBLIC AIL_register_driver
                PUBLIC AIL_release_driver_handle
                PUBLIC AIL_describe_driver
                PUBLIC AIL_detect_device
                PUBLIC AIL_init_driver
                PUBLIC AIL_shutdown_driver
                
                ;
                ;Extended MIDI (XMIDI) performance services
                ;

                PUBLIC AIL_state_table_size
                PUBLIC AIL_register_sequence
                PUBLIC AIL_release_sequence_handle

                PUBLIC AIL_default_timbre_cache_size
                PUBLIC AIL_define_timbre_cache
                PUBLIC AIL_timbre_request
                PUBLIC AIL_install_timbre
                PUBLIC AIL_protect_timbre
                PUBLIC AIL_unprotect_timbre
                PUBLIC AIL_timbre_status

                PUBLIC AIL_start_sequence
                PUBLIC AIL_stop_sequence
                PUBLIC AIL_resume_sequence
                PUBLIC AIL_sequence_status
                PUBLIC AIL_relative_volume
                PUBLIC AIL_relative_tempo
                PUBLIC AIL_set_relative_volume
                PUBLIC AIL_set_relative_tempo
                PUBLIC AIL_beat_count
                PUBLIC AIL_measure_count
                PUBLIC AIL_branch_index

                PUBLIC AIL_controller_value
                PUBLIC AIL_set_controller_value
                PUBLIC AIL_channel_notes
                PUBLIC AIL_send_channel_voice_message
                PUBLIC AIL_send_sysex_message
                PUBLIC AIL_write_display
                PUBLIC AIL_install_callback
                PUBLIC AIL_cancel_callback

                PUBLIC AIL_lock_channel
                PUBLIC AIL_map_sequence_channel
                PUBLIC AIL_true_sequence_channel
                PUBLIC AIL_release_channel

                ;
                ;Digital performance services
                ;

                PUBLIC AIL_index_VOC_block
                PUBLIC AIL_register_sound_buffer
                PUBLIC AIL_format_sound_buffer
                PUBLIC AIL_sound_buffer_status
                PUBLIC AIL_play_VOC_file
                PUBLIC AIL_format_VOC_file
                PUBLIC AIL_VOC_playback_status
                PUBLIC AIL_start_digital_playback
                PUBLIC AIL_stop_digital_playback
                PUBLIC AIL_pause_digital_playback
                PUBLIC AIL_resume_digital_playback
                PUBLIC AIL_set_digital_playback_volume
                PUBLIC AIL_digital_playback_volume
                PUBLIC AIL_set_digital_playback_panpot
                PUBLIC AIL_digital_playback_panpot

;*****************************************************************************
;*                                                                           *
;* Internal procedures                                                       *
;*                                                                           *
;*****************************************************************************

find_proc       PROC                    ;Return addr of function AX in driver
                                        ;DX (Note: reentrant!)
                cmp edx,16
                jae __bad_handle        ;exit sanely if handle invalid
                shl edx,1               ;(a legitimate action for apps)
                shl edx,1
                mov edx,index_base[edx] ;EDX -> driver procedure table
                cmp edx,0
                jz __bad_handle         ;handle -> unreg'd driver, exit

__find_proc:    mov ecx,[edx]           ;search for requested function in 
                cmp ecx,eax             ;driver procedure table
                je __found
                add edx,8
                cmp ecx,-1               
                jne __find_proc
                                        ;return 0: function not available
__bad_handle:   mov eax,0               ;return 0: invalid driver handle
                ret                                                     

__found:        mov eax,[edx+4]         ;get offset from start of driver
                ret

find_proc	ENDP

;*****************************************************************************
call_driver     PROC                    ;Call function AX in specified driver
                                        ;(Warning: re-entrant procedure!)
                mov edx,esp
                mov edx,ss:[edx+4]      ;get handle

                call find_proc

                cmp eax,0
                je __invalid_call

                jmp eax                 ;call driver function

__invalid_call: ret                     ;return DX:AX = 0 if call failed

call_driver	ENDP

;*****************************************************************************
API_timer       PROC                    ;API INT 8 time-slice dispatcher

                pushad  
                push ds
                push es
                push fs
                push gs
                mov ebp,esp
                cld

                IF EARLY_EOI
                mov al,20h       
                out 20h,al
                sti
                ENDIF

                mov ds,cs:local_DS
                mov es,cs:local_DS

                cmp timer_busy,0
                je __no_reentry
                jmp __end_of_IRQ        

__no_reentry:   mov timer_busy,1        ;avoid re-entry or undesirable calls
                                
                mov old_ss,ss           ;switch to internal stack
                mov old_sp,esp
                mov ax,ds               
                mov ss,ax              
                mov esp,OFFSET intstack

                mov current_timer,0
__for_timer:    mov esi,current_timer   ;for timer = 0 to 16
                shl esi,1
                cmp timer_status[esi],2 ;is timer "running"?
                jne __next_timer        ;no, go on to the next one
                
                shl esi,1
                mov eax,timer_elapsed[esi]
                add eax,timer_period
                cmp eax,timer_value[esi]
                jae __timer_tick

__dec_timer:    mov timer_elapsed[esi],eax
                jmp __next_timer
                
__timer_tick:   sub eax,timer_value[esi]
                mov timer_elapsed[esi],eax

                cmp current_timer,16    ;DDA timer expired, call timer proc
                jb __do_callback

                mov timer_busy,0        ;(won't return from BIOS chain)
                mov ss,old_ss
                mov esp,old_sp

__do_callback:  call timer_callback[esi]

__next_timer:   inc current_timer       ;(may be externally set to -1 to 
                cmp current_timer,16    ; cancel further callbacks)
                jbe __for_timer

                mov timer_busy,0
                mov ss,old_ss
                mov esp,old_sp

__end_of_IRQ:   mov esp,ebp
                pop gs
                pop fs
                pop es
                pop ds
                popad   

                IF NOT EARLY_EOI
                push eax
                mov al,20h       
                out 20h,al
                pop eax
                ENDIF

                iretd

API_timer	ENDP

;*****************************************************************************
bios_caller     PROC C                  ;Call old INT8 handler to service BIOS

                IFDEF PHARLAP

                mov eax,250eh
                mov ebx,bios_cb_real
                mov ecx,2
                pushf
                int 21h

                mov esp,ebp             ;EBP -> interrupt stack frame
                pop gs
                pop fs
                pop es
                pop ds
                popad
                iretd

                ELSE

                mov ecx,bios_cb         ;ECX = 32-bit offset
                movzx eax,bios_cb_cs    ;EAX = 16-bit selector

                mov esp,ebp             ;EBP -> interrupt stack frame

                xchg ecx,[ebp+28H]
                xchg eax,[ebp+2cH]

                pop gs
                pop fs
                pop es
                pop ds
                pop edi
                pop esi
                pop ebp
                pop ebx
                pop ebx
                pop edx
                retf

                ENDIF

bios_caller	ENDP

;*****************************************************************************
init_DDA_arrays PROC C \                ;Initialize timer DDA counters
                USES ebx esi edi es

                pushfd
                cli

                push ds
                pop es

                cld
                mov timer_period,-1
                
                mov edi,OFFSET timer_status
                mov ecx,17
                mov eax,0
                rep stosw         ;mark all timer handles "free"

                mov edi,OFFSET timer_elapsed
                mov ecx,17
                rep stosd
                                                        
                mov edi,OFFSET timer_value
                mov ecx,17
                rep stosd

                POP_F
                ret
init_DDA_arrays ENDP

;*****************************************************************************
hook_timer_process PROC C \             ;Take over default BIOS INT 8 handler
                USES ebx esi edi

                pushfd
                cli

                mov eax,HOOK_INT         ;get current INT 8 vector and save it
                GET_VECT                 ;as reserved timer function (stopped)
                mov bios_cb,ebx      
                mov bios_cb_cs,dx
                mov bios_cb_real,ecx

                mov ebx,OFFSET bios_caller
                mov timer_callback[BIOS_H*4],ebx

                mov eax,HOOK_INT         ;replace default handler with API task
                mov edx,OFFSET API_timer ;manager
                mov bx,cs
                SET_VECT

                POP_F
                ret
hook_timer_process ENDP

;*****************************************************************************
unhook_timer_process PROC C \           ;Restore default BIOS INT 8 handler
                USES ebx esi edi

                pushfd
                cli

                mov current_timer,-1    ;disallow any further callbacks

                mov eax,HOOK_INT
                mov ebx,bios_cb_real
                SET_REAL_VECT

                mov eax,HOOK_INT
                mov edx,bios_cb
                movzx ebx,bios_cb_cs
                SET_PROT_VECT

                POP_F
                ret
unhook_timer_process ENDP

;*****************************************************************************
set_PIT_divisor PROC C \		;Set 8253 Programmable Interval Timer
                USES ebx esi edi \      ;to desired IRQ 0 (INT 8) interval
                ,Divisor:DWORD

                pushfd
                cli

                mov al,36h
                out 43h,al
	mov eax,[Divisor]       ;PIT granularity = 1/1193181 sec.
                mov PIT_divisor,eax
                jmp $+2
                out 40h,al
                mov al,ah
                jmp $+2
                out 40h,al

                POP_F
                ret
set_PIT_divisor ENDP

;*****************************************************************************
set_PIT_period	PROC C \   	        ;Set 8253 Programmable Interval Timer
                USES ebx esi edi\	;to desired period in microseconds
                ,Period:DWORD

                mov eax,0               ;special case: no rounding error
                cmp [Period],54925      ;if period=55 msec. BIOS default value
                jae __set_PIT

                mov eax,[Period]
                mov ebx,8380            ;PIT granularity = .83809532 uS
                mov ecx,10000           ;round down to avoid cumulative error
                mul ecx
                div ebx

__set_PIT:      invoke set_PIT_divisor,eax

                ret
set_PIT_period	ENDP

;*****************************************************************************
program_timers	PROC C \                ;Establish timer interrupt rates
                USES ebx esi edi es     ;based on fastest active timer

                pushfd
                cli                     ;non-reentrant, don't interrupt

                cld
                mov temp_period,-1

                mov esi,0
__for_timer:    mov ebx,esi             ;find fastest active timer....
                shl ebx,1
                cmp timer_status[ebx],0 ;timer active (registered)?
                je __next_timer         ;no, skip it
                mov eax,timer_value[ebx*2]

                cmp eax,temp_period
                jae __next_timer

__set_temp:     mov temp_period,eax

__next_timer:   inc esi
                cmp esi,16              ;(include BIOS reserved timer)
                jbe __for_timer        

                mov eax,temp_period

                cmp eax,timer_period
                je __no_change          ;current rate hasn't changed, exit

__must_reset:   mov current_timer,-1    ;else set new base timer rate
                                        ;(slowest possible base = 54 msec!)
                mov timer_period,eax

                invoke set_PIT_period,eax

                mov edi,OFFSET timer_elapsed
                mov ecx,17
                mov eax,0               ;reset ALL elapsed counters to 0 uS

                push ds
                pop es

                rep stosd
__no_change:    
                POP_F
                ret
program_timers	ENDP

;*****************************************************************************
;*                                                                           *
;* Process services                                                          *
;*                                                                           *
;*****************************************************************************

AIL_startup	PROC \                  ;Initialize AIL API
                USES ebx esi edi es

                pushfd
                cli

                mov local_DS,ds         ;*** WARNING: Assumes CS = DS! ***
                mov es,local_DS

                mov active_timers,0     ;# of registered timers
                mov timer_busy,0        ;timer re-entrancy protection

                cld
                mov edi,OFFSET index_base
                mov ecx,16
                mov eax,0
                rep stosd

                mov edi,OFFSET assigned_timer
                mov ecx,16
                mov eax,-1
                rep stosd

                mov edi,OFFSET driver_active
                mov ecx,16
                mov eax,0
                rep stosd

                call init_DDA_arrays    ;init timer countdown values

                POP_F    
                ret
AIL_startup	ENDP

;*****************************************************************************
AIL_shutdown	PROC \                  ;Quick shutdown of all AIL resources
                USES ebx esi edi \
                ,SignOff:PTR

                pushfd
                cli

                mov cur_drvr,0
__for_slot:     mov esi,cur_drvr
                shl esi,2
                mov edx,assigned_timer[esi]
                mov eax,index_base[esi]   
                cmp eax,0
                jz __next_slot          ;no driver installed, skip slot

                cmp edx,-1
                je __shut_down          ;no timer assigned, continue
                invoke AIL_release_timer_handle,edx

__shut_down:	mov ebx,[SignOff]
                push ebx
                push cur_drvr
                call AIL_shutdown_driver
                add esp,8

__next_slot:    inc cur_drvr
                cmp cur_drvr,16
                jne __for_slot

                call AIL_release_all_timers

                POP_F
                ret
AIL_shutdown	ENDP

;*****************************************************************************
AIL_register_timer PROC C \
                USES ebx esi edi \
                ,Callback:DWORD

                pushfd
                cli

                mov ebx,0               ;look for a free timer handle....
__find_free:    cmp WORD PTR timer_status[ebx],0
                je __found              ;found one
                add ebx,2
                cmp ebx,32
                jb __find_free
                mov eax,-1
                jmp __return            ;no free timers, return -1

__found:        mov eax,ebx             ;yes, set up to return handle
                shr eax,1
                mov timer_status[ebx],1 ;turn the new timer "off" (stopped)
	mov esi,[Callback]	;register the timer proc
                shl ebx,1
                mov timer_callback[ebx],esi
                mov timer_value[ebx],-1
                inc active_timers
                cmp active_timers,1     ;is this the first timer registered?
                jne __return            ;no, just return

                push eax                ;yes, set up our own INT 8 handler
                call init_DDA_arrays    ;init timer countdown values
                mov timer_status[BIOS_H*2],1
                call hook_timer_process ;seize INT 8 and register BIOS handler
                invoke AIL_set_timer_period,BIOS_H,54925
                invoke AIL_start_timer,BIOS_H
                pop eax

                mov ebx,eax
                shl ebx,1
                mov timer_status[ebx],1 ;(cleared by init_DDA_arrays)
                shl ebx,1
                mov timer_value[ebx],-1

__return:                               
                POP_F
                ret
AIL_register_timer ENDP

;*****************************************************************************
AIL_release_timer_handle PROC C \
                USES ebx esi edi \
                ,Timer:TIMER_TYPE

                pushfd
                cli

                mov ebx,[Timer]
                cmp ebx,-1
                je __return

                shl ebx,1
                cmp timer_status[ebx],0 ;is the specified timer active?
                je __return             ;no, exit

                mov timer_status[ebx],0 ;release the timer's handle
                
                dec active_timers       ;any active timers left?
                jnz __return            ;if not, put the default handler back

                invoke set_PIT_divisor,0
                call unhook_timer_process             
__return:       
                POP_F
                ret
AIL_release_timer_handle ENDP

;*****************************************************************************
AIL_release_all_timers PROC C \
                USES ebx esi edi

                pushfd
                cli

                mov esi,15              ;free all external timer handles
__release_it:	invoke AIL_release_timer_handle,esi
                dec esi
                jge __release_it

                POP_F
                ret
AIL_release_all_timers ENDP

;*****************************************************************************
AIL_start_timer PROC C \
                USES ebx esi edi \
                ,Timer:TIMER_TYPE

                pushfd
                cli

                mov ebx,[Timer]
                shl ebx,1
                cmp timer_status[ebx],1 ;is the specified timer stopped?
                jne __return
                mov timer_status[ebx],2 ;yes, start it
__return:       
                POP_F
                ret
AIL_start_timer ENDP

;*****************************************************************************
AIL_start_all_timers PROC C \
                USES ebx esi edi

                pushfd 
                cli

                mov esi,15              ;start all stopped timers
__start_it:	invoke AIL_start_timer,esi
                dec esi
                jge __start_it

                POP_F
                ret
AIL_start_all_timers ENDP

;*****************************************************************************
AIL_stop_timer	PROC C \
                USES ebx esi edi \
                ,Timer:TIMER_TYPE

                pushfd
                cli

                mov ebx,[Timer]
                shl ebx,1
                cmp timer_status[ebx],2 ;is the specified timer running?
                jne __return
                mov timer_status[ebx],1 ;yes, stop it
__return:       
                POP_F
                ret
AIL_stop_timer	ENDP

;*****************************************************************************
AIL_stop_all_timers PROC C \
                USES ebx esi edi

                pushfd
                cli

                mov esi,15              ;stop all running timers
__stop_it:	invoke AIL_stop_timer,esi
                dec esi
                jge __stop_it

                POP_F
                ret
AIL_stop_all_timers ENDP

;*****************************************************************************
AIL_set_timer_period PROC C \
                USES ebx esi edi \      ;accepts timer period in microseconds
                ,Timer:TIMER_TYPE,uS:DWORD

                pushfd
                cli

                mov ebx,[Timer]
                shl ebx,1               ;save timer's status
                movzx eax,WORD PTR timer_status[ebx] 
                push eax
                mov timer_status[ebx],1 ;stop timer while calculating...

                shl ebx,1
	mov eax,[uS]
                mov timer_value[ebx],eax

                mov timer_elapsed[ebx],0

                call program_timers     ;reset base interrupt rate if needed

                pop eax
                mov ebx,[Timer]         ;restore timer's former status
                shl ebx,1
                mov WORD PTR timer_status[ebx],ax 

                POP_F
                ret
AIL_set_timer_period ENDP

;*****************************************************************************
AIL_set_timer_frequency PROC C \
                USES ebx esi edi \      ;accepts timer frequency in Hertz
                ,Timer:TIMER_TYPE,Hz:DWORD

                pushfd
                cli

                mov edx,0
                mov eax,0f4240h
                mov ebx,[Hz]
                div ebx

                invoke AIL_set_timer_period,[Timer],eax

                POP_F
                ret
AIL_set_timer_frequency ENDP

;*****************************************************************************
AIL_set_timer_divisor PROC C \
                USES ebx esi edi\       ;accepts PIT register values directly
                ,Timer:TIMER_TYPE,PIT:DWORD

                pushfd
                cli

                cmp [PIT],0             ;special case: 0 wraps to 65536
                jne __nonzero
                mov eax,54925      
                jmp __set_AXDX

__nonzero:      mov eax,10000           ;convert to microseconds
                mov edx,0
                mov ebx,11932
                mul [PIT]
                div ebx                 ;(accurate to �.01%)

__set_AXDX:	invoke AIL_set_timer_period,[Timer],eax

                POP_F
                ret
AIL_set_timer_divisor ENDP

;*****************************************************************************
AIL_interrupt_divisor PROC C \ 		;Get value last used by the API to
                USES ebx esi edi	        ;program the PIT chip

                pushfd
                cli

                mov eax,PIT_divisor

                POP_F
                ret
AIL_interrupt_divisor ENDP

;*****************************************************************************
;*                                                                           *
;* Installation services                                                     *
;*                                                                           *
;*****************************************************************************

AIL_register_driver PROC C \
                USES ebx esi edi\
	,Address:PTR

                pushfd
                cli
     
                mov cur_drvr,0
__find_handle:  mov esi,cur_drvr
                shl esi,1
                shl esi,1
                mov eax,index_base[esi]
                cmp eax,0
                je __found
                inc cur_drvr
                cmp cur_drvr,16
                jne __find_handle
                mov eax,-1              ;return -1 if no free handles
                jmp __return

__found:	mov edi,[Address]       ;get driver base address

__check_ADV:    mov eax,-1              ;else check for copyright string to
                cmp DWORD PTR [edi+4],'ypoC'	
                jne __return            ;avoid calling non-AIL drivers

                mov edi,[edi]           ;skip copyright message text
                mov index_base[esi],edi

                push cur_drvr
                call AIL_describe_driver
                add esp,4

                mov edi,eax             ;check API version compatibility
                cmp eax,0
                mov eax,-1
                je __return             ;return -1 if description call failed

                mov edx,[edi].drvr_desc.min_API_version

                cmp edx,CURRENT_REV
                ja __return             ;return -1 if API out of date

__valid_handle: mov eax,cur_drvr        ;else return AX=new driver handle

__return:       POP_F
                ret

AIL_register_driver ENDP

;*****************************************************************************
AIL_release_driver_handle PROC C \
                USES ebx esi edi \
                ,H:DWORD

                pushfd
                cli

                mov ebx,[H]
                cmp ebx,16
                jae __exit              ;exit cleanly if invalid handle passed
                shl ebx,1
                shl ebx,1
                mov index_base[ebx],0
        
__exit:         POP_F
                ret
AIL_release_driver_handle ENDP

;*****************************************************************************
AIL_describe_driver PROC C \
                USES ebx esi edi \
                ,HDrvr:DWORD

                mov eax,OFFSET AIL_interrupt_divisor
                push eax
                push [HDrvr]
                mov eax,AIL_DESC_DRVR
                call call_driver
                add esp,8
                ret

AIL_describe_driver ENDP

;*****************************************************************************
AIL_detect_device PROC

                mov eax,AIL_DET_DEV
                jmp call_driver

AIL_detect_device ENDP

;*****************************************************************************
AIL_init_driver PROC C \
                USES ebx esi edi \
                ,HDrvr,IO:DWORD,IRQ:DWORD,DMA:DWORD,DRQ:DWORD

                pushfd
                cli

                cmp [HDrvr],16
                jae __return            ;exit cleanly if invalid handle passed

                mov timer_handle,-1

                push [HDrvr]
                call AIL_describe_driver
                add esp,4

                mov edi,eax
                mov esi,[edi].drvr_desc.svc_rate
                                        ;get desired service rate
                cmp esi,-1
                je __do_init            ;(no timer service requested)

                mov eax,AIL_SERVE_DRVR 
                mov edx,[HDrvr]
                call find_proc

                mov ebx,eax
                cmp ebx,0
                jz __do_init            ;(no driver service proc)

                mov edi,eax  		;EDI = serve_driver() address

                invoke AIL_register_timer,edi
                mov ebx,[HDrvr]
                shl ebx,1           
                shl ebx,1           
                mov assigned_timer[ebx],eax
                mov timer_handle,eax

                invoke AIL_set_timer_frequency,timer_handle,esi

__do_init:      push DRQ
                push DMA
                push IRQ
                push IO
                push HDrvr
                mov eax,AIL_INIT_DRVR
                call call_driver
                add esp,20

                mov ebx,[HDrvr]
                shl ebx,1           
                shl ebx,1
                mov driver_active[ebx],1

                cmp timer_handle,-1
                je __return
                invoke AIL_start_timer,timer_handle

__return:       POP_F
                ret

AIL_init_driver ENDP

;*****************************************************************************
AIL_shutdown_driver PROC C

                mov ebx,esp
                mov ebx,[ebx+4]         ;get handle
                cmp ebx,16
                jae __exit              ;exit cleanly if invalid handle passed

                shl ebx,1
                shl ebx,1
                mov edx,0
                xchg driver_active[ebx],edx
                cmp edx,0
                je __exit               ;driver never initialized, exit
                mov edx,assigned_timer[ebx]
                cmp edx,-1
                je __shut_down          ;no timer assigned, continue
                invoke AIL_release_timer_handle,edx

__shut_down:    mov eax,AIL_SHUTDOWN_DRVR
                jmp call_driver

__exit:         ret
AIL_shutdown_driver ENDP

;*****************************************************************************
;*                                                                           *
;* Performance services                                                      *
;*                                                                           *
;*****************************************************************************

AIL_index_VOC_block PROC

                mov eax,AIL_INDEX_VOC_BLK  
                jmp call_driver

AIL_index_VOC_block ENDP

;*****************************************************************************
AIL_register_sound_buffer PROC

                mov eax,AIL_REG_SND_BUFF  
                jmp call_driver

AIL_register_sound_buffer ENDP

;*****************************************************************************
AIL_format_sound_buffer PROC

                mov eax,AIL_F_SND_BUFF  
                jmp call_driver

AIL_format_sound_buffer ENDP

;*****************************************************************************
AIL_sound_buffer_status PROC

                mov eax,AIL_SND_BUFF_STAT 
                jmp call_driver

AIL_sound_buffer_status ENDP

;*****************************************************************************
AIL_play_VOC_file  PROC

                mov eax,AIL_P_VOC_FILE 
                jmp call_driver

AIL_play_VOC_file ENDP

;*****************************************************************************
AIL_format_VOC_file PROC

                mov eax,AIL_F_VOC_FILE 
                jmp call_driver

AIL_format_VOC_file ENDP

;*****************************************************************************
AIL_VOC_playback_status PROC

                mov eax,AIL_VOC_PB_STAT   
                jmp call_driver

AIL_VOC_playback_status ENDP

;*****************************************************************************
AIL_start_digital_playback PROC

                mov eax,AIL_START_D_PB    
                jmp call_driver

AIL_start_digital_playback ENDP

;*****************************************************************************
AIL_stop_digital_playback PROC

                mov eax,AIL_STOP_D_PB     
                jmp call_driver

AIL_stop_digital_playback ENDP

;*****************************************************************************
AIL_pause_digital_playback PROC

                mov eax,AIL_PAUSE_D_PB    
                jmp call_driver

AIL_pause_digital_playback ENDP

;*****************************************************************************
AIL_resume_digital_playback PROC

                mov eax,AIL_RESUME_D_PB   
                jmp call_driver

AIL_resume_digital_playback ENDP

;*****************************************************************************
AIL_set_digital_playback_volume PROC

                mov eax,AIL_SET_D_PB_VOL  
                jmp call_driver

AIL_set_digital_playback_volume ENDP

;*****************************************************************************
AIL_digital_playback_volume PROC

                mov eax,AIL_D_PB_VOL      
                jmp call_driver

AIL_digital_playback_volume ENDP

;*****************************************************************************
AIL_set_digital_playback_panpot PROC

                mov eax,AIL_SET_D_PB_PAN  
                jmp call_driver

AIL_set_digital_playback_panpot ENDP

;*****************************************************************************
AIL_digital_playback_panpot PROC

                mov eax,AIL_D_PB_PAN      
                jmp call_driver

AIL_digital_playback_panpot ENDP

;*****************************************************************************
AIL_state_table_size PROC

                mov eax,AIL_STATE_TAB_SIZE
                jmp call_driver

AIL_state_table_size ENDP

;*****************************************************************************
AIL_register_sequence PROC

                mov eax,AIL_REG_SEQ       
                jmp call_driver

AIL_register_sequence ENDP

;*****************************************************************************
AIL_release_sequence_handle PROC

                mov eax,AIL_REL_SEQ_HND   
                jmp call_driver

AIL_release_sequence_handle ENDP

;*****************************************************************************
AIL_default_timbre_cache_size PROC

                mov eax,AIL_T_CACHE_SIZE  
                jmp call_driver

AIL_default_timbre_cache_size ENDP

;*****************************************************************************
AIL_define_timbre_cache PROC

                mov eax,AIL_DEFINE_T_CACHE
                jmp call_driver

AIL_define_timbre_cache ENDP

;*****************************************************************************
AIL_timbre_request PROC

                mov eax,AIL_T_REQ         
                jmp call_driver

AIL_timbre_request ENDP

;*****************************************************************************
AIL_install_timbre PROC

                mov eax,AIL_INSTALL_T     
                jmp call_driver

AIL_install_timbre ENDP

;*****************************************************************************
AIL_protect_timbre PROC

                mov eax,AIL_PROTECT_T     
                jmp call_driver

AIL_protect_timbre ENDP

;*****************************************************************************
AIL_unprotect_timbre PROC

                mov eax,AIL_UNPROTECT_T   
                jmp call_driver

AIL_unprotect_timbre ENDP

;*****************************************************************************
AIL_timbre_status  PROC

                mov eax,AIL_T_STATUS
                jmp call_driver

AIL_timbre_status ENDP

;*****************************************************************************
AIL_start_sequence PROC

                mov eax,AIL_START_SEQ     
                jmp call_driver

AIL_start_sequence ENDP

;*****************************************************************************
AIL_stop_sequence  PROC

                mov eax,AIL_STOP_SEQ      
                jmp call_driver

AIL_stop_sequence ENDP

;*****************************************************************************
AIL_resume_sequence PROC

                mov eax,AIL_RESUME_SEQ    
                jmp call_driver

AIL_resume_sequence ENDP

;*****************************************************************************
AIL_sequence_status PROC

                mov eax,AIL_SEQ_STAT      
                jmp call_driver

AIL_sequence_status ENDP

;*****************************************************************************
AIL_relative_volume PROC

                mov eax,AIL_REL_VOL       
                jmp call_driver

AIL_relative_volume ENDP

;*****************************************************************************
AIL_relative_tempo PROC

                mov eax,AIL_REL_TEMPO
                jmp call_driver

AIL_relative_tempo ENDP

;*****************************************************************************
AIL_set_relative_volume PROC

                mov eax,AIL_SET_REL_VOL       
                jmp call_driver

AIL_set_relative_volume ENDP

;*****************************************************************************
AIL_set_relative_tempo PROC

                mov eax,AIL_SET_REL_TEMPO  
                jmp call_driver
                        
AIL_set_relative_tempo ENDP

;*****************************************************************************
AIL_beat_count     PROC

                mov eax,AIL_BEAT_CNT      
                jmp call_driver

AIL_beat_count	   ENDP

;*****************************************************************************
AIL_measure_count  PROC

                mov eax,AIL_BAR_CNT       
                jmp call_driver

AIL_measure_count  ENDP

;*****************************************************************************
AIL_branch_index   PROC

                mov eax,AIL_BRA_INDEX  
                jmp call_driver

AIL_branch_index   ENDP

;*****************************************************************************
AIL_controller_value PROC

                mov eax,AIL_CON_VAL       
                jmp call_driver

AIL_controller_value ENDP

;*****************************************************************************
AIL_set_controller_value PROC

                mov eax,AIL_SET_CON_VAL   
                jmp call_driver

AIL_set_controller_value ENDP

;*****************************************************************************
AIL_channel_notes  PROC

                mov eax,AIL_CHAN_NOTES    
                jmp call_driver

AIL_channel_notes  ENDP

;*****************************************************************************
AIL_send_channel_voice_message PROC

                mov eax,AIL_SEND_CV_MSG   
                jmp call_driver

AIL_send_channel_voice_message ENDP

;*****************************************************************************
AIL_send_sysex_message PROC

                mov eax,AIL_SEND_SYSEX_MSG
                jmp call_driver

AIL_send_sysex_message ENDP

;*****************************************************************************
AIL_write_display  PROC

                mov eax,AIL_WRITE_DISP    
                jmp call_driver

AIL_write_display  ENDP

;*****************************************************************************
AIL_install_callback PROC

                mov eax,AIL_INSTALL_CB    
                jmp call_driver

AIL_install_callback ENDP

;*****************************************************************************
AIL_cancel_callback PROC

                mov eax,AIL_CANCEL_CB     
                jmp call_driver

AIL_cancel_callback ENDP

;*****************************************************************************
AIL_lock_channel   PROC

                mov eax,AIL_LOCK_CHAN     
                jmp call_driver

AIL_lock_channel   ENDP

;*****************************************************************************
AIL_map_sequence_channel PROC

                mov eax,AIL_MAP_SEQ_CHAN  
                jmp call_driver

AIL_map_sequence_channel ENDP

;*****************************************************************************
AIL_release_channel PROC

                mov eax,AIL_RELEASE_CHAN  
                jmp call_driver

AIL_release_channel ENDP

;*****************************************************************************
AIL_true_sequence_channel PROC

                mov eax,AIL_TRUE_SEQ_CHAN  
                jmp call_driver

AIL_true_sequence_channel ENDP

;*****************************************************************************
                END
