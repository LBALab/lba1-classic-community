# LIB_SVGA - SVGA Graphics Library
# Mixed C and Assembly library for SVGA video modes

# Source files (from MAKEFILE OBJETS1-7)
# Platform-specific files moved to LIB_SVGA_DOS/LIB_SVGA_WIN:
#   - INITSVGA.ASM (SVGA initialization)
#   - VESA.ASM (VESA BIOS interface)
#   - S_MOUSE.C (DOS mouse driver INT 33h)
#   - INITMODE.C (DOS video mode initialization)
#   - S_DLL.C (DOS SVGA DLL loader)
#   - S_MODE.ASM, DLL.ASM, DLL_I.ASM, S_FIND.ASM (INT 10h BIOS video mode setting)
set(LIB_SVGA_SOURCES
    # OBJETS1
    GRAPH_A.ASM
    # OBJETS2
    S_TEXT.C
    S_STRING.ASM
    MASK_A.ASM
    FONT_A.ASM
    # OBJETS3
    #S_SCREEN.ASM
    S_SCREEN.C
    S_BLOCK.ASM
    S_BLOCK2.ASM
    # S_PAL.ASM  # Moved to LIB_SVGA_DOS (uses VGA port I/O)
    # OBJETS4
    S_PLOT.ASM
    S_BOX.ASM
    S_LINE.ASM
    S_POLY.ASM
    S_FILLV.ASM
    # OBJETS5
    S_MOUSEA.ASM
    RECT.C
    ZOOM.ASM
    MASKGPH.C
    GRAPHMSK.ASM
    # OBJETS6
    # S_PHYS.ASM  # Moved to LIB_SVGA_DOS (uses bank switching, 0xA0000 video memory)
    TEXTURE.ASM
    S_BLOCK3.ASM
    LBM.C
    PCX.C
    # OBJETS7
    GIF.C
)

# Create static library
add_library(LIB_SVGA STATIC ${LIB_SVGA_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LIB_SVGA_SOURCES})
    get_filename_component(ext ${src} EXT)
    if(ext STREQUAL ".C")
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    elseif(ext STREQUAL ".ASM")
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endif()
endforeach()

# Include current directory for headers
target_include_directories(LIB_SVGA PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Set library output name and linker language
set_target_properties(LIB_SVGA PROPERTIES
    OUTPUT_NAME "LIB_SVGA"
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LINKER_LANGUAGE C
)
