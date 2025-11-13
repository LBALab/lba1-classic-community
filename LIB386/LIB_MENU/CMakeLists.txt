# LIB_MENU - Menu System Library
# Platform-agnostic menu UI (rendering and interaction)

# Source files (from MAKEFILE OBJETS1)
set(LIB_MENU_SOURCES
    MENU.C
    # SELECTOR.C     # Moved to LIB_MENU_DOS (uses DOS file system API)
    MENUFUNC.C
)

# Create static library
add_library(LIB_MENU STATIC ${LIB_MENU_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_MENU_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_MENU PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_MENU PROPERTIES
    OUTPUT_NAME "LIB_MENU"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
