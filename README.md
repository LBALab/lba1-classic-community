# Little Big Adventure 1 - Engine source code - Community
Little Big Adventure (aka Relentless: Twinsen's Adventure) is an action-adventure game, developed by Adeline Software International in 1994. 

We are releasing this code with preservation in mind, as this piece of work was exceptional for the time and we believe it can be a valuable source of education.

The engine uses Assembly code and was originally compiled with non-open source libraries which have been excluded from the project. 

### Build using CMake and OpenWatcom 2

This project supports building for multiple platforms:
- **DOS** (32-bit protected mode with DOS/4GW) → `LBAD.EXE` (original assembly) and `LBADC.EXE` (translated C modules)
- **Windows 9x/NT** (Win32 PE executable) → `LBAW.exe`

#### Quick Start

**Build both platforms at once:**
```bash
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/openwatcom.cmake -DBUILD_DOS=ON -DBUILD_DOS_C=ON -DENABLE_CDROM=ON -B build
cmake --build . --config Release
```

**Test DOS version:**
```bash
dosbox-x build/bin/LBAD.EXE
```

The translated C build lives at `build/bin/LBADC.EXE`, linking against C variants of the core `LIB386` libraries where replacements exist. Disable either flavour with `-DBUILD_DOS=OFF` or `-DBUILD_DOS_C=OFF` when configuring.

**Test Windows version:**
```bash
wine build/bin/LBAW.exe
```

For detailed platform information, see [BUILD_PLATFORMS.md](BUILD_PLATFORMS.md)

### Licence
This source code is licensed under the [GNU General Public License](https://github.com/2point21/lba1-classic-community/blob/main/LICENSE).

Please note this license only applies to **Little Big Adventure 1** engine source code. **Little Big Adventure 1** game assets (art, models, textures, audio, etc.) are not open-source and therefore aren't redistributable.

## How can I contribute ?

Read our [Contribution Guidelines](https://github.com/2point21/lba1-classic-community/blob/main/CONTRIBUTING.md).

## Links:
**Official Website:** https://twinsenslittlebigadventure.com/

**Discord:** https://discord.gg/gfzna5SfZ5

**Docs:** https://lba-classic-doc.readthedocs.io/

## Buy the game:
 [[GoG]](https://www.gog.com/game/little_big_adventure)  [[Steam]](https://store.steampowered.com/app/397330/Little_Big_Adventure__Enhanced_Edition/?l=french)

## Original Dev Team
Direction: Frédérick Raynal

Programming: Frédérick Raynal / Laurent Salmeron / Serge Plagnol / Frantz Cournil / Olivier Lhermite

3D Objects & Animation: Didier Chanfray

Scenery Graphics: Yaël Barroz / Jean-Marc Torroella

Story & Design: Frédérick Raynal / Yaël Barroz / Jean-Jacques Poncet / Didier Chanfray / Laurent Salmeron

Dialogs: Jean-Jacques Poncet

Story Coding: Sébastien Viannay / Frantz Cournil / Jean-Jacques Poncet

Video Sequences: Frédéric Taquet / Didier Chanfray

Music & Sound FX: Philippe Vachey

Testing: Nicolas Viannay / Alexis Madinier / Lionel Chaze / Vijay Maharaj

Quality Control: Emmanuel Oualid

## Copyright
The intellectual property is currently owned by [2.21]. Copyright [2.21]

Originaly developed by Adeline Software International in 1994
