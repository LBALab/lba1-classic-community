# LIB_SAMP_DOS - DOS-Specific Wave/Sample Implementation
# Implements sound card hardware access, DMA transfers, and DOS memory allocation
# Supports Sound Blaster, Gravis Ultrasound, and compatible cards

# C source files
set(SAMP_DOS_C_SOURCES
    # GUS.C requires Gravis Ultrasound SDK, not part of standard build
)

# Assembly source files
set(SAMP_DOS_ASM_SOURCES
    WAVE_I.ASM    # Wave interrupt and hardware control
    # Other ASM files (WAVE_A.ASM, N_WAVE.ASM, DMA_CODE.ASM, REAL.ASM, BALANCE.ASM)
    # are for DLL builds or specific hardware drivers, not part of static library
)

# Compile C sources
foreach(src ${SAMP_DOS_C_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION}
        COMMAND ${CMAKE_C_COMPILER}
        ARGS ${CMAKE_C_FLAGS} -i=${LIB386_PATH} -fo=${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION} ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compiling LIB_SAMP_DOS C file ${src}"
    )
    list(APPEND SAMP_DOS_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION})
endforeach()

# Compile Assembly sources
foreach(src ${SAMP_DOS_ASM_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj
        COMMAND ${CMAKE_ASM_MASM_COMPILER}
        ARGS ${CMAKE_ASM_MASM_FLAGS} -Fo${CMAKE_CURRENT_BINARY_DIR}/${src}.obj ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Assembling LIB_SAMP_DOS file ${src}"
    )
    list(APPEND SAMP_DOS_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj)
endforeach()

# Create static library as an imported library
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/samp_dos.lib
    COMMAND ${CMAKE_AR}
    ARGS -n ${CMAKE_CURRENT_BINARY_DIR}/samp_dos.lib ${SAMP_DOS_OBJECTS}
    DEPENDS ${SAMP_DOS_OBJECTS}
    COMMENT "Creating LIB_SAMP_DOS library"
)

# Create a custom target for building
add_custom_target(samp_dos_build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/samp_dos.lib)

# Create an imported library that can be linked
add_library(samp_dos STATIC IMPORTED GLOBAL)
set_target_properties(samp_dos PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/samp_dos.lib
)
add_dependencies(samp_dos samp_dos_build)
