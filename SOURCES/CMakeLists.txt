# SOURCES - Main LBA1 Classic Executable
# Mixed C and Assembly sources for the main game

# Common source files (from MAKEFILE OBJECTS)
set(LBA_C_SOURCES
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
    # converted from asm
    ADFLI_A.C
    BALANCE.C
    CPYMASK.C
    FIRE.C
    FUNC.C
    GRILLE_A.C
    MCGA.C
)

set(LBA_ASM_SOURCES
    #GRILLE_A.ASM
    #FUNC.ASM
    #CPYMASK.ASM
    #BALANCE.ASM
    #FIRE.ASM
    #ADFLI_A.ASM
    #MCGA.ASM
)

# Common libraries
set(COMMON_LIBRARIES
    LIB_3D
    LIB_CD
    LIB_MENU
    LIB_MIDI
    LIB_MIX
    LIB_WAVE
    LIB_SVGA
    LIB_SYS
)

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

#------------------------------------------------------------------------------
# DOS Executable - LBAD.EXE
#------------------------------------------------------------------------------
if(BUILD_DOS)
    # Create DOS executable
    add_executable(LBAD ${LBA_C_SOURCES} ${LBA_ASM_SOURCES})
    
    # Set language for source files
    foreach(src ${LBA_C_SOURCES})
        set_source_files_properties(${src} PROPERTIES LANGUAGE C)
    endforeach()
    foreach(src ${LBA_ASM_SOURCES})
        set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
    endforeach()
    
    # Link libraries
    target_link_libraries(LBAD ${COMMON_LIBRARIES} cd_dos menu_dos mix_dos samp_dos sys_dos svga_dos)
    # midi_dos not linked - requires AIL32 SDK, using stub implementation instead
    
    # Include directories
    target_include_directories(LBAD PRIVATE ${COMMON_INCLUDES})
    
    # Set executable properties
    set_target_properties(LBAD PROPERTIES
        OUTPUT_NAME "LBAD"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        LINKER_LANGUAGE C
    )
    
    # DOS-specific linker flags
    if(CMAKE_C_COMPILER_ID MATCHES "Watcom")
        set_target_properties(LBAD PROPERTIES
            LINK_FLAGS "OPTION stack=7000 SYSTEM dos4g"
        )
    endif()
    
    # Dependencies
    add_dependencies(LBAD ${COMMON_LIBRARIES} sys_dos_build svga_dos_build)
    
    # Copy CSV files
    add_custom_command(TARGET LBAD POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/EN_ctxt.csv
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/FR_ctxt.csv
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/DE_ctxt.csv
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/IT_ctxt.csv
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/text/SP_ctxt.csv
        ${CMAKE_BINARY_DIR}/bin/
        COMMENT "Copying CSV files for DOS build"
    )
    
    message(STATUS "DOS executable: LBAD.EXE (DOS/4GW 32-bit protected mode)")
endif()
