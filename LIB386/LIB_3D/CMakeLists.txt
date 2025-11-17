# LIB_3D - 3D Graphics Library
# Provides both original assembly and translated C variants for selectable builds

set(LIB_3D_MODULES
    "P_SINTAB|ASM|C"
    "P_ANIM|ASM|C"
    "P_OB_ISO|ASM"
    "P_TRIGO|ASM"
    "P_FUNC|ASM|C"
)

function(lba_lib3d_collect_sources variant out_var)
    set(result)
    foreach(entry IN LISTS LIB_3D_MODULES)
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

function(lba_add_lib3d target variant)
    lba_lib3d_collect_sources("${variant}" sources)

    add_library(${target} STATIC ${sources})

    foreach(src IN LISTS sources)
        get_filename_component(ext ${src} EXT)
        if(ext STREQUAL ".C")
            set_source_files_properties(${src} PROPERTIES LANGUAGE C)
        elseif(ext STREQUAL ".ASM")
            set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
        endif()
    endforeach()

    target_include_directories(${target} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${LIB386_PATH}/LIB_SYS
    )

    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${target}"
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LINKER_LANGUAGE C
    )
endfunction()

lba_add_lib3d(LIB_3D ASM)
lba_add_lib3d(LIB_3D_C C)
