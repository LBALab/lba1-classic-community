# LIB_CD_DOS - DOS-Specific CD-ROM Implementation
# Implements CD-ROM access using MSCDEX (int 2Fh) and DPMI
# This is 100% DOS-specific hardware/driver interface code

# Assembly source files
set(CD_DOS_ASM_SOURCES
    CDROM.ASM   # MSCDEX CD-ROM driver interface (DPMI, int 2Fh interrupts)
)

# Compile Assembly sources
foreach(src ${CD_DOS_ASM_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj
        COMMAND ${CMAKE_ASM_MASM_COMPILER}
        ARGS ${CMAKE_ASM_MASM_FLAGS} -Fo${CMAKE_CURRENT_BINARY_DIR}/${src}.obj ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Assembling LIB_CD_DOS file ${src}"
    )
    list(APPEND CD_DOS_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj)
endforeach()

# Create static library as an imported library
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cd_dos.lib
    COMMAND ${CMAKE_AR}
    ARGS -n ${CMAKE_CURRENT_BINARY_DIR}/cd_dos.lib ${CD_DOS_OBJECTS}
    DEPENDS ${CD_DOS_OBJECTS}
    COMMENT "Creating LIB_CD_DOS library"
)

# Create a custom target for building
add_custom_target(cd_dos_build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cd_dos.lib)

# Create an imported library that can be linked
add_library(cd_dos STATIC IMPORTED GLOBAL)
set_target_properties(cd_dos PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/cd_dos.lib
)
add_dependencies(cd_dos cd_dos_build)
