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
#ifndef SOLARUSEDITOR_SETTINGS_DIALOG_H
#define SOLARUSEDITOR_SETTINGS_DIALOG_H

#include "ui_settings_dialog.h"
#include "include/settings.h"
#include <QDialog>

/**
 * @brief A dialog to edit settings of the editor.
 */
class SettingsDialog : public QDialog {
  Q_OBJECT

public:

  SettingsDialog(QWidget *parent = nullptr);

public slots:

  void done(int result) override;

  void reset();
  void restore_default();
  void apply();

  void update();

  void update_buttons();

  // General.
  void update_working_directory();
  void change_working_directory();
  void browse_working_directory();
  void update_save_files();
  void change_save_files();
  void update_no_audio();
  void change_no_audio();
  void update_video_acceleration();
  void change_video_acceleration();
  void update_win_console();
  void change_win_console();
  void update_quest_size();
  void change_quest_size();

  // Text editor.
  void update_font_family();
  void change_font_family();
  void update_font_size();
  void change_font_size();
  void update_tab_length();
  void change_tab_length();
  void update_replace_tab_by_spaces();
  void change_replace_tab_by_spaces();

  // Map editor.
  void update_map_background();
  void change_map_background();
  void update_map_grid_show_at_opening();
  void change_map_grid_show_at_opening();
  void update_map_grid_size();
  void change_map_grid_size();
  void update_map_grid_style();
  void change_map_grid_style();
  void update_map_grid_color();
  void change_map_grid_color();

  // Sprite editor.
  void update_sprite_main_background();
  void change_sprite_main_background();
  void update_sprite_grid_show_at_opening();
  void change_sprite_grid_show_at_opening();
  void update_sprite_grid_size();
  void change_sprite_grid_size();
  void update_sprite_grid_style();
  void change_sprite_grid_style();
  void update_sprite_grid_color();
  void change_sprite_grid_color();
  void update_sprite_auto_detect_grid();
  void change_sprite_auto_detect_grid();
  void update_sprite_previewer_background();
  void change_sprite_previewer_background();
  void update_sprite_origin_show_at_opening();
  void change_sprite_origin_show_at_opening();
  void update_sprite_origin_color();
  void change_sprite_origin_color();

signals:

  void settings_changed();

private:

  Ui::SettingsDialog ui;                    /**< The settings dialog widgets. */

  Settings settings;                        /**< The settings. */
  QMap<QString, QVariant> edited_settings;  /**< The edited settings. */

};

#endif
