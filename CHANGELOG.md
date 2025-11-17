# Changelog

All notable changes to this project will be documented in this file.


## Unreleased

### Added
- Skip Adeline Logo
- Enable/Disable Wall Collision damage.
- Community-localized text system: load dialogs from CSV files per language, plus documentation on how to customize texts.
- Extended save system:
  - New in-game save/load options in the menu and related UI improvements.
  - Persistence of NPCs, objects, clover boxes, flags, extra items, ListZone/ListExtra, and ListFlagCube state across saves to keep scenes consistent and avoid softlocks.
  - Support for loading and migrating classic save files and AUTO saves, including compatibility for older versions and both DOS and Windows filename schemes.
  - Last valid position and a short invulnerability window when Twinsen respawns after losing a life.
  - Increased number of saves shown in the load menu and better handling of AUTO saves.
- Toggle in `LBA.CFG` to switch between Windows and DOS file name saving.
- GitHub Actions CI to build the project and CMake/Makefile improvements, including cross-platform and cross-compile setup and retro-compatibility build flows.
- Initial platform-agnostic and DOS-agnostic library refactors, including an initial DOS platform layer.
- Debug tools preprocessor switch.

### Changed
- Converted many ASM routines to C for better maintainability and portability.
- Improved DOSBox/VESA emulation behavior and fixed DOS keyboard build issues.
- Removed remaining DOS-specific references from core libraries to prepare for other platforms.
- Removed the player selection screen now that autosave and the enhanced save system are available.
- Updated CMake and Makefiles to support Open Watcom, cross-compile scenarios, and retro-compatible builds.
- Improved encoding and file handling, including conversion from CP437 to UTF-8 and more standard file operations in C.

### Fixed
- FLA movie playback reliability and the ability to skip FLA movies with ENTER.
- Sprite sort order and several visual glitches.

### Removed
- Unused and obsolete DOS-specific files to simplify the codebase.


---

_Check the [AUTHORS.md](AUTHORS.md) file for contributions and authorship information._
