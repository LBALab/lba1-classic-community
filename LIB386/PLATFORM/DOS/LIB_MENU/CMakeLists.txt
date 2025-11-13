# LIB_MENU_DOS - Menu System Library
# Platform-agnostic menu UI (rendering and interaction)

# Source files (from MAKEFILE OBJETS1)
set(LIB_MENU_DOS_SOURCES
    SELECTOR.C
)

# Create static library
add_library(LIB_MENU_DOS STATIC ${LIB_MENU_DOS_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_MENU_DOS_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_MENU_DOS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_MENU_DOS PROPERTIES
    OUTPUT_NAME "LIB_MENU_DOS"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
