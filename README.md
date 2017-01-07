![Solarus Quest Editor logo](/images/solarus-quest-editor-github-readme-logo.png)

[![Build Status](https://travis-ci.org/solarus-games/solarus-quest-editor.svg?branch=dev)](https://travis-ci.org/solarus-games/solarus-quest-editor)

Solarus Quest Editor is a graphical user interface to create and modify quests for the
[Solarus engine](https://github.com/solarus-games/solarus).

This software is written in C++ with Qt.

## License

The source code of Solarus Quest Editor is licensed under the terms of the
GNU General Public License v3.

Images used in the editor are licensed under
Creatives Commons Attribution-ShareAlike 3.0 (CC BY-SA 3.0).
http://creativecommons.org/licenses/by-sa/3.0/

See the license.txt file for more details.

## Compilation instructions

To build Solarus Quest Editor, you need:
- A C++ compiler with support of C++11 (gcc 4.8 and clang 3.4 are okay).
- CMake 2.8.11 or greater.
- Qt 5.2 or greater.
  - Debian qt5 packages required:
    - qtbase5-dev
    - qttools5-dev
    - qttools5-dev-tools
- Solarus and its dependencies:
  - SDL2
  - SDL2main
  - SDL2_image
  - SDL2_ttf
  - openal
  - vorbisfile
  - modplug (0.8.8.4 or greater)
  - lua5.1 or luajit (LuaJIT is recommended)
  - physfs

We always keep branch dev of Solarus Quest Editor compatible with branch
dev of Solarus.

### With Qt Creator

In Qt Creator, you can load the Solarus Quest Editor project by opening the
CMakeLists.txt file.
You might need to tell Qt Creator the path to the CMake binary.
If Solarus is installed in a standard paths known by CMake, it should directly
work. Otherwise, you need to set CMake variables indicating the location of the
Solarus includes and libraries.
(see the example in the command-line section below).

### With the command line

If you don't want to use Qt Creator, you can build the project from the
command line using CMake.

#### Configure:

    $ cd solarus-quest-editor
    $ mkdir build
    $ cd build
    $ cmake ..

If CMake fails to find Solarus include directories or libraries,
for example because they are not properly installed in standard paths,
you can explictly indicate their location instead:

    $ cmake -DSOLARUS_INCLUDE_DIR=/path/to/solarus/include -DSOLARUS_GUI_INCLUDE_DIR=/path/to/solarus/include -DSOLARUS_LIBRARY=/path/to/solarus/libsolarus.so -DSOLARUS_GUI_LIBRARY=/path/to/solarus/libsolarus-gui.so ..

#### Build:

    $ make

#### Run:

    $ ./solarus-quest-editor

