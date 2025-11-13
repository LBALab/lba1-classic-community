# LIB_CD_DOS - CD System Library
# Platform-agnostic CD UI (rendering and interaction)

# Source files (from MAKEFILE OBJETS1)
set(LIB_CD_DOS_SOURCES
    CDROM.ASM
)

# Create static library
add_library(LIB_CD_DOS STATIC ${LIB_CD_DOS_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_CD_DOS_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_CD_DOS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_CD_DOS PROPERTIES
    OUTPUT_NAME "LIB_CD_DOS"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
