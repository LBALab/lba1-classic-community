# LIB_SYS_DOS - DOS-Specific System Implementation
# Implements platform functions defined in lib_sys/lib_sys_platform.h
# Timer, Keyboard, Memory (DosMalloc), Critical Error Handler
# File System, Time (Platform Abstraction Layer)

# C source files
set(SYS_DOS_C_SOURCES
    TIMER.C
    KEYB.C
    MALLOC.C
    # converted from assembly
    KEYBOARD.C
    TIMER_A.C
    FILES_A.C
    # Platform abstraction layer
    FILESYSTEM.C
    TIME.C
)

# Assembly source files
set(SYS_DOS_ASM_SOURCES
    #TIMER_A.ASM
    #KEYBOARD.ASM
    #FILES_A.ASM
)

# Compile C sources
foreach(src ${SYS_DOS_C_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION}
        COMMAND ${CMAKE_C_COMPILER}
        ARGS ${CMAKE_C_FLAGS} -i=${LIB386_PATH} -i=${CMAKE_SOURCE_DIR}/LIB386 -fo=${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION} ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compiling LIB_SYS_DOS C file ${src}"
    )
    list(APPEND SYS_DOS_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION})
endforeach()

# Compile Assembly sources
foreach(src ${SYS_DOS_ASM_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj
        COMMAND ${CMAKE_ASM_MASM_COMPILER}
        ARGS ${CMAKE_ASM_MASM_FLAGS} -Fo${CMAKE_CURRENT_BINARY_DIR}/${src}.obj ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Assembling LIB_SYS_DOS file ${src}"
    )
    list(APPEND SYS_DOS_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}.obj)
endforeach()

# Create static library as an imported library
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sys_dos.lib
    COMMAND ${CMAKE_AR}
    ARGS -n ${CMAKE_CURRENT_BINARY_DIR}/sys_dos.lib ${SYS_DOS_OBJECTS}
    DEPENDS ${SYS_DOS_OBJECTS}
    COMMENT "Creating LIB_SYS_DOS library"
)

# Create a custom target for building
add_custom_target(sys_dos_build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sys_dos.lib)

# Create an imported library that can be linked
add_library(sys_dos STATIC IMPORTED GLOBAL)
set_target_properties(sys_dos PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/sys_dos.lib
)
add_dependencies(sys_dos sys_dos_build)
