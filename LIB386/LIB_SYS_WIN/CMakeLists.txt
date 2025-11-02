# LIB_SYS_WIN - Windows-Specific System Implementation
# Implements platform functions defined in lib_sys/lib_sys_platform.h
# Timer, Keyboard, Memory, Critical Error Handler

set(SYS_WIN_SOURCES
    timer.c
    keyboard.c
    malloc.c
    harderr.c
)

# Compile C sources
foreach(src ${SYS_WIN_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION}
        COMMAND ${CMAKE_C_COMPILER}
        ARGS ${CMAKE_C_FLAGS} -i=${LIB386_PATH} -i=${CMAKE_CURRENT_SOURCE_DIR} -fo=${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION} ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compiling LIB_SYS_WIN C file ${src}"
    )
    list(APPEND SYS_WIN_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION})
endforeach()

# Create static library as an imported library
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sys_win.lib
    COMMAND ${CMAKE_AR}
    ARGS -n ${CMAKE_CURRENT_BINARY_DIR}/sys_win.lib ${SYS_WIN_OBJECTS}
    DEPENDS ${SYS_WIN_OBJECTS}
    COMMENT "Creating LIB_SYS_WIN library"
)

# Create a custom target for building
add_custom_target(sys_win_build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sys_win.lib)

# Create an imported library that can be linked
add_library(sys_win STATIC IMPORTED GLOBAL)
set_target_properties(sys_win PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/sys_win.lib
)
add_dependencies(sys_win sys_win_build)
