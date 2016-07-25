/*
 * Copyright (C) 2014-2016 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus Quest Editor is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus Quest Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "grid_style.h"
#include "editor_settings.h"
#include <QPalette>
#include <QSize>

namespace SolarusEditor {

// General keys.
const QString EditorSettings::working_directory = "working_directory";
const QString EditorSettings::last_quests = "last_quests";
const QString EditorSettings::last_files = "last_files";
const QString EditorSettings::last_file = "last_file";
const QString EditorSettings::restore_last_files = "restore_last_files";
const QString EditorSettings::save_files_before_running = "save_files_before_running";
const QString EditorSettings::no_audio = "no_audio";
const QString EditorSettings::video_acceleration = "video_acceleration";
const QString EditorSettings::quest_size = "quest_size";

// Console keys.
const QString EditorSettings::console_history = "console_history";

// Text editor keys.
const QString EditorSettings::font_family = "text_editor/font_family";
const QString EditorSettings::font_size = "text_editor/font_size";
const QString EditorSettings::tab_length = "text_editor/tab_length";
const QString EditorSettings::replace_tab_by_spaces =
  "text_editor/replace_tab_by_spaces";

// Map editor keys.
const QString EditorSettings::map_main_background =
  "map_editor/main_background";
const QString EditorSettings::map_grid_show_at_opening =
  "map_editor/grid_show_at_opening";
const QString EditorSettings::map_grid_size = "map_editor/grid_size";
const QString EditorSettings::map_grid_style = "map_editor/grid_style";
const QString EditorSettings::map_grid_color = "map_editor/grid_color";
const QString EditorSettings::map_tileset_background =
  "map_editor/tileset_background";

// Sprite editor keys.
const QString EditorSettings::sprite_main_background =
  "sprite_editor/main_background";
const QString EditorSettings::sprite_grid_show_at_opening =
  "sprite_editor/grid_show_at_opening";
const QString EditorSettings::sprite_grid_size = "sprite_editor/grid_size";
const QString EditorSettings::sprite_grid_style = "sprite_editor/grid_style";
const QString EditorSettings::sprite_grid_color = "sprite_editor/grid_color";
const QString EditorSettings::sprite_auto_detect_grid =
  "sprite_editor/auto_detect_grid";
const QString EditorSettings::sprite_previewer_background =
  "sprite_editor/previewer_background";
const QString EditorSettings::sprite_origin_show_at_opening =
  "sprite_editor/origin_show_at_opening";
const QString EditorSettings::sprite_origin_color = "sprite_editor/origin_color";

// Tileset editor keys.
const QString EditorSettings::tileset_background = "tileset_editor/background";
const QString EditorSettings::tileset_grid_show_at_opening =
  "tileset_editor/grid_show_at_opening";
const QString EditorSettings::tileset_grid_size = "tileset_editor/grid_size";
const QString EditorSettings::tileset_grid_style = "tileset_editor/grid_style";
const QString EditorSettings::tileset_grid_color = "tileset_editor/grid_color";

QMap<QString, QVariant> EditorSettings::default_values = {

  // General.
  { EditorSettings::working_directory, "" },
  { EditorSettings::last_quests, QStringList() },
  { EditorSettings::last_files, QStringList() },
  { EditorSettings::last_file, "" },
  { EditorSettings::restore_last_files, true },
  { EditorSettings::save_files_before_running, "ask" },
  { EditorSettings::no_audio, false },
  { EditorSettings::video_acceleration, true },
  { EditorSettings::quest_size, QSize() },

  // Console.
  { EditorSettings::console_history, QStringList() },

  // Text editor.
  { EditorSettings::font_family, "DejaVu Sans Mono" },
  { EditorSettings::font_size, 10 },
  { EditorSettings::tab_length, 2 },
  { EditorSettings::replace_tab_by_spaces, true },

  // Map editor.
  { EditorSettings::map_main_background, "#888888" },
  { EditorSettings::map_grid_show_at_opening, false },
  { EditorSettings::map_grid_size, QSize(16, 16) },
  { EditorSettings::map_grid_style, static_cast<int>(GridStyle::DASHED) },
  { EditorSettings::map_grid_color, "#000000" },
  { EditorSettings::map_tileset_background, "#888888" },

  // Sprite editor.
  { EditorSettings::sprite_main_background, "#888888" },
  { EditorSettings::sprite_grid_show_at_opening, false },
  { EditorSettings::sprite_grid_size, QSize(16, 16) },
  { EditorSettings::sprite_grid_style, static_cast<int>(GridStyle::DASHED) },
  { EditorSettings::sprite_grid_color, "#000000" },
  { EditorSettings::sprite_auto_detect_grid, false },
  { EditorSettings::sprite_previewer_background, "#888888" },
  { EditorSettings::sprite_origin_show_at_opening, false },
  { EditorSettings::sprite_origin_color, "#0000ff" },

  // Tileset editor.
  { EditorSettings::tileset_background, "#888888" },
  { EditorSettings::tileset_grid_show_at_opening, false },
  { EditorSettings::tileset_grid_size, QSize(16, 16) },
  { EditorSettings::tileset_grid_style, static_cast<int>(GridStyle::DASHED) },
  { EditorSettings::tileset_grid_color, "#000000" }
};

/**
 * @brief Creates settings.
 */
