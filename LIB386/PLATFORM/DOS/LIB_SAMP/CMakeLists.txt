# LIB_SAMP_DOS - DOS-Specific Wave/Sample Implementation
# Implements sound card hardware access, DMA transfers, and DOS memory allocation
# Supports Sound Blaster, Gravis Ultrasound, and compatible cards

# Source files (from MAKEFILE OBJETS1)
# NOTE: WAVE_A.ASM, N_WAVE.ASM, and WAVE_I.ASM require wave.inc which is not available
# They appear to be sound driver implementations that may not be needed
# NOTE: GUS.C requires Gravis Ultrasound SDK headers (forte.h, gf1proto.h, extern.h, ultraerr.h)
set(LIB_SAMP_DOS_SOURCES
    BALANCE.ASM
    DMA_CODE.ASM
    N_WAVE.ASM
    REAL.ASM
    WAVE_A.ASM
    WAVE_I.ASM
    # GUS.C          # Requires Gravis Ultrasound SDK
    # DOS-specific files moved from LIB_SAMP
    WAVE.C         # DOS wave driver with DosMalloc for DMA buffers
)

# Create static library
add_library(LIB_SAMP_DOS STATIC ${LIB_SAMP_DOS_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_SAMP_DOS_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_SAMP_DOS PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../LIB_SAMP  # For parent lib_samp headers
)

# Set library output name and linker language
set_target_properties(LIB_SAMP_DOS PROPERTIES
    OUTPUT_NAME "LIB_SAMP_DOS"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
