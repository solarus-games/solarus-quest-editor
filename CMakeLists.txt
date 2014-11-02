# CMake 2.8.11 or above is necessary for recent Qt5 cmake helper functions.
cmake_minimum_required(VERSION 2.8.11)  

project(solarus-quest-editor)

# As Qt generates ui header files, the build directory needs to be an include
# directory too.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Tell CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Compile as C++11.
if(MINGW)
  # To avoid a compilation error in vorbisfile.h with fseeko64.
  set(CMAKE_CXX_FLAGS "-std=gnu++11 ${CMAKE_CXX_FLAGS}")
else()
  set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
endif()

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
  include/gui/change_pattern_id_dialog.h
  include/gui/closable_tab_bar.h
  include/gui/editor.h
  include/gui/editor_tabs.h
  include/gui/external_script_dialog.h
  include/gui/enum_menus.h
  include/gui/enum_menus.inl
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
  include/file_tools.h
  include/ground_traits.h
  include/layer_traits.h
  include/natural_comparator.h
  include/new_quest_builder.h
  include/obsolete_editor_exception.h
  include/obsolete_quest_exception.h
  include/pattern_animation.h
  include/pattern_animation_traits.h
  include/pattern_separation.h
  include/pattern_separation_traits.h
  include/quest.h
  include/quest_properties.h
  include/quest_resources.h
  include/rectangle.h
  include/tileset_model.h
  src/gui/change_pattern_id_dialog.cpp
  src/gui/closable_tab_bar.cpp
  src/gui/editor.cpp
  src/gui/editor_tabs.cpp
  src/gui/external_script_dialog.cpp
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
  src/file_tools.cpp
  src/ground_traits.cpp
  src/layer_traits.cpp
  src/main.cpp
  src/new_quest_builder.cpp
  src/obsolete_editor_exception.cpp
  src/obsolete_quest_exception.cpp
  src/pattern_animation_traits.cpp
  src/pattern_separation_traits.cpp
  src/quest.cpp
  src/quest_properties.cpp
  src/quest_resources.cpp
  src/rectangle.cpp
  src/tileset_model.cpp
)

# UI files.
set(solarus_quest_editor_FORMS
  src/gui/change_pattern_id_dialog.ui
  src/gui/external_script_dialog.ui
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
  resources/initial_quest.qrc
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
  "${SOL2MAIN_LIBRARY}"
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

# Platform specific.

# Windows: disable the console.
if(WIN32)
  if(MSVC)
    set_target_properties(solarus-quest-editor PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-mwindows ${CMAKE_CXX_FLAGS}")
  endif()
endif()

