# LIB_SYS_DOS - DOS-Specific System Implementation
# Implements platform functions defined in lib_sys/lib_sys_platform.h
# Timer, Keyboard, Memory (DosMalloc), Critical Error Handler
# File System, Time (Platform Abstraction Layer)

# Source files (from MAKEFILE OBJETS1)
set(LIB_SYS_DOS_SOURCES
    TIMER.C
    KEYB.C
    MALLOC.C
    KEYBOARD.ASM
    TIMER_A.C
    FILES_A.C
    FILESYSTEM.C
    TIME.C
    # DOS-specific system files moved from LIB_SYS
    SYSTEM.C      # DOS interrupt vector management
    DLLLOAD.C     # DOS4GW LX DLL loader
)

# Create static library
add_library(LIB_SYS_DOS STATIC ${LIB_SYS_DOS_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_SYS_DOS_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_SYS_DOS PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../LIB_SYS  # For parent lib_sys headers
)

# Set library output name and linker language
set_target_properties(LIB_SYS_DOS PROPERTIES
    OUTPUT_NAME "LIB_SYS_DOS"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
