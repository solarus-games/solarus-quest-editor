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
#ifndef SOLARUSEDITOR_EDITOR_SETTINGS_H
#define SOLARUSEDITOR_EDITOR_SETTINGS_H

#include <QSettings>

namespace SolarusEditor {

/**
 * @brief Manages settings of the quest editor.
 */
class EditorSettings {

public:

  // General keys.
  static const QString working_directory;
  static const QString current_quest;
  static const QString last_quests;
  static const QString last_files;
  static const QString last_file;
  static const QString restore_last_files;
  static const QString save_files_before_running;
  static const QString no_audio;
  static const QString video_acceleration;
  static const QString quest_size;

  // Console keys.
  static const QString console_history;

  // Text editor keys.
  static const QString font_family;
  static const QString font_size;
  static const QString tab_length;
  static const QString replace_tab_by_spaces;

  // Map editor keys.
  static const QString map_main_background;
  static const QString map_main_zoom;
  static const QString map_grid_show_at_opening;
  static const QString map_grid_size;
  static const QString map_grid_style;
  static const QString map_grid_color;
  static const QString map_tileset_background;
  static const QString map_tileset_zoom;

  // Sprite editor keys.
  static const QString sprite_main_background;
  static const QString sprite_main_zoom;
  static const QString sprite_grid_show_at_opening;
  static const QString sprite_grid_size;
  static const QString sprite_grid_style;
  static const QString sprite_grid_color;
  static const QString sprite_auto_detect_grid;
  static const QString sprite_previewer_background;
  static const QString sprite_previewer_zoom;
  static const QString sprite_origin_show_at_opening;
  static const QString sprite_origin_color;

  // Tileset editor keys.
  static const QString tileset_background;
  static const QString tileset_zoom;
  static const QString tileset_grid_show_at_opening;
  static const QString tileset_grid_size;
  static const QString tileset_grid_style;
  static const QString tileset_grid_color;

  EditorSettings();

  static void load_default_application_settings();

  QVariant get_value(const QString& key);
  bool get_value_bool(const QString& key);
  int get_value_int(const QString& key);
  double get_value_double(const QString& key);
  QString get_value_string(const QString& key);
  QStringList get_value_string_list(const QString& key);
  QSize get_value_size(const QString& key);
  QColor get_value_color(const QString& key);

  QVariant get_default(const QString& key);
  bool get_default_bool(const QString& key);
  int get_default_int(const QString& key);
  double get_default_double(const QString& key);
  QString get_default_string(const QString& key);
  QSize get_default_size(const QString& key);
  QColor get_default_color(const QString& key);

  void set_value(const QString& key, const QVariant& value);
  void set_value_color(const QString& key, const QColor& value);

  void restore_default();

private:

  QSettings settings;  /**< The wrapped settings. */

  static QMap<QString, QVariant>
    default_values;    /**< The default values. */

};

}

#endif
