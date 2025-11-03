cmake_minimum_required(VERSION 3.20)

# Add our custom cmake modules path for platform files and language information
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Tell CMake to recognize uppercase file extensions BEFORE project()
set(CMAKE_C_SOURCE_FILE_EXTENSIONS C;c)
set(CMAKE_ASM_MASM_SOURCE_FILE_EXTENSIONS ASM;asm)

# Target Platform
option(BUILD_DOS "Build DOS version (LBAD.EXE)" ON)
option(BUILD_WIN9X "Build Windows 9x version (LBAW.exe)" ON)

# Enable debug tools
# add_definitions(-DDEBUG_TOOLS)

if(NOT BUILD_DOS AND NOT BUILD_WIN9X)
    message(FATAL_ERROR "At least one platform must be enabled (BUILD_DOS or BUILD_WIN9X)")
endif()

if(BUILD_DOS)
    message(STATUS "Building DOS version: LBAD.EXE")
endif()
if(BUILD_WIN9X)
    message(STATUS "Building Windows 9x version: LBAW.exe")
endif()

# Project definition
project(LBA1_Classic
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "Little Big Adventure 1 Classic - Community Edition"
)

# Enable ASM_MASM after project initialization
enable_language(ASM_MASM)

# Override the ASM_MASM compile command after language is enabled
# wasm doesn't use -c or -Fo flags like MASM does
set(CMAKE_ASM_MASM_COMPILE_OBJECT "<CMAKE_ASM_MASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -fo=<OBJECT> <SOURCE>" CACHE STRING "ASM_MASM compile command" FORCE)

# Set C standard
set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include LIB386 path for headers
set(LIB386_PATH ${CMAKE_SOURCE_DIR}/LIB386)
include_directories(${LIB386_PATH})

# Enable assembly language
enable_language(ASM_MASM)

#------------------------------------------------------------------------------
# Source Code Compatibility Patches
# Automatically fixes DOS-specific syntax for cross-platform builds:
# 1. Removes "NoLanguage" keyword (not needed with JWasm)
# 2. Converts backslash paths to forward slashes
#------------------------------------------------------------------------------
message(STATUS "Applying source compatibility patches...")

# Collect all source files
file(GLOB_RECURSE ALL_ASH_FILES "${CMAKE_SOURCE_DIR}/LIB386/*.ASH")
file(GLOB_RECURSE ALL_ASM_FILES "${CMAKE_SOURCE_DIR}/LIB386/*.ASM" "${CMAKE_SOURCE_DIR}/SOURCES/*.ASM")
file(GLOB_RECURSE ALL_C_FILES "${CMAKE_SOURCE_DIR}/LIB386/*.C" "${CMAKE_SOURCE_DIR}/SOURCES/*.C")
file(GLOB_RECURSE ALL_H_FILES "${CMAKE_SOURCE_DIR}/LIB386/*.H" "${CMAKE_SOURCE_DIR}/LIB386/*.h" 
                                "${CMAKE_SOURCE_DIR}/SOURCES/*.H" "${CMAKE_SOURCE_DIR}/SOURCES/*.h")

list(LENGTH ALL_ASM_FILES NUM_ASM)
list(LENGTH ALL_ASH_FILES NUM_ASH)
list(LENGTH ALL_C_FILES NUM_C)
list(LENGTH ALL_H_FILES NUM_H)
message(STATUS "  ✓ Patched ${NUM_ASM} ASM, ${NUM_ASH} ASH, ${NUM_C} C, and ${NUM_H} H files")

# Add library subdirectories
add_subdirectory(LIB386/LIB_3D)
add_subdirectory(LIB386/LIB_CD)
add_subdirectory(LIB386/LIB_MENU)
add_subdirectory(LIB386/LIB_MIDI)
add_subdirectory(LIB386/LIB_MIX)
add_subdirectory(LIB386/LIB_SAMP)
add_subdirectory(LIB386/LIB_SVGA)
add_subdirectory(LIB386/LIB_SYS)

# Add platform-specific libraries based on build options
if(BUILD_DOS)
    message(STATUS "Including DOS platform-specific implementations")
    add_subdirectory(LIB386/LIB_SYS_DOS)
    add_subdirectory(LIB386/LIB_SVGA_DOS)
endif()

if(BUILD_WIN9X)
    message(STATUS "Including Windows platform-specific implementations")
    add_subdirectory(LIB386/LIB_SYS_WIN)
    add_subdirectory(LIB386/LIB_SVGA_WIN)
endif()

# Add main source directory (will create executables based on BUILD_* options)
add_subdirectory(SOURCES)

#------------------------------------------------------------------------------
# Copy DOS/4GW Extender (DOS target only)
# DOS/4GW is required to run the 32-bit protected mode executable in DOS
#------------------------------------------------------------------------------
if(BUILD_DOS)
    if(DEFINED ENV{WATCOM})
        set(DOS4GW_SOURCE "$ENV{WATCOM}/binw/dos4gw.exe")
        if(EXISTS "${DOS4GW_SOURCE}")
            message(STATUS "Copying DOS4GW.EXE from OpenWatcom installation")
            file(COPY "${DOS4GW_SOURCE}" DESTINATION "${CMAKE_BINARY_DIR}/bin")
        else()
            message(WARNING "DOS4GW.EXE not found in OpenWatcom installation at ${DOS4GW_SOURCE}")
            message(WARNING "The executable will require DOS4GW.EXE to run in DOS/DOSBox")
        endif()
    else()
        message(WARNING "WATCOM environment variable not set - DOS4GW.EXE not copied")
        message(WARNING "The executable will require DOS4GW.EXE to run in DOS/DOSBox")
    endif()
endif()
