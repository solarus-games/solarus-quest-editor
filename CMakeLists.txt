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

# In debug, set strict warnings for developers.
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Werror -Wextra -pedantic ${CMAKE_CXX_FLAGS_DEBUG}")

# Find dependencies.
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/modules/")
option(SOLARUS_USE_LUAJIT "Use LuaJIT instead of default Lua (recommended)" ON)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Solarus REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(OpenAL REQUIRED)
find_package(VorbisFile REQUIRED)
find_package(Ogg REQUIRED)
find_package(ModPlug REQUIRED)
find_package(PhysFS REQUIRED)
if(SOLARUS_USE_LUAJIT)
  find_package(LuaJit REQUIRED)
else()
  find_package(Lua51 REQUIRED)
endif()

# Include directories.
include_directories(
  "${CMAKE_SOURCE_DIR}/include"
)

include_directories(SYSTEM
  "${SOLARUS_INCLUDE_DIR}"
  "${MODPLUG_INCLUDE_DIR}"  # Before SDL2 because we want the sndfile.h of ModPlug.
  "${SDL2_INCLUDE_DIR}"
  "${SDL2_TTF_INCLUDE_DIR}"
  "${SDL2_IMAGE_INCLUDE_DIR}"
  "${OPENAL_INCLUDE_DIR}"
  "${VORBISFILE_INCLUDE_DIR}"
  "${OGG_INCLUDE_DIR}"
  "${LUA_INCLUDE_DIR}"
  "${PHYSFS_INCLUDE_DIR}"
)

# Source files.
set(solarus_quest_editor_SOURCES
  include/gui/closable_tab_bar.h
  include/gui/editor.h
  include/gui/editor_tabs.h
  include/gui/enum_selector.h
  include/gui/enum_selector.inl
  include/gui/gui_tools.h
  include/gui/lua_syntax_highlighter.h
  include/gui/main_window.h
  include/gui/new_resource_element_dialog.h
  include/gui/quest_files_model.h
  include/gui/quest_tree_view.h
  include/gui/text_editor.h
  include/gui/text_editor_widget.h
  include/gui/tile_patterns_list_view.h
  include/gui/tileset_editor.h
  include/gui/tileset_scene.h
  include/gui/tileset_view.h
  include/color.h
  include/editor_exception.h
  include/enum_traits.h
  include/ground_traits.h
  include/layer_traits.h
  include/natural_comparator.h
  include/quest.h
  include/quest_manager.h
  include/quest_resources.h
  include/tile_pattern_animation.h
  include/tile_pattern_animation_traits.h
  include/tile_pattern_separation.h
  include/tile_pattern_separation_traits.h
  include/tileset_model.h
  src/gui/closable_tab_bar.cpp
  src/gui/editor.cpp
  src/gui/editor_tabs.cpp
  src/gui/gui_tools.cpp
  src/gui/lua_syntax_highlighter.cpp
  src/gui/main_window.cpp
  src/gui/new_resource_element_dialog.cpp
  src/gui/quest_files_model.cpp
  src/gui/quest_tree_view.cpp
  src/gui/text_editor.cpp
  src/gui/text_editor_widget.cpp
  src/gui/tile_patterns_list_view.cpp
  src/gui/tileset_editor.cpp
  src/gui/tileset_scene.cpp
  src/gui/tileset_view.cpp
  src/color.cpp
  src/editor_exception.cpp
  src/ground_traits.cpp
  src/layer_traits.cpp
  src/main.cpp
  src/quest.cpp
  src/quest_manager.cpp
  src/quest_resources.cpp
  src/tile_pattern_animation_traits.cpp
  src/tile_pattern_separation_traits.cpp
  src/tileset_model.cpp
)

# UI files.
set(solarus_quest_editor_FORMS
  src/gui/main_window.ui
  src/gui/new_resource_element_dialog.ui
  src/gui/tileset_editor.ui
)

# Generate .h from .ui.
qt5_wrap_ui(solarus_quest_editor_FORMS_HEADERS
  ${solarus_quest_editor_FORMS}
)

# Resources.
set(solarus_quest_editor_RESOURCES
  resources/resources.qrc
)

# Compile resource files.
qt5_add_resources(solarus_quest_editor_RESOURCES_RCC
  ${solarus_quest_editor_RESOURCES}
)

# Translations.
set(solarus_quest_editor_TRANSLATIONS
  translations/solarus_editor_fr.ts
)
qt5_add_translation(solarus_quest_editor_TRANSLATIONS_QM
  ${solarus_quest_editor_TRANSLATIONS}
)

# Main executable.
add_executable(solarus-quest-editor
  ${solarus_quest_editor_SOURCES}
  ${solarus_quest_editor_FORMS_HEADERS}
  ${solarus_quest_editor_RESOURCES_RCC}
  ${solarus_quest_editor_TRANSLATIONS_QM}
)

target_link_libraries(solarus-quest-editor
  Qt5::Widgets
  "${SOLARUS_LIBRARIES}"
  "${SDL2_LIBRARY}"
  "${SDL2_IMAGE_LIBRARY}"
  "${SDL2_TTF_LIBRARY}"
  "${OPENAL_LIBRARY}"
  "${LUA_LIBRARY}"
  "${DL_LIBRARY}"
  "${PHYSFS_LIBRARY}"
  "${VORBISFILE_LIBRARY}"
  "${OGG_LIBRARY}"
  "${MODPLUG_LIBRARY}"
)
