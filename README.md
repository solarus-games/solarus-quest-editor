# Solarus Quest Editor

Solarus Quest Editor is a GUI to create and modify quests for the
[Solarus engine](https://github.com/christopho/solarus).

This project is a full rewriting in C++/Qt of the official quest editor
(written in Java).

## Compilation instructions

To build Solarus Quest Editor, you need:
- A C++ compiler with support of C++11.
- CMake 2.8.11 or greater.
- Qt5
- Solarus

We always keep branch master of Solarus Quest Editor compatible with branch
master of Solarus.

### With Qt Creator

In Qt Creator, you can load the Solarus Quest Editor project by opening the
CMakeLists.txt file.
You will need to tell Qt Creator the path to the CMake binary.
You might need to set CMake variables, like the location of the Solarus library
(see the example in the command-line section below).

### With the command line

If you don't want to use Qt Creator, you can build the project from the
command line using CMake.

Configure:

    $ cd solarus-quest-editor
    $ mkdir build
    $ cd build
    $ cmake -DSOLARUS_INCLUDE_DIR=/path/to/solarus/include -DSOLARUS_LIBRARY=/path/to/solarus/libsolarus.so ..

Build:

    $ make

Run:

    $ ./solarus-quest-editor

