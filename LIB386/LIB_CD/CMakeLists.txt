# LIB_CD - CD-ROM Library
# Platform-agnostic CD-ROM API (currently empty - implementation in LIB_CD_DOS)
# Future: Cross-platform CD audio interface

# Source files (from MAKEFILE OBJETS1)
set(LIB_CD_SOURCES
    LIB_CD.C
)

# Create static library
add_library(LIB_CD STATIC ${LIB_CD_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_CD_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_CD PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_CD PROPERTIES
    OUTPUT_NAME "LIB_CD"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
