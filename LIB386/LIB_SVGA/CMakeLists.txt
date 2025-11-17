# LIB_SVGA - SVGA Graphics Library
# Provides both original assembly and translated C variants for selectable builds

set(LIB_SVGA_MODULES
    "GRAPH_A|ASM"
    "S_TEXT|C"
    "S_STRING|ASM"
    "MASK_A|ASM"
    "FONT_A|ASM"
    "S_SCREEN|ASM|C"
    "S_BLOCK|ASM"
    "S_BLOCK2|ASM"
    "S_PLOT|ASM"
    "S_BOX|ASM"
    "S_LINE|ASM"
    "S_POLY|ASM"
    "S_FILLV|ASM"
    "S_MOUSEA|ASM"
    "RECT|C"
    "ZOOM|ASM"
    "MASKGPH|C"
    "GRAPHMSK|ASM"
    "TEXTURE|ASM"
    "S_BLOCK3|ASM"
    "LBM|C"
    "PCX|C"
    "GIF|C"
)

function(lba_libsvga_collect_sources variant out_var)
    set(result)
    foreach(entry IN LISTS LIB_SVGA_MODULES)
        string(REPLACE "|" ";" parts "${entry}")
        list(GET parts 0 module)
        list(GET parts 1 asm_ext)
        list(LENGTH parts parts_len)
        if(parts_len GREATER 2)
            list(GET parts 2 c_ext)
        else()
            set(c_ext "")
        endif()

        if(variant STREQUAL "C" AND c_ext)
            set(ext "${c_ext}")
        else()
            set(ext "${asm_ext}")
        endif()

        list(APPEND result "${module}.${ext}")
    endforeach()

    set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

function(lba_add_libsvga target variant)
    lba_libsvga_collect_sources("${variant}" sources)

    add_library(${target} STATIC ${sources})

    foreach(src IN LISTS sources)
        get_filename_component(ext ${src} EXT)
        if(ext STREQUAL ".C")
            set_source_files_properties(${src} PROPERTIES LANGUAGE C)
        elseif(ext STREQUAL ".ASM")
            set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
        endif()
    endforeach()

    target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${target}"
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LINKER_LANGUAGE C
    )
endfunction()

lba_add_libsvga(LIB_SVGA ASM)
lba_add_libsvga(LIB_SVGA_C C)
