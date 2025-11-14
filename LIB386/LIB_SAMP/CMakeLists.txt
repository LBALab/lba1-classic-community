# LIB_SAMP - Wave/Sample Library (LIB_WAVE)
# High-level wave/sample API (platform-agnostic interface)
# All implementation moved to platform-specific folders (LIB_SAMP_DOS)

# Source files (from MAKEFILE OBJETS1)
# WAVE.C moved to LIB_SAMP_DOS (uses DosMalloc for DMA buffers)

# Create interface library (empty for now, serves as placeholder for future platform-agnostic API)
add_library(LIB_WAVE INTERFACE)

# Include current directory for headers
target_include_directories(LIB_WAVE INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
