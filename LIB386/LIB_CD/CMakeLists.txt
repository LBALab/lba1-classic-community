# LIB_CD - CD-ROM Library
# Assembly library for CD-ROM access

# Source files (from MAKEFILE OBJETS1)
set(LIB_CD_SOURCES
    CDROM.ASM
)

# Create static library
add_library(LIB_CD STATIC ${LIB_CD_SOURCES})

# Include current directory for headers
target_include_directories(LIB_CD PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_CD PROPERTIES
    OUTPUT_NAME "LIB_CD"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
