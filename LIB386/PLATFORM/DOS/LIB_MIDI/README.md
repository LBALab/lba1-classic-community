# LIB_MIDI_DOS

DOS-specific MIDI implementation using Miles Sound System AIL32 (Audio Interface Library).

## Requirements

Building LIB_MIDI_DOS requires the **Miles Sound System AIL32 v1.0 SDK**, which is not included in this repository due to licensing restrictions.

AIL32 v1.0 is:
- Copyright (C) 1991, 1992 Miles Design, Inc.
- Proprietary software
- Not freely distributable

## Required Files

The following files from the AIL32 SDK must be present to build LIB_MIDI_DOS:

### Header Files (for compilation):
- `AIL32.H` - Main API header
- `AIL.H` - Additional API definitions

### DLL Files (for runtime):
- `A32ADLIB.DLL` - AdLib driver
- `A32ALGFM.DLL` - AdLib Gold FM driver  
- `A32GFX.DLL` - General MIDI driver
- `A32MT32.DLL` - Roland MT-32 driver
- `A32OP3FM.DLL` - OPL3 FM driver
- `A32PASFM.DLL` - Pro Audio Spectrum FM driver
- `A32PASOP.DLL` - Pro Audio Spectrum OPL driver
- `A32SP1FM.DLL` - Sound Blaster FM driver
- `A32SP2FM.DLL` - Sound Blaster FM driver (alternate)
- `A32TANDY.DLL` - Tandy driver
- `GUSMID32.DLL` - Gravis Ultrasound MIDI driver

## Current Status

**LIB_MIDI_DOS is currently disabled in the build** because the AIL32 SDK is not available.

Instead, the game uses **LIB_MIDI with stub implementations** - MIDI functions are present but do nothing. The game will run without music.

## Enabling LIB_MIDI_DOS

If you have access to the AIL32 SDK:

1. Copy the header files to `LIB386/LIB_MIDI_DOS/` or a system include path
2. Copy the DLL files to the game's `bin/` directory (already present in releases)
3. Uncomment the following line in the root `CMakeLists.txt`:
   ```cmake
   # add_subdirectory(LIB386/LIB_MIDI_DOS)
   ```
4. Update `SOURCES/CMakeLists.txt` to link `midi_dos` instead of using the stub:
   ```cmake
   target_link_libraries(LBAD ${COMMON_LIBRARIES} cd_dos menu_dos midi_dos mix_dos samp_dos sys_dos svga_dos)
   ```

## Implementation

- **MIDI.C** - Main AIL32 MIDI driver implementation
  - Initializes AIL32 library
  - Loads and plays XMIDI sequences
  - Handles timbre/patch loading from Global Timbre Library (GTL) files
  - Provides volume control and fading

## See Also

- [LIB_MIDI README](../LIB_MIDI/README.md) - Original documentation about AIL32 requirements
