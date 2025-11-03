# LIB_SVGA_WIN - Windows-Specific GDI/DirectDraw Implementation
# Implements platform functions defined in lib_svga/lib_svga_platform.h
# GDI rendering, window management, linear framebuffer

set(SVGA_WIN_SOURCES
    video.c
    initsvga_win.c
)

# Compile C sources
foreach(src ${SVGA_WIN_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION}
        COMMAND ${CMAKE_C_COMPILER}
        ARGS ${CMAKE_C_FLAGS} -i=${LIB386_PATH} -i=${CMAKE_CURRENT_SOURCE_DIR} -fo=${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION} ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compiling LIB_SVGA_WIN C file ${src}"
    )
    list(APPEND SVGA_WIN_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION})
endforeach()

# Create static library as an imported library
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/svga_win.lib
    COMMAND ${CMAKE_AR}
    ARGS -n ${CMAKE_CURRENT_BINARY_DIR}/svga_win.lib ${SVGA_WIN_OBJECTS}
    DEPENDS ${SVGA_WIN_OBJECTS}
    COMMENT "Creating LIB_SVGA_WIN library"
)

# Create a custom target for building
add_custom_target(svga_win_build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/svga_win.lib)

# Create an imported library that can be linked
add_library(svga_win STATIC IMPORTED GLOBAL)
set_target_properties(svga_win PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/svga_win.lib
)
add_dependencies(svga_win svga_win_build)
