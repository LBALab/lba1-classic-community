# LIB_MIX - Audio Mixer Library
# Mixed C and Assembly library for audio mixing

# Source files (from MAKEFILE OBJETS1)
set(LIB_MIX_SOURCES
    MIXER.C
    MIXER_I.ASM
)

# Create static library
add_library(LIB_MIX STATIC ${LIB_MIX_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_MIX_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_MIX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_MIX PROPERTIES
    OUTPUT_NAME "LIB_MIX"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
