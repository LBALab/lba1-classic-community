# SOURCES - Main LBA1 Classic Executable
# Mixed C and Assembly sources for the main game

# Core C source files (from MAKEFILE OBJECTS)
set(LBA_CORE_C_SOURCES
    VERSION.C
    PERSO.C
    OBJECT.C
    GLOBAL.C
    FLIPBOX.C
    DISKFUNC.C
    FICHE.C
    EXTRA.C
    INCRUST.C
    GRILLE.C
    MESSAGE.C
    AMBIANCE.C
    GAMEMENU.C
    GERETRAK.C
    GERELIFE.C
    HOLOMAP.C
    PLAYFLA.C
    MSG_CUST.C
)

# Modules in transition from ASM to C implementations
set(LBA_ASM_TO_C_MODULES
    ADFLI_A
    BALANCE
    CPYMASK
    FIRE
    FUNC
    GRILLE_A
    MCGA
)

set(LBA_CONVERTED_C_SOURCES)
set(LBA_ORIGINAL_ASM_SOURCES)
foreach(module ${LBA_ASM_TO_C_MODULES})
    list(APPEND LBA_CONVERTED_C_SOURCES "${module}.C")
    list(APPEND LBA_ORIGINAL_ASM_SOURCES "${module}.ASM")
endforeach()

# Common libraries (base targets)
set(COMMON_LIBRARY_BASES
    LIB_3D
    LIB_CD
    LIB_MENU
    LIB_MIDI
    LIB_MIX
    LIB_WAVE
    LIB_SVGA
    LIB_SYS
)

set(COMMON_LIBRARIES_ASM)
set(COMMON_LIBRARIES_TRANSLATED)
foreach(lib ${COMMON_LIBRARY_BASES})
    list(APPEND COMMON_LIBRARIES_ASM ${lib})
    if(TARGET ${lib}_C)
        list(APPEND COMMON_LIBRARIES_TRANSLATED ${lib}_C)
    else()
        list(APPEND COMMON_LIBRARIES_TRANSLATED ${lib})
    endif()
endforeach()

# Common include directories
set(COMMON_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIB386_PATH}/LIB_3D
    ${LIB386_PATH}/LIB_CD
    ${LIB386_PATH}/LIB_MENU
    ${LIB386_PATH}/LIB_MIDI
    ${LIB386_PATH}/LIB_MIX
    ${LIB386_PATH}/LIB_SAMP
    ${LIB386_PATH}/LIB_SVGA
    ${LIB386_PATH}/LIB_SYS
)

function(lba_configure_dos_executable target output_name common_libs)
    target_link_libraries(${target} ${common_libs} LIB_CD_DOS LIB_MENU_DOS LIB_MIDI_DOS LIB_MIX_DOS LIB_SAMP_DOS LIB_SYS_DOS LIB_SVGA_DOS)

    target_include_directories(${target} PRIVATE ${COMMON_INCLUDES})

    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${output_name}"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LINKER_LANGUAGE C
    )

    if(CMAKE_C_COMPILER_ID MATCHES "Watcom")
        set_target_properties(${target} PROPERTIES
            LINK_FLAGS "OPTION stack=7000 SYSTEM dos4g"
        )
    endif()

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/EN_ctxt.csv
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/FR_ctxt.csv
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/DE_ctxt.csv
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/IT_ctxt.csv
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/SP_ctxt.csv
            ${CMAKE_BINARY_DIR}/bin/
        COMMENT "Copying CSV files for DOS build (${output_name})"
    )
endfunction()

#------------------------------------------------------------------------------
# DOS Executables
#------------------------------------------------------------------------------
if(BUILD_DOS)
    add_executable(LBAD ${LBA_CORE_C_SOURCES} ${LBA_ORIGINAL_ASM_SOURCES})

    foreach(src ${LBA_CORE_C_SOURCES})
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    endforeach()
    foreach(src ${LBA_ORIGINAL_ASM_SOURCES})
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endforeach()

    lba_configure_dos_executable(LBAD "LBAD" "${COMMON_LIBRARIES_ASM}")
    message(STATUS "DOS executable: LBAD.EXE (original assembly, DOS/4GW 32-bit protected mode)")
endif()

if(BUILD_DOS_C)
    add_executable(LBADC ${LBA_CORE_C_SOURCES} ${LBA_CONVERTED_C_SOURCES})

    foreach(src ${LBA_CORE_C_SOURCES} ${LBA_CONVERTED_C_SOURCES})
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    endforeach()

    target_compile_definitions(LBADC PRIVATE SKIP_INTRO)

    lba_configure_dos_executable(LBADC "LBADC" "${COMMON_LIBRARIES_TRANSLATED}")
    message(STATUS "DOS executable: LBADC.EXE (translated C modules, DOS/4GW 32-bit protected mode)")
endif()
