# LIB_SAMP - Wave/Sample Library (LIB_WAVE)
# Mixed C and Assembly library for wave audio

# Source files (from MAKEFILE OBJETS1)
set(LIB_WAVE_SOURCES
    WAVE.C
    WAVE_I.ASM
)

# Create static library
add_library(LIB_WAVE STATIC ${LIB_WAVE_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_WAVE_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_WAVE PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_WAVE PROPERTIES
    OUTPUT_NAME "LIB_WAVE"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
