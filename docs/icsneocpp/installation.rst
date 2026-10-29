============
Installation
============

Dependencies
============

The minimum requirements to build libicsneo are:
- CMake version 3.12 or newer
- A C++17 compiler


Building library & examples
===========================

For a list of available configuration options, see the ``option()`` entries in
``CMakeLists.txt``.

To build libicsneo with default options:
#. ``cmake -B build``
#. ``cmake --build build``

Adding to existing projects
===========================

libicsneo supports being added as a sub-projects with CMake's
``add_subdirectory()``.

#. Clone libicsneo into the desired location within the project
#. Add ``add_subdirectory(path/to/libicsneo)`` to ``CMakeLists.txt``
#. Link the project to libicsneo with ``target_link_libraries(app icsneocpp)``
