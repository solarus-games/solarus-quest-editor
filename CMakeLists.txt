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
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
  set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++ ${CMAKE_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
else()
  set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
endif()

# Compile in release mode by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
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
find_package(SolarusGui REQUIRED)
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

# Explicit link to libdl is needed for Lua on some systems.
find_library(DL_LIBRARY dl)
if("${DL_LIBRARY}" MATCHES DL_LIBRARY-NOTFOUND)
  set(DL_LIBRARY "")
endif()

# Include directories.
include_directories(
  "${CMAKE_BINARY_DIR}"
  "${CMAKE_SOURCE_DIR}/include"
)

include_directories(SYSTEM
  "${SOLARUS_INCLUDE_DIR}"
  "${SOLARUS_GUI_INCLUDE_DIR}"
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
  include/widgets/change_dialog_id_dialog.h
  include/widgets/change_pattern_id_dialog.h
  include/widgets/change_resource_id_dialog.h
  include/widgets/change_source_image_dialog.h
  include/widgets/change_string_key_dialog.h
  include/widgets/closable_tab_bar.h
  include/widgets/color_picker.h
  include/widgets/dialog_properties_table.h
  include/widgets/dialogs_editor.h
  include/widgets/dialogs_tree_view.h
  include/widgets/edit_entity_dialog.h
  include/widgets/editor.h
  include/widgets/editor_tabs.h
  include/widgets/entity_item.h
  include/widgets/entity_selector.h
  include/widgets/enum_menus.h
  include/widgets/enum_menus.inl
  include/widgets/enum_selector.h
  include/widgets/enum_selector.inl
  include/widgets/external_script_dialog.h
  include/widgets/find_text_dialog.h
  include/widgets/get_animation_name_dialog.h
  include/widgets/gui_tools.h
  include/widgets/lua_syntax_highlighter.h
  include/widgets/map_editor.h
  include/widgets/map_view.h
  include/widgets/map_scene.h
  include/widgets/main_window.h
  include/widgets/mouse_coordinates_tracking_tool.h
  include/widgets/music_chooser.h
  include/widgets/new_resource_element_dialog.h
  include/widgets/new_string_dialog.h
  include/widgets/pair_spin_box.h
  include/widgets/pan_tool.h
  include/widgets/plain_text_edit.h
  include/widgets/quest_properties_editor.h
  include/widgets/quest_tree_view.h
  include/widgets/resource_model.h
  include/widgets/resource_selector.h
  include/widgets/settings_dialog.h
  include/widgets/sound_chooser.h
  include/widgets/sprite_editor.h
  include/widgets/sprite_tree_view.h
  include/widgets/sprite_scene.h
  include/widgets/sprite_view.h
  include/widgets/sprite_previewer.h
  include/widgets/strings_editor.h
  include/widgets/strings_tree_view.h
  include/widgets/text_editor.h
  include/widgets/text_editor_widget.h
  include/widgets/tile_patterns_list_view.h
  include/widgets/tileset_editor.h
  include/widgets/tileset_scene.h
  include/widgets/tileset_view.h
  include/widgets/zoom_tool.h
  include/audio.h
  include/color.h
  include/dialogs_model.h
  include/editor_exception.h
  include/editor_settings.h
  include/enum_traits.h
  include/file_tools.h
  include/grid_style.h
  include/ground_traits.h
  include/indexed_string_tree.h
  include/map_model.h
  include/natural_comparator.h
  include/new_quest_builder.h
  include/obsolete_editor_exception.h
  include/obsolete_quest_exception.h
  include/pattern_animation.h
  include/pattern_animation_traits.h
  include/pattern_repeat_mode_traits.h
  include/pattern_separation.h
  include/pattern_separation_traits.h
  include/point.h
  include/quest.h
  include/quest_files_model.h
  include/quest_properties.h
  include/quest_resources.h
  include/rectangle.h
  include/refactoring.h
  include/resize_mode.h
  include/size.h
  include/sprite_model.h
  include/starting_location_mode_traits.h
  include/strings_model.h
  include/tileset_model.h
  include/transition_traits.h
  include/version.h
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
  src/widgets/change_dialog_id_dialog.cpp
  src/widgets/change_pattern_id_dialog.cpp
  src/widgets/change_resource_id_dialog.cpp
  src/widgets/change_source_image_dialog.cpp
  src/widgets/change_string_key_dialog.cpp
  src/widgets/closable_tab_bar.cpp
  src/widgets/color_picker.cpp
  src/widgets/dialog_properties_table.cpp
  src/widgets/dialogs_editor.cpp
  src/widgets/dialogs_tree_view.cpp
  src/widgets/edit_entity_dialog.cpp
  src/widgets/editor.cpp
  src/widgets/editor_tabs.cpp
  src/widgets/entity_item.cpp
  src/widgets/entity_selector.cpp
  src/widgets/external_script_dialog.cpp
  src/widgets/find_text_dialog.cpp
  src/widgets/get_animation_name_dialog.cpp
  src/widgets/gui_tools.cpp
  src/widgets/lua_syntax_highlighter.cpp
  src/widgets/main_window.cpp
  src/widgets/map_editor.cpp
  src/widgets/map_view.cpp
  src/widgets/map_scene.cpp
  src/widgets/mouse_coordinates_tracking_tool.cpp
  src/widgets/music_chooser.cpp
  src/widgets/new_resource_element_dialog.cpp
  src/widgets/new_string_dialog.cpp
  src/widgets/pair_spin_box.cpp
  src/widgets/pan_tool.cpp
  src/widgets/quest_properties_editor.cpp
  src/widgets/quest_tree_view.cpp
  src/widgets/resource_model.cpp
  src/widgets/resource_selector.cpp
  src/widgets/settings_dialog.cpp
  src/widgets/sound_chooser.cpp
  src/widgets/sprite_editor.cpp
  src/widgets/sprite_tree_view.cpp
  src/widgets/sprite_scene.cpp
  src/widgets/sprite_view.cpp
  src/widgets/sprite_previewer.cpp
  src/widgets/strings_editor.cpp
  src/widgets/strings_tree_view.cpp
  src/widgets/text_editor.cpp
  src/widgets/text_editor_widget.cpp
  src/widgets/tile_patterns_list_view.cpp
  src/widgets/tileset_editor.cpp
  src/widgets/tileset_scene.cpp
  src/widgets/tileset_view.cpp
  src/widgets/zoom_tool.cpp
  src/audio.cpp
  src/color.cpp
  src/dialogs_model.cpp
  src/editor_exception.cpp
  src/editor_settings.cpp
  src/file_tools.cpp
  src/grid_style.cpp
  src/ground_traits.cpp
  src/indexed_string_tree.cpp
  src/main.cpp
  src/map_model.cpp
  src/new_quest_builder.cpp
  src/obsolete_editor_exception.cpp
  src/obsolete_quest_exception.cpp
  src/pattern_animation_traits.cpp
  src/pattern_repeat_mode_traits.cpp
  src/pattern_separation_traits.cpp
  src/point.cpp
  src/quest.cpp
  src/quest_files_model.cpp
  src/quest_properties.cpp
  src/quest_resources.cpp
  src/rectangle.cpp
  src/refactoring.cpp
  src/size.cpp
  src/sprite_model.cpp
  src/starting_location_mode_traits.cpp
  src/strings_model.cpp
  src/tileset_model.cpp
  src/transition_traits.cpp
  src/view_settings.cpp
)

# Add an icon for the executable in Windows.
if(WIN32)
  set(solarus_quest_editor_SOURCES
    ${solarus_quest_editor_SOURCES}
    cmake/win32/resources.rc
  )
endif()

# UI files.
set(solarus_quest_editor_FORMS
  src/widgets/change_dialog_id_dialog.ui
  src/widgets/change_pattern_id_dialog.ui
  src/widgets/change_resource_id_dialog.ui
  src/widgets/change_source_image_dialog.ui
  src/widgets/change_string_key_dialog.ui
  src/widgets/dialogs_editor.ui
  src/widgets/edit_entity_dialog.ui
  src/widgets/external_script_dialog.ui
  src/widgets/find_text_dialog.ui
  src/widgets/main_window.ui
  src/widgets/map_editor.ui
  src/widgets/new_resource_element_dialog.ui
  src/widgets/new_string_dialog.ui
  src/widgets/quest_properties_editor.ui
  src/widgets/settings_dialog.ui
  src/widgets/sprite_editor.ui
  src/widgets/sprite_previewer.ui
  src/widgets/strings_editor.ui
  src/widgets/tileset_editor.ui
)

# Generate .h from .ui.
qt5_wrap_ui(solarus_quest_editor_FORMS_HEADERS
  ${solarus_quest_editor_FORMS}
)

# Resources.
set(solarus_quest_editor_RESOURCES
  resources/images.qrc
  resources/initial_files.qrc
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
  "${SOLARUS_GUI_LIBRARIES}"
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

# Set files to install
install(TARGETS solarus-quest-editor
  RUNTIME DESTINATION bin
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

# Configuration for OSX build and deployment.
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  include(cmake/apple/OSXBuild.cmake)
endif()
