# SOURCES - Main LBA1 Classic Executable
# Mixed C and Assembly sources for the main game

# Source files (from MAKEFILE OBJECTS)
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
)

set(LBA_ASM_SOURCES
    GRILLE_A.ASM
    FUNC.ASM
    CPYMASK.ASM
    BALANCE.ASM
    FIRE.ASM
    ADFLI_A.ASM
    MCGA.ASM
)

# Create executable
add_executable(LBA0 ${LBA_C_SOURCES} ${LBA_ASM_SOURCES})

# Explicitly set language for source files with uppercase extensions
foreach(src ${LBA_C_SOURCES})
    set_source_files_properties(${src} PROPERTIES LANGUAGE C)
endforeach()

foreach(src ${LBA_ASM_SOURCES})
    set_source_files_properties(${src} PROPERTIES LANGUAGE ASM_MASM)
endforeach()

# Link with all required libraries
target_link_libraries(LBA0
    LIB_3D
    LIB_CD
    LIB_MENU
    LIB_MIDI
    LIB_MIX
    LIB_WAVE
    LIB_SVGA
    LIB_SYS
)

# Include directories
target_include_directories(LBA0 PRIVATE
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

# Set executable output name, directory, and linker language
set_target_properties(LBA0 PROPERTIES
    OUTPUT_NAME "LBA0"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LINKER_LANGUAGE C
)

# Set OpenWatcom-specific linker flags
if(CMAKE_C_COMPILER_ID MATCHES "Watcom")
    set_target_properties(LBA0 PROPERTIES
        LINK_FLAGS "OPTION stack=7000 SYSTEM dos4g"
    )
endif()

# Add dependency to ensure libraries are built first
add_dependencies(LBA0
    LIB_3D
    LIB_CD
    LIB_MENU
    LIB_MIDI
    LIB_MIX
    LIB_WAVE
    LIB_SVGA
    LIB_SYS
)
