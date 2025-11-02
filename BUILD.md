# Building LBA1 Classic Community

This project uses CMake with OpenWatcom 2 C compiler and JWasm MASM-compatible assembler to build the original DOS executable.

## Prerequisites

### Required Tools

1. **CMake 3.20+**
   ```bash
   # macOS
   brew install cmake
   
   # Linux
   sudo apt install cmake
   ```

2. **OpenWatcom 2**
   - Download from: https://github.com/open-watcom/open-watcom-v2/releases
   - Set `WATCOM` environment variable to installation directory
   ```bash
   export WATCOM=/path/to/watcom
   export PATH=$WATCOM/bino64:$WATCOM/binl64:$PATH
   ```

3. **JWasm 2.20+**
   - MASM-compatible assembler with procedure-local label support
   - Install from package manager or build from source:
   ```bash
   # macOS
   brew install jwasm
   
   # Or build from source
   git clone https://github.com/JWasm/JWasm.git
   cd JWasm
   make -f GccUnix.mak
   sudo cp jwasm /usr/local/bin/
   ```

## Build Instructions

### Quick Start

```bash
# Configure
cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/openwatcom.cmake -S . -B build

# Build
cmake --build build

# Output will be in build/bin/LBA0.exe
```

### Manual Build

```bash
# Create build directory
mkdir build && cd build

# Configure with OpenWatcom toolchain
cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/openwatcom.cmake ..

# Build all targets
make

# Or build specific library
make LIB_3D
make LIB_SVGA
make LBA0
```

### Clean Build

```bash
# Remove build directory and rebuild
rm -rf build
mkdir build && cd build
cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/openwatcom.cmake ..
make
```

## Build Targets

### Libraries
- `LIB_3D` - 3D rendering engine
- `LIB_CD` - CD-ROM support
- `LIB_MENU` - Menu system
- `LIB_MIDI` - MIDI music
- `LIB_MIX` - Audio mixer
- `LIB_WAVE` - Wave/sample audio
- `LIB_SVGA` - SVGA graphics
- `LIB_SYS` - System utilities

### Executable
- `LBA0` - Main game executable (builds all libraries first)

## Build Output

```
build/
├── bin/
│   └── LBA0.exe          # DOS executable (DOS/4GW format)
└── lib/
    ├── libLIB_3D.a       # 3D library
    ├── libLIB_CD.a       # CD library
    ├── libLIB_MENU.a     # Menu library
    ├── libLIB_MIDI.a     # MIDI library
    ├── libLIB_MIX.a      # Mixer library
    ├── libLIB_WAVE.a     # Wave library
    ├── libLIB_SVGA.a     # SVGA library
    └── libLIB_SYS.a      # System library
```

## Technical Details

### Compiler Flags

**C Compiler (wcc386):**
- `-onatx -oh -oi -ei` - Optimizations
- `-zp2` - 2-byte structure packing
- `-6s` - 686 stack-based calling convention
- `-fp6` - 686 floating point
- `-wx` - Warnings as errors

**Assembler (JWasm):**
- `-6` - 686 instructions
- `-q` - Quiet mode
- `-Cx` - Case-insensitive symbols (MASM compatibility)

**Linker (wlink):**
- `SYSTEM dos4g` - DOS/4GW 32-bit protected mode
- `OPTION stack=7000` - 28KB stack

### Source Patching

The build system automatically patches source files during CMake configuration:

1. **NoLanguage Removal**: Removes obsolete `NoLanguage` keyword from ASM files
2. **Path Conversion**: Converts DOS backslashes (`\`) to forward slashes (`/`)
3. **Backup Creation**: Original files backed up with `.bak` extension (auto-cleaned)

### Why JWasm?

JWasm provides full MASM compatibility including:
- Procedure-local labels (same label in multiple procedures)
- Case-insensitive symbol matching
- Full MASM directive support
- Better than OpenWatcom's WASM for legacy code

## Troubleshooting

### "WATCOM environment variable is not set"
```bash
export WATCOM=/path/to/watcom
```

### "jwasm: command not found"
Install JWasm or add it to PATH:
```bash
export PATH=/usr/local/bin:$PATH
```

### "Cannot open file" errors
Run clean build to regenerate patched sources:
```bash
rm -rf build && mkdir build && cd build && cmake .. && make
```

### Build hangs or crashes
Check OpenWatcom installation and ensure `wcc386`, `wlink`, `wlib` are in PATH.

## Platform Support

- ✅ **macOS** - Tested on macOS 14+ (Apple Silicon and Intel)
- ✅ **Linux** - Tested on Ubuntu 22.04+
- ❌ **Windows** - Use WSL or native DOS build tools

**Target:** DOS/4GW 32-bit protected mode (runs in DOSBox, DOS32, or native DOS)

## License

See [LICENSE](LICENSE) file for details.
