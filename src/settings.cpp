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
#include "settings.h"
#include <QPalette>
#include <QSize>

// General keys.
const QString Settings::working_directory = "working_directory";
const QString Settings::last_quests = "last_quests";
const QString Settings::last_files = "last_files";
const QString Settings::last_file = "last_file";
const QString Settings::restore_last_files = "restore_last_files";
const QString Settings::save_files_before_running = "save_files_before_running";
const QString Settings::no_audio = "no_audio";
const QString Settings::video_acceleration = "video_acceleration";
const QString Settings::win_console = "win_console";
const QString Settings::quest_size = "quest_size";

// Console keys.
const QString Settings::console_history = "console_history";

// Text editor keys.
const QString Settings::font_family = "text_editor/font_family";
const QString Settings::font_size = "text_editor/font_size";
const QString Settings::tab_length = "text_editor/tab_length";
const QString Settings::replace_tab_by_spaces =
  "text_editor/replace_tab_by_spaces";

// Map editor keys.
const QString Settings::map_background = "map_editor/background";
const QString Settings::map_grid_show_at_opening =
  "map_editor/grid_show_at_opening";
const QString Settings::map_grid_size = "map_editor/grid_size";
const QString Settings::map_grid_style = "map_editor/grid_style";
const QString Settings::map_grid_color = "map_editor/grid_color";

// Sprite editor keys.
const QString Settings::sprite_main_background =
  "sprite_editor/main_background";
const QString Settings::sprite_grid_show_at_opening =
  "sprite_editor/grid_show_at_opening";
const QString Settings::sprite_grid_size = "sprite_editor/grid_size";
const QString Settings::sprite_grid_style = "sprite_editor/grid_style";
const QString Settings::sprite_grid_color = "sprite_editor/grid_color";
const QString Settings::sprite_auto_detect_grid =
  "sprite_editor/auto_detect_grid";
const QString Settings::sprite_previewer_background =
  "sprite_editor/previewer_background";
const QString Settings::sprite_origin_show_at_opening =
  "sprite_editor/origin_show_at_opening";
const QString Settings::sprite_origin_color = "sprite_editor/origin_color";

QMap<QString, QVariant> Settings::default_values = {

  // General.
  { Settings::working_directory, "" },
  { Settings::last_quests, QStringList() },
  { Settings::last_files, QStringList() },
  { Settings::last_file, "" },
  { Settings::restore_last_files, true },
  { Settings::save_files_before_running, "ask" },
  { Settings::no_audio, false },
  { Settings::video_acceleration, true },
  { Settings::win_console, false },
  { Settings::quest_size, QSize() },

  // Console.
  { Settings::console_history, QStringList() },

  // Text editor.
  { Settings::font_family, "DejaVu Sans Mono" },
  { Settings::font_size, 10 },
  { Settings::tab_length, 2 },
  { Settings::replace_tab_by_spaces, true },

  // Map editor.
  { Settings::map_background, "#888888" },
  { Settings::map_grid_show_at_opening, false },
  { Settings::map_grid_size, QSize(16, 16) },
  { Settings::map_grid_style, static_cast<int>(GridStyle::DASHED) },
  { Settings::map_grid_color, "#000000" },

  // Sprite editor.
  { Settings::sprite_main_background, "#888888" },
  { Settings::sprite_grid_show_at_opening, false },
  { Settings::sprite_grid_size, QSize(16, 16) },
  { Settings::sprite_grid_style, static_cast<int>(GridStyle::DASHED) },
  { Settings::sprite_grid_color, "#000000" },
  { Settings::sprite_auto_detect_grid, false },
  { Settings::sprite_previewer_background, "#888888" },
  { Settings::sprite_origin_show_at_opening, false },
  { Settings::sprite_origin_color, "#0000ff" }
};

/**
 * @brief Creates settings.
 */
Settings::Settings() :
  settings("solarus", "solarus-quest-editor") {
}

/**
 * @brief Loads the default application settings.
 */
void Settings::load_default_application_settings() {

  QPalette palette;
  QString alternate_color = palette.alternateBase().color().name();
  QString base_color = palette.base().color().name();

  // Map editor.
  default_values[map_background] = alternate_color;

  // Sprite editor.
  default_values[sprite_main_background] = base_color;
  default_values[sprite_previewer_background] = base_color;
}

/**
 * @brief Returns a settings value.
 * @param key The key of the setting.
 * @return The value of the setting.
 */
QVariant Settings::get_value(const QString& key) {

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
bool Settings::get_value_bool(const QString& key) {

  return get_value(key).toBool();
}

/**
 * @brief Returns a settings integer value.
 * @param key The key of the setting.
 * @return The integer value of the setting.
 */
int Settings::get_value_int(const QString& key) {

  return get_value(key).toInt();
}

/**
 * @brief Returns a settings string value.
 * @param key The key of the setting.
 * @return The string value of the setting.
 */
QString Settings::get_value_string(const QString& key) {

  return get_value(key).toString();
}

/**
 * @brief Returns a settings string list value.
 * @param key The key of the setting.
 * @return The string list value of the setting.
 */
QStringList Settings::get_value_string_list(const QString& key) {

  return get_value(key).toStringList();
}

/**
 * @brief Returns a settings size value.
 * @param key The key of the setting.
 * @return The size value of the setting.
 */
QSize Settings::get_value_size(const QString& key) {

  return get_value(key).toSize();
}

/**
 * @brief Returns a settings color value.
 * @param key The key of the setting.
 * @return The color value of the setting.
 */
QColor Settings::get_value_color(const QString& key) {

  return QColor(get_value(key).toString());
}

/**
 * @brief Returns a settings default value.
 * @param key The key of the setting.
 * @return The default value of the setting.
 */
QVariant Settings::get_default(const QString& key) {

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
bool Settings::get_default_bool(const QString& key) {

  return get_default(key).toBool();
}

/**
 * @brief Returns a settings default integer value.
 * @param key The key of the setting.
 * @return The default integer value of the setting.
 */
int Settings::get_default_int(const QString& key) {

  return get_default(key).toInt();
}

/**
 * @brief Returns a settings default string value.
 * @param key The key of the setting.
 * @return The default string value of the setting.
 */
QString Settings::get_default_string(const QString& key) {

  return get_default(key).toString();
}

/**
 * @brief Returns a settings default size value.
 * @param key The key of the setting.
 * @return The default size value of the setting.
 */
QSize Settings::get_default_size(const QString& key) {

  return get_default(key).toSize();
}

/**
 * @brief Returns a settings default color value.
 * @param key The key of the setting.
 * @return The default color value of the setting.
 */
QColor Settings::get_default_color(const QString& key) {

  return QColor(get_default(key).toString());
}

/**
 * @brief Changes a settings value.
 * @param key The key of the setting.
 * @param value The new value of the setting.
 */
void Settings::set_value(const QString& key, const QVariant& value) {

  settings.setValue(key, value);
}

/**
 * @brief Changes a settings color value.
 * @param key The key of the setting.
 * @param value The new color value of the setting.
 */
void Settings::set_value_color(const QString& key, const QColor& value) {

  settings.setValue(key, value.name());
}

/**
 * @brief Restores all default values.
 */
void Settings::restore_default() {

  for (QString key : default_values.keys()) {
    settings.setValue(key, default_values[key]);
  }
}
