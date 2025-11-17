# Platforms
## Targeted Platforms
- DOS (32-bit protected mode with DOS/4GW)
- Windows 9X
- Windows 32/64
- Linux (Steam Deck / Steam Machine)
- macOS (Intel/Apple Silicon)
- Web (Emscripten)
- Mobile Android
- Mobile iOS

## DOS
The DOS build targets 32-bit protected mode using the DOS/4GW extender. It produces two executables:
- `LBAD.EXE`: Original assembly modules
- `LBADC.EXE`: Translated C modules
Both executables link against the `LIB386` libraries, with C variants used in the translated build where available.

### Target Audio Support
    - AdLib (FM Synthesis)
    - Sound Blaster (Digital Audio)
    - MIDI
    - CDROM support

## Windows 9X/NT
The Windows build targets Win32 PE executables, producing `LBAW9X.exe`. It uses the same `LIB386` libraries as the DOS build, adapted for Windows.

### Target Audio Support
    - DirectSound
    - MIDI
    - CDROM support

## Windows 32/64
The Windows 32/64-bit build targets Win32 PE executables, producing `LBAW.exe`. It uses the `LIB386` libraries adapted for Windows. 

### Target Audio Support
 - XAudio2
 - MP3/OGG
 - No CDROM support

## Linux (Steam Deck / Steam Machine)
The Linux build targets 32/64-bit ELF executables. It uses the `LIB386` libraries adapted for Linux.

### Target Audio Support
 - ALSA
 - PulseAudio
 - SDL2 Audio
 - MP3/OGG
 - No CDROM support

## macOS
The macOS build targets 64-bit Mach-O executables. It uses the `LIB386` libraries adapted for macOS.

### Target Audio Support
 - CoreAudio
 - SDL2 Audio
 - MP3/OGG
 - No CDROM support

## Web (Emscripten)
The Web build targets WebAssembly using Emscripten. It uses the `LIB386` libraries adapted for WebAssembly.

### Target Audio Support
 - Web Audio API
 - MP3/OGG
 - No CDROM support
