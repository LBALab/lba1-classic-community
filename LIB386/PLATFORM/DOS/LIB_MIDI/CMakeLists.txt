# LIB_MIDI_DOS - DOS-Specific MIDI Implementation
# Uses Miles Sound System AIL32 (Audio Interface Library) for XMIDI playback
# Requires AIL32 v1.0 SDK (not included, see README.md)

# C source files
set(MIDI_DOS_C_SOURCES
    MIDI.C       # AIL32 MIDI driver implementation
    # INCLUDE.C is just headers, not compiled separately
)

# Compile C sources
foreach(src ${MIDI_DOS_C_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION}
        COMMAND ${CMAKE_C_COMPILER}
        ARGS ${CMAKE_C_FLAGS} -DUSE_AIL32 -i=${LIB386_PATH} -fo=${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION} ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compiling LIB_MIDI_DOS C file ${src}"
    )
    list(APPEND MIDI_DOS_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION})
endforeach()

# Create static library as an imported library
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/midi_dos.lib
    COMMAND ${CMAKE_AR}
    ARGS -n ${CMAKE_CURRENT_BINARY_DIR}/midi_dos.lib ${MIDI_DOS_OBJECTS}
    DEPENDS ${MIDI_DOS_OBJECTS}
    COMMENT "Creating LIB_MIDI_DOS library"
)

# Create a custom target for building
add_custom_target(midi_dos_build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/midi_dos.lib)

# Create an imported library that can be linked
add_library(midi_dos STATIC IMPORTED GLOBAL)
set_target_properties(midi_dos PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/midi_dos.lib
)
add_dependencies(midi_dos midi_dos_build)
