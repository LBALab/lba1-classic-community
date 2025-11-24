###############################################################
#                                                             #
#  MAKEFILE for AIL/32 development                            #             
#  10-Aug-92 John Miles                                       #
#                                                             #
#  This file builds drivers and sample applications for use   #
#  with Watcom C++ and Rational Systems DOS/4GW               #
#                                                             #
#  Execute with Microsoft (or compatible) MAKE                #
#                                                             #
#  MASM 6.x and Watcom C/C++ toolsets required to build       #
#  driver DLLs for all target environments                    #
#                                                             #
###############################################################

#
# DLL/file loader
#

dllload.obj: dllload.c dll.h
   wcc386p dllload

#
# Process Services API module
#

ail32.obj: ail32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DDPMI ail32.asm

#
# XMIDI driver: MT-32 family with Roland MPU-401-compatible interface
#

a32mt32.dll: xmidi32.asm mt3232.inc mpu40132.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DMT32 /DMPU401 /DDPMI xmidi32.asm
   wlink n a32mt32.dll f xmidi32 format os2 lx dll 

#
# XMIDI driver: MT-32 family with Sound Blaster MIDI-compatible interface
#

a32mt32s.dll: xmidi32.asm mt3232.inc sbmidi32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DMT32 /DSBMIDI /DDPMI xmidi32.asm
   wlink n a32mt32s.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Tandy 3-voice internal speaker
#

a32tandy.dll: xmidi32.asm spkr32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DTANDY /DDPMI xmidi32.asm
   wlink n a32tandy.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: IBM-PC internal speaker
#

a32spkr.dll: xmidi32.asm spkr32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DIBMPC /DDPMI xmidi32.asm
   wlink n a32spkr.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Standard Ad Lib or compatible
#

a32adlib.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DADLIBSTD /DDPMI xmidi32.asm
   wlink n a32adlib.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Ad Lib Gold
#

a32algfm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DADLIBG /DDPMI xmidi32.asm
   wlink n a32algfm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Standard Sound Blaster
#

a32sbfm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBSTD /DDPMI xmidi32.asm
   wlink n a32sbfm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Sound Blaster Pro I (dual-3812 version)
#

a32sp1fm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBPRO1 /DDPMI xmidi32.asm
   wlink n a32sp1fm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Sound Blaster Pro II (OPL3 version) XMIDI driver
#

a32sp2fm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBPRO2 /DDPMI xmidi32.asm
   wlink n a32sp2fm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Pro Audio Spectrum (dual-3812 version)
#

a32pasfm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DPAS /DDPMI xmidi32.asm
   wlink n a32pasfm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Pro Audio Spectrum Plus/16 (with OPL3)
#

a32pasop.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DPASOPL /DDPMI xmidi32.asm
   wlink n a32pasop.dll f xmidi32 format os2 lx dll

#
# Digital sound driver: Ad Lib Gold
#

a32algdg.dll: dmasnd32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DADLIBG /DDPMI dmasnd32.asm
   wlink n a32algdg.dll f dmasnd32 format os2 lx dll

#
# Digital sound driver: Standard Sound Blaster
#

a32sbdg.dll: dmasnd32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBSTD /DDPMI dmasnd32.asm
   wlink n a32sbdg.dll f dmasnd32 format os2 lx dll

#
# Digital sound driver: Sound Blaster Pro
#

a32sbpdg.dll: dmasnd32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBPRO /DDPMI dmasnd32.asm
   wlink n a32sbpdg.dll f dmasnd32 format os2 lx dll

#
# Digital sound driver: Pro Audio Spectrum
#

a32pasdg.dll: dmasnd32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DPAS /DDPMI dmasnd32.asm
   wlink n a32pasdg.dll f dmasnd32 format os2 lx dll

#
# STP32.EXE: 32-bit protected-mode version of STPLAY
#

stp32.exe: stp32.c ail32.h dll.h ail32.obj dllload.obj
   wcc386p /dDPMI stp32
   wlink n stp32 f stp32,ail32,dllload system dos4g

#
# VP32.EXE: 32-bit protected-mode version of VOCPLAY
#

vp32.exe: vp32.c ail32.h dll.h ail32.obj dllload.obj
   wcc386p /dDPMI vp32
   wlink n vp32 f vp32,ail32,dllload system dos4g

#
# MIX32.EXE: 32-bit protected-mode version of MIXDEMO
#

mix32.exe: mix32.c ail32.h dll.h ail32.obj dllload.obj
   wcc386p /dDPMI mix32
   wlink n mix32 f mix32,ail32,dllload system dos4g

#
# XP32.EXE: 32-bit protected-mode version of XPLAY
#

xp32.exe: xp32.c ail32.h dll.h ail32.obj dllload.obj
   wcc386p /dDPMI xp32
   wlink n xp32 f xp32,ail32,dllload system dos4g
