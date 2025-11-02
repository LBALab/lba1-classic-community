# OpenWatcom 2 + JWasm Toolchain for CMake
# Configures CMake to use OpenWatcom 2 C compiler and JWasm MASM-compatible assembler

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i386)

# Compiler toolchain
set(CMAKE_C_COMPILER wcc386)           # OpenWatcom C compiler
set(CMAKE_CXX_COMPILER wpp386)         # OpenWatcom C++ compiler
set(CMAKE_ASM_MASM_COMPILER jwasm)     # JWasm MASM-compatible assembler
set(CMAKE_LINKER wlink)                # OpenWatcom linker
set(CMAKE_AR wlib)                     # OpenWatcom librarian

# Where to find the target environment
if(DEFINED ENV{WATCOM})
    set(CMAKE_FIND_ROOT_PATH $ENV{WATCOM})
else()
    message(FATAL_ERROR "WATCOM environment variable is not set. Please set it to your OpenWatcom installation directory.")
endif()

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Set the compiler identification
set(CMAKE_C_COMPILER_ID "Watcom")
set(CMAKE_C_COMPILER_ID_RUN TRUE)
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_C_ABI_COMPILED TRUE)

# Assembly support
set(CMAKE_ASM_MASM_COMPILER_ID "MASM")
set(CMAKE_ASM_MASM_COMPILER_ID_RUN TRUE)
set(CMAKE_ASM_MASM_COMPILER_FORCED TRUE)
set(CMAKE_ASM_MASM_COMPILER_WORKS TRUE)

# Object file extension
set(CMAKE_C_OUTPUT_EXTENSION .obj)
set(CMAKE_ASM_MASM_OUTPUT_EXTENSION .obj)

# Source file extensions - recognize uppercase extensions
set(CMAKE_C_SOURCE_FILE_EXTENSIONS C;c)
set(CMAKE_ASM_MASM_SOURCE_FILE_EXTENSIONS ASM;asm)

# Library prefix/suffix
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
set(CMAKE_EXECUTABLE_SUFFIX ".exe")

# C compiler flags (use - not / on Unix systems)
# -onatx = optimizations (natural order, time, relax alias, expensive)
# -oh = enable repeated optimizations
# -oi = expand intrinsic functions
# -ei = force enums to be int-sized
# -zp2 = pack structures on 2-byte boundary
# -6s = generate 686 stack-based calling convention
# -fp6 = 686 floating point instructions
# -s = disable stack overflow checks
# -wx = warnings as errors
# -wcd=N = disable specific warnings
set(CMAKE_C_FLAGS_INIT "-onatx -oh -oi -ei -zp2 -6s -fp6 -s -wx -wcd=131 -wcd=202 -wcd=303 -wcd=308")
set(CMAKE_C_FLAGS_DEBUG_INIT "-d2")
set(CMAKE_C_FLAGS_RELEASE_INIT "-onatx -oh -oi -ei")

# JWasm assembler flags
# -6 = 686 instructions
# -q = quiet operation
# -Cx = case insensitive symbols (MASM default)
set(CMAKE_ASM_MASM_FLAGS_INIT "-6 -q -Cx")

# Compilation commands
set(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -fo=<OBJECT> <SOURCE>")
set(CMAKE_ASM_MASM_COMPILE_OBJECT "<CMAKE_ASM_MASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -fo=<OBJECT> <SOURCE>")

# Library creation and linking
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> -q -b -n -c <TARGET> <OBJECTS>")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> NAME <TARGET> <LINK_FLAGS> FILE { <OBJECTS> } LIBRARY { <LINK_LIBRARIES> }")
