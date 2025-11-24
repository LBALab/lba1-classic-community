###############################################################
#                                                             #
#  MAKEFILE for AIL/32 development                            #             
#  10-Aug-92 John Miles                                       #
#                                                             #
#  This file builds drivers and sample applications for use   #
#  with MetaWare High C and Phar Lap 386|DOS Extender         #
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
   hc386 -c dllload.c

#
# Process Services API module
#

ail32.obj: ail32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DINT21 /DPHARLAP ail32.asm

#
# XMIDI driver: MT-32 family with Roland MPU-401-compatible interface
#

a32mt32.dll: xmidi32.asm mt3232.inc mpu40132.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DMT32 /DMPU401 /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32mt32.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: MT-32 family with Sound Blaster MIDI-compatible interface
#

a32mt32s.dll: xmidi32.asm mt3232.inc sbmidi32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DMT32 /DSBMIDI /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32mt32s.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Tandy 3-voice internal speaker
#

a32tandy.dll: xmidi32.asm spkr32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DTANDY /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32tandy.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: IBM-PC internal speaker
#

a32spkr.dll: xmidi32.asm spkr32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DIBMPC /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32spkr.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Standard Ad Lib or compatible
#

a32adlib.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DADLIBSTD /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32adlib.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Ad Lib Gold
#

a32algfm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DADLIBG /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32algfm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Standard Sound Blaster
#

a32sbfm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBSTD /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32sbfm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Sound Blaster Pro I (dual-3812 version)
#

a32sp1fm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBPRO1 /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32sp1fm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Sound Blaster Pro II (OPL3 version) XMIDI driver
#

a32sp2fm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBPRO2 /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32sp2fm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Pro Audio Spectrum (dual-3812 version)
#

a32pasfm.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DPAS /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32pasfm.dll f xmidi32 format os2 lx dll

#
# XMIDI driver: Pro Audio Spectrum Plus/16 (with OPL3)
#

a32pasop.dll: xmidi32.asm yamaha32.inc ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DPASOPL /DINT21 /DPHARLAP xmidi32.asm
   wlink n a32pasop.dll f xmidi32 format os2 lx dll

#
# Digital sound driver: Ad Lib Gold
#

a32algdg.dll: dmasnd32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DADLIBG /DINT21 /DPHARLAP dmasnd32.asm
   wlink n a32algdg.dll f dmasnd32 format os2 lx dll

#
# Digital sound driver: Standard Sound Blaster
#

a32sbdg.dll: dmasnd32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBSTD /DINT21 /DPHARLAP dmasnd32.asm
   wlink n a32sbdg.dll f dmasnd32 format os2 lx dll

#
# Digital sound driver: Sound Blaster Pro
#

a32sbpdg.dll: dmasnd32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DSBPRO /DINT21 /DPHARLAP dmasnd32.asm
   wlink n a32sbpdg.dll f dmasnd32 format os2 lx dll

#
# Digital sound driver: Pro Audio Spectrum
#

a32pasdg.dll: dmasnd32.asm ail32.inc 386.mac
   ml /c /W0 /Cp /Zd /DPAS /DINT21 /DPHARLAP dmasnd32.asm
   wlink n a32pasdg.dll f dmasnd32 format os2 lx dll

#
# STP32.EXE: 32-bit protected-mode version of STPLAY
#

stp32.exe: stp32.c ail32.h dll.h ail32.obj dllload.obj
   hc386 -DINT21 -Hldopt=-MAXREAL,0FFFFh,-MAXDATA,0 stp32.c ail32.obj dllload.obj

#
# VP32.EXE: 32-bit protected-mode version of VOCPLAY
#

vp32.exe: vp32.c ail32.h dll.h ail32.obj dllload.obj
   hc386 -DINT21 -Hldopt=-MAXREAL,0FFFFh,-MAXDATA,0 vp32.c ail32.obj dllload.obj

#
# MIX32.EXE: 32-bit protected-mode version of MIXDEMO
#

mix32.exe: mix32.c ail32.h dll.h ail32.obj dllload.obj
   hc386 -DINT21 mix32.c ail32.obj dllload.obj

#
# XP32.EXE: 32-bit protected-mode version of XPLAY
#

xp32.exe: xp32.c ail32.h dll.h ail32.obj dllload.obj
   hc386 -DINT21 xp32.c ail32.obj dllload.obj
