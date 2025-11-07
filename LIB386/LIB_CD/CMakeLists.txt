# LIB_CD - CD-ROM Library
# Platform-agnostic CD-ROM API (currently empty - implementation in LIB_CD_DOS)
# Future: Cross-platform CD audio interface

# Source files (from MAKEFILE OBJETS1)
set(LIB_CD_SOURCES
    # CDROM.ASM  # Moved to LIB_CD_DOS (uses MSCDEX, DPMI, int 2Fh)
)

# Create interface library (empty for now, serves as placeholder for future platform-agnostic API)
add_library(LIB_CD INTERFACE)

# Include current directory for headers
target_include_directories(LIB_CD INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

