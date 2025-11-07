# LIB_MIDI - MIDI Music Library
# Platform-agnostic MIDI API (stub implementation - real implementation in LIB_MIDI_DOS)
# Future: Cross-platform MIDI playback interface

# Source files (from MAKEFILE OBJETS1)
set(LIB_MIDI_SOURCES
    MIDI_STUB.C  # Stub implementation when AIL32 is not available
    # MIDI.C  # Moved to LIB_MIDI_DOS (uses AIL32/Miles Sound System)
)

# Create static library
add_library(LIB_MIDI STATIC ${LIB_MIDI_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_MIDI_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_MIDI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_MIDI PROPERTIES
    OUTPUT_NAME "LIB_MIDI"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)


