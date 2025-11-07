# LIB_MENU_DOS - DOS-Specific Menu/File Selector Implementation
# Implements file system access using DOS API (_dos_findfirst, _dos_getdrive, etc.)

# C source files
set(MENU_DOS_C_SOURCES
    SELECTOR.C    # File selector using DOS file system API
)

# Compile C sources
foreach(src ${MENU_DOS_C_SOURCES})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION}
        COMMAND ${CMAKE_C_COMPILER}
        ARGS ${CMAKE_C_FLAGS} -i=${LIB386_PATH} -fo=${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION} ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compiling LIB_MENU_DOS C file ${src}"
    )
    list(APPEND MENU_DOS_OBJECTS ${CMAKE_CURRENT_BINARY_DIR}/${src}${CMAKE_C_OUTPUT_EXTENSION})
endforeach()

# Create static library as an imported library
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/menu_dos.lib
    COMMAND ${CMAKE_AR}
    ARGS -n ${CMAKE_CURRENT_BINARY_DIR}/menu_dos.lib ${MENU_DOS_OBJECTS}
    DEPENDS ${MENU_DOS_OBJECTS}
    COMMENT "Creating LIB_MENU_DOS library"
)

# Create a custom target for building
add_custom_target(menu_dos_build ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/menu_dos.lib)

# Create an imported library that can be linked
add_library(menu_dos STATIC IMPORTED GLOBAL)
set_target_properties(menu_dos PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/menu_dos.lib
)
add_dependencies(menu_dos menu_dos_build)
