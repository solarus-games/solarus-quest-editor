# CMake 2.8.11 or above is necessary for recent Qt5 cmake helper functions.
cmake_minimum_required(VERSION 2.8.11)  

project(solarus-quest-editor)

# As Qt generates ui header files, the build directory needs to be an include
# directory too.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Tell CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Compile as C++11.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

# Find dependencies.
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/modules/")
find_package(Qt5Widgets REQUIRED)
find_package(Solarus REQUIRED)

include_directories(
  "${SOLARUS_INCLUDE_DIR}"
)

# Source files.
set(solarus_quest_editor_SOURCES
  main.cpp
  mainwindow.cpp
  mainwindow.h
)

# UI files.
set(solarus_quest_editor_FORMS
  mainwindow.ui
)

# Generate .h from .ui.
qt5_wrap_ui(solarus_quest_editor_FORMS_HEADERS
  ${solarus_quest_editor_FORMS}
)

# Resources.
set(solarus_quest_editor_RESOURCES
)

# Compile resource files.
qt5_add_resources(solarus_quest_editor_RESOURCES_RCC
  ${solarus_quest_editor_RESOURCES}
)

# Main executable.
add_executable(solarus-quest-editor
  ${solarus_quest_editor_SOURCES}
  ${solarus_quest_editor_FORMS_HEADERS}
  ${solarus_quest_editor_RESOURCES_RCC}
)

target_link_libraries(solarus-quest-editor
  Qt5::Widgets
  "${SOLARUS_LIBRARIES}"
)

