# LIB_MIX_DOS - DOS-Specific Audio Mixer Implementation
# Implements hardware mixer control for Sound Blaster, AdLib Gold, Pro Audio Spectrum
# Uses direct port I/O (out dx, al / in al, dx) and DOS interrupts

# Assembly source files
set(MIX_DOS_ASM_SOURCES
    MIXER_I.ASM   # Mixer interface code
    # MIXER_A.ASM is for DLL builds, not part of static library
)

# Compile Assembly sources
foreach(src ${MIX_DOS_ASM_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj
        COMMAND ${CMAKE_ASM_MASM_COMPILER}
        ARGS ${CMAKE_ASM_MASM_FLAGS} -Fo${CMAKE_CURRENT_BINARY_DIR}/${src}.obj ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Assembling LIB_MIX_DOS file ${src}"
    )
    list(APPEND MIX_DOS_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj)
endforeach()

# Create static library as an imported library
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mix_dos.lib
    COMMAND ${CMAKE_AR}
    ARGS -n ${CMAKE_CURRENT_BINARY_DIR}/mix_dos.lib ${MIX_DOS_OBJECTS}
    DEPENDS ${MIX_DOS_OBJECTS}
    COMMENT "Creating LIB_MIX_DOS library"
)

# Create a custom target for building
add_custom_target(mix_dos_build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/mix_dos.lib)

# Create an imported library that can be linked
add_library(mix_dos STATIC IMPORTED GLOBAL)
set_target_properties(mix_dos PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/mix_dos.lib
)
add_dependencies(mix_dos mix_dos_build)