EditorSettings::EditorSettings() :
  settings() {
}

/**
 * @brief Loads the default application settings.
 */
void EditorSettings::load_default_application_settings() {

  QPalette palette;
  QString alternate_color = palette.alternateBase().color().name();
  QString base_color = palette.base().color().name();

  // Map editor.
  default_values[map_main_background] = alternate_color;
  default_values[map_tileset_background] = base_color;

  // Sprite editor.
  default_values[sprite_main_background] = base_color;
  default_values[sprite_previewer_background] = base_color;

  // Tileset editor.
  default_values[tileset_background] = base_color;
}

/**
 * @brief Returns a settings value.
 * @param key The key of the setting.
 * @return The value of the setting.
 */
QVariant EditorSettings::get_value(const QString& key) {

  if (default_values.contains(key)) {
    return settings.value(key, default_values[key]);
  }
  return settings.value(key);
}

/**
 * @brief Returns a settings boolean value.
 * @param key The key of the setting.
 * @return The boolean value of the setting.
 */
bool EditorSettings::get_value_bool(const QString& key) {

  return get_value(key).toBool();
}

/**
 * @brief Returns a settings integer value.
 * @param key The key of the setting.
 * @return The integer value of the setting.
 */
int EditorSettings::get_value_int(const QString& key) {

  return get_value(key).toInt();
}

/**
 * @brief Returns a settings string value.
 * @param key The key of the setting.
 * @return The string value of the setting.
 */
QString EditorSettings::get_value_string(const QString& key) {

  return get_value(key).toString();
}

/**
 * @brief Returns a settings string list value.
 * @param key The key of the setting.
 * @return The string list value of the setting.
 */
QStringList EditorSettings::get_value_string_list(const QString& key) {

  return get_value(key).toStringList();
}

/**
 * @brief Returns a settings size value.
 * @param key The key of the setting.
 * @return The size value of the setting.
 */
QSize EditorSettings::get_value_size(const QString& key) {

  return get_value(key).toSize();
}

/**
 * @brief Returns a settings color value.
 * @param key The key of the setting.
 * @return The color value of the setting.
 */
QColor EditorSettings::get_value_color(const QString& key) {

  return QColor(get_value(key).toString());
}

/**
 * @brief Returns a settings default value.
 * @param key The key of the setting.
 * @return The default value of the setting.
 */
QVariant EditorSettings::get_default(const QString& key) {

  if (!default_values.contains(key)) {
    return QVariant();
  }
  return default_values[key];
}

/**
 * @brief Returns a settings default boolean value.
 * @param key The key of the setting.
 * @return The default boolean value of the setting.
 */
bool EditorSettings::get_default_bool(const QString& key) {

  return get_default(key).toBool();
}

/**
 * @brief Returns a settings default integer value.
 * @param key The key of the setting.
 * @return The default integer value of the setting.
 */
int EditorSettings::get_default_int(const QString& key) {

  return get_default(key).toInt();
}

/**
 * @brief Returns a settings default string value.
 * @param key The key of the setting.
 * @return The default string value of the setting.
 */
QString EditorSettings::get_default_string(const QString& key) {

  return get_default(key).toString();
}

/**
 * @brief Returns a settings default size value.
 * @param key The key of the setting.
 * @return The default size value of the setting.
 */
QSize EditorSettings::get_default_size(const QString& key) {

  return get_default(key).toSize();
}

/**
 * @brief Returns a settings default color value.
 * @param key The key of the setting.
 * @return The default color value of the setting.
 */
QColor EditorSettings::get_default_color(const QString& key) {

  return QColor(get_default(key).toString());
}

/**
 * @brief Changes a settings value.
 * @param key The key of the setting.
 * @param value The new value of the setting.
 */
void EditorSettings::set_value(const QString& key, const QVariant& value) {

  settings.setValue(key, value);
}

/**
 * @brief Changes a settings color value.
 * @param key The key of the setting.
 * @param value The new color value of the setting.
 */
void EditorSettings::set_value_color(const QString& key, const QColor& value) {

  settings.setValue(key, value.name());
}

/**
 * @brief Restores all default values.
 */
void EditorSettings::restore_default() {

  Q_FOREACH (const QString& key, default_values.keys()) {
    settings.setValue(key, default_values[key]);
  }
}

}
