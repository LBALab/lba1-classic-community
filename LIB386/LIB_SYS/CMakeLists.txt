# LIB_SYS - System Library
# Provides both original assembly and translated C variants for selectable builds

set(LIB_SYS_MODULES
    "FILES|C"
    "LOADMALL|C"
    "LOADSAVE|C"
    "DEF_FILE|C"
    "BUFFER_A|ASM|C"
    "DIVERS|C"
    "EXPAND|ASM|C"
    "HQ_RESS|C"
    "HQR|ASM"
    "ADELINE|C"
    "HQ_MEM|C"
    "HQ_R_M|C"
)

function(lba_libsys_collect_sources variant out_var)
    set(result)
    foreach(entry IN LISTS LIB_SYS_MODULES)
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

function(lba_add_libsys target variant)
    lba_libsys_collect_sources("${variant}" sources)

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

lba_add_libsys(LIB_SYS ASM)
lba_add_libsys(LIB_SYS_C C)
