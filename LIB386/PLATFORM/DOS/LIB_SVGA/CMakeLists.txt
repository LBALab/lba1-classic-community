# LIB_SVGA_DOS - DOS-Specific VESA/SVGA Implementation
# Implements platform functions defined in lib_svga/lib_svga_platform.h
# VESA BIOS interface, mode setting, bank switching

set(SVGA_DOS_ASM_SOURCES
    VESA.ASM
    INITSVGA.ASM
    S_PHYS.ASM   # Physical screen operations (bank switching, Flip, CopyBlockPhys)
    S_PAL.ASM    # VGA palette I/O operations
)

# Compile Assembly sources
foreach(src ${SVGA_DOS_ASM_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj
        COMMAND ${CMAKE_ASM_MASM_COMPILER}
        ARGS ${CMAKE_ASM_MASM_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}/../../../LIB_SVGA -Fo${CMAKE_CURRENT_BINARY_DIR}/${src}.obj ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Assembling LIB_SVGA_DOS file ${src}"
    )
    list(APPEND SVGA_DOS_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj)
endforeach()

# Create static library as an imported library
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/svga_dos.lib
    COMMAND ${CMAKE_AR}
    ARGS -n ${CMAKE_CURRENT_BINARY_DIR}/svga_dos.lib ${SVGA_DOS_OBJECTS}
    DEPENDS ${SVGA_DOS_OBJECTS}
    COMMENT "Creating LIB_SVGA_DOS library"
)

# Create a custom target for building
add_custom_target(svga_dos_build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/svga_dos.lib)

# Create an imported library that can be linked
add_library(svga_dos STATIC IMPORTED GLOBAL)
set_target_properties(svga_dos PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/svga_dos.lib
)
add_dependencies(svga_dos svga_dos_build)
