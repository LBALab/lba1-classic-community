# LIB_3D - 3D Graphics Library
# Assembly-only library for 3D rendering

# Source files (from MAKEFILE OBJETS1-4)
set(LIB_3D_SOURCES
    P_SINTAB.ASM
    P_ANIM.ASM
    P_OB_ISO.ASM
    P_TRIGO.ASM
    P_FUNC.ASM
)

# Create static library
add_library(LIB_3D STATIC ${LIB_3D_SOURCES})

# Include current directory for headers
target_include_directories(LIB_3D PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIB386_PATH}/LIB_SYS
)

# Set library output name and linker language
set_target_properties(LIB_3D PROPERTIES
    OUTPUT_NAME "LIB_3D"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
