# LIB_SYS - System Library
# Mixed C and Assembly library for system functions

# Source files (from MAKEFILE OBJETS1-4)
# Platform-specific files moved to LIB_SYS_DOS/LIB_SYS_WIN:
#   - TIMER.C, TIMER_A.ASM (timer implementation)
#   - KEYB.C, KEYBOARD.ASM (keyboard implementation)
#   - MALLOC.C (platform-specific memory management - in LIB_SYS_DOS/LIB_SYS_WIN)
# Note: MALLOC.C provides Malloc/Free/SmartMalloc/Mshrink functions
#       DOS version is in LIB_SYS_DOS, Windows version is in LIB_SYS_WIN
set(LIB_SYS_SOURCES
    # OBJETS1 - File handling
    FILES.C
    LOADMALL.C
    LOADSAVE.C
    DEF_FILE.C
    # OBJETS2 - System and buffers
    SYSTEM.C
    BUFFER_A.ASM
    DIVERS.C
    # OBJETS3 - HQR resources and expansion
    EXPAND.ASM
    HQ_RESS.C
    HQR.ASM
    # OBJETS4 - DLL loading and memory management
    DLLLOAD.C
    ADELINE.C
    HQ_MEM.C
    HQ_R_M.C
    FILES_A.ASM
)

# Create static library
add_library(LIB_SYS STATIC ${LIB_SYS_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_SYS_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_SYS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_SYS PROPERTIES
    OUTPUT_NAME "LIB_SYS"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
