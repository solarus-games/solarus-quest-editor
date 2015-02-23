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
find_package(Qt5Core REQUIRED)
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
  include/entities/block.h
  include/entities/chest.h
  include/entities/crystal.h
  include/entities/crystal_block.h
  include/entities/custom_entity.h
  include/entities/destination.h
  include/entities/destructible.h
  include/entities/door.h
  include/entities/dynamic_tile.h
  include/entities/enemy.h
  include/entities/entity_model.h
  include/entities/entity_traits.h
  include/entities/jumper.h
  include/entities/npc.h
  include/entities/pickable.h
  include/entities/sensor.h
  include/entities/separator.h
  include/entities/shop_treasure.h
  include/entities/stairs.h
  include/entities/stream.h
  include/entities/switch.h
  include/entities/teletransporter.h
  include/entities/tile.h
  include/entities/wall.h
  include/gui/change_pattern_id_dialog.h
  include/gui/change_source_image_dialog.h
  include/gui/closable_tab_bar.h
  include/gui/editor.h
  include/gui/editor_tabs.h
  include/gui/external_script_dialog.h
  include/gui/enum_menus.h
  include/gui/enum_menus.inl
  include/gui/enum_selector.h
  include/gui/enum_selector.inl
  include/gui/get_animation_name_dialog.h
  include/gui/gui_tools.h
  include/gui/lua_syntax_highlighter.h
  include/gui/map_editor.h
  include/gui/map_view.h
  include/gui/map_scene.h
  include/gui/main_window.h
  include/gui/mouse_coordinates_tracking_tool.h
  include/gui/new_resource_element_dialog.h
  include/gui/pan_tool.h
  include/gui/quest_files_model.h
  include/gui/quest_tree_view.h
  include/gui/resource_model.h
  include/gui/resource_selector.h
  include/gui/sprite_editor.h
  include/gui/sprite_tree_view.h
  include/gui/sprite_scene.h
  include/gui/sprite_view.h
  include/gui/sprite_previewer.h
  include/gui/text_editor.h
  include/gui/text_editor_widget.h
  include/gui/tile_patterns_list_view.h
  include/gui/tileset_editor.h
  include/gui/tileset_scene.h
  include/gui/tileset_view.h
  include/gui/zoom_tool.h
  include/gui/quest_properties_editor.h
  include/color.h
  include/editor_exception.h
  include/enum_traits.h
  include/file_tools.h
  include/ground_traits.h
  include/layer_traits.h
  include/map_model.h
  include/natural_comparator.h
  include/new_quest_builder.h
  include/obsolete_editor_exception.h
  include/obsolete_quest_exception.h
  include/pattern_animation.h
  include/pattern_animation_traits.h
  include/pattern_separation.h
  include/pattern_separation_traits.h
  include/point.h
  include/quest.h
  include/quest_properties.h
  include/quest_resources.h
  include/rectangle.h
  include/size.h
  include/sprite_model.h
  include/tileset_model.h
  include/view_settings.h
  src/entities/block.cpp
  src/entities/chest.cpp
  src/entities/crystal.cpp
  src/entities/crystal_block.cpp
  src/entities/custom_entity.cpp
  src/entities/destination.cpp
  src/entities/destructible.cpp
  src/entities/door.cpp
  src/entities/dynamic_tile.cpp
  src/entities/enemy.cpp
  src/entities/entity_model.cpp
  src/entities/entity_traits.cpp
  src/entities/jumper.cpp
  src/entities/npc.cpp
  src/entities/pickable.cpp
  src/entities/sensor.cpp
  src/entities/separator.cpp
  src/entities/shop_treasure.cpp
  src/entities/stairs.cpp
  src/entities/stream.cpp
  src/entities/switch.cpp
  src/entities/teletransporter.cpp
  src/entities/tile.cpp
  src/entities/wall.cpp
  src/gui/change_pattern_id_dialog.cpp
  src/gui/change_source_image_dialog.cpp
  src/gui/closable_tab_bar.cpp
  src/gui/editor.cpp
  src/gui/editor_tabs.cpp
  src/gui/external_script_dialog.cpp
  src/gui/get_animation_name_dialog.cpp
  src/gui/gui_tools.cpp
  src/gui/lua_syntax_highlighter.cpp
  src/gui/main_window.cpp
  src/gui/map_editor.cpp
  src/gui/map_view.cpp
  src/gui/map_scene.cpp
  src/gui/mouse_coordinates_tracking_tool.cpp
  src/gui/new_resource_element_dialog.cpp
  src/gui/pan_tool.cpp
  src/gui/quest_files_model.cpp
  src/gui/quest_tree_view.cpp
  src/gui/resource_model.cpp
  src/gui/resource_selector.cpp
  src/gui/sprite_editor.cpp
  src/gui/sprite_tree_view.cpp
  src/gui/sprite_scene.cpp
  src/gui/sprite_view.cpp
  src/gui/sprite_previewer.cpp
  src/gui/text_editor.cpp
  src/gui/text_editor_widget.cpp
  src/gui/tile_patterns_list_view.cpp
  src/gui/tileset_editor.cpp
  src/gui/tileset_scene.cpp
  src/gui/tileset_view.cpp
  src/gui/zoom_tool.cpp
  src/gui/quest_properties_editor.cpp
  src/color.cpp
  src/editor_exception.cpp
  src/file_tools.cpp
  src/ground_traits.cpp
  src/layer_traits.cpp
  src/main.cpp
  src/map_model.cpp
  src/new_quest_builder.cpp
  src/obsolete_editor_exception.cpp
  src/obsolete_quest_exception.cpp
  src/pattern_animation_traits.cpp
  src/pattern_separation_traits.cpp
  src/point.cpp
  src/quest.cpp
  src/quest_properties.cpp
  src/quest_resources.cpp
  src/rectangle.cpp
  src/size.cpp
  src/sprite_model.cpp
  src/tileset_model.cpp
  src/view_settings.cpp
)

# UI files.
set(solarus_quest_editor_FORMS
  src/gui/change_pattern_id_dialog.ui
  src/gui/external_script_dialog.ui
  src/gui/main_window.ui
  src/gui/map_editor.ui
  src/gui/new_resource_element_dialog.ui
  src/gui/tileset_editor.ui
  src/gui/sprite_editor.ui
  src/gui/sprite_previewer.ui
  src/gui/change_source_image_dialog.ui
  src/gui/quest_properties_editor.ui
)

# Generate .h from .ui.
qt5_wrap_ui(solarus_quest_editor_FORMS_HEADERS
  ${solarus_quest_editor_FORMS}
)

# Resources.
set(solarus_quest_editor_RESOURCES
  resources/images.qrc
  resources/initial_quest.qrc
  resources/quest_converter.qrc
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

