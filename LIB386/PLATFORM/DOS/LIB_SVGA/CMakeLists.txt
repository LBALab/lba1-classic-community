# LIB_SVGA_DOS - DOS-Specific VESA/SVGA Implementation
# Implements platform functions defined in lib_svga/lib_svga_platform.h
# VESA BIOS interface, mode setting, bank switching

# Source files (from MAKEFILE OBJETS1)
set(LIB_SVGA_DOS_SOURCES
    VESA.ASM
    INITSVGA.ASM
    S_PHYS.ASM
    S_PAL.ASM
)

# Create static library
add_library(LIB_SVGA_DOS STATIC ${LIB_SVGA_DOS_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_SVGA_DOS_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_SVGA_DOS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_SVGA_DOS PROPERTIES
    OUTPUT_NAME "LIB_SVGA_DOS"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
