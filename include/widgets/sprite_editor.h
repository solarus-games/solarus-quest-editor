/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_SPRITE_EDITOR_H
#define SOLARUSEDITOR_SPRITE_EDITOR_H

#include "widgets/editor.h"
#include "ui_sprite_editor.h"
#include <QMenu>

namespace SolarusEditor {

class SpriteModel;

/**
 * \brief A widget to edit graphically a sprite file.
 */
class SpriteEditor : public Editor {
  Q_OBJECT

public:

  SpriteEditor(Quest& quest, const QString& path, QWidget* parent = nullptr);
  ~SpriteEditor();

  SpriteModel& get_model();

  void save() override;
  void reload_settings() override;

public slots:

  void update();

  void update_sprite_id_field();

  void update_description_to_gui();
  void set_description_from_gui();

  void update_selection();
  void create_requested();
  void create_animation_requested();
  void rename_animation_requested();
  void create_direction_requested();
  void add_direction_requested(const QRect& frame);
  void duplicate_requested();
  void duplicate_selected_direction_requested(const QPoint &position);
  void delete_requested();
  void delete_direction_requested();

  void update_animation_view();
  void update_default_animation_field();
  void change_default_animation_requested();
  void update_animation_source_image_field();
  void change_animation_source_image_requested();
  void refresh_animation_source_image();
  void tileset_selector_activated();
  void update_animation_frame_delay_field();
  void change_animation_frame_delay_requested();
  void update_animation_loop_on_frame_field();
  void change_animation_loop_on_frame_requested();

  void update_direction_view();
  void update_direction_size_field();
  void change_direction_size_requested();
  void update_direction_position_field();
  void change_direction_position_requested_from_field();
  void change_direction_position_requested(const QPoint& position);
  void update_direction_origin_field();
  void change_direction_origin_requested();
  void update_direction_num_frames_field();
  void change_direction_num_frames_requested();
  void update_direction_num_columns_field();
  void change_direction_num_columns_requested();

private:

  void load_settings();

  void auto_detect_grid_size();

  Ui::SpriteEditor ui;          /**< The sprite editor widgets. */
  QString sprite_id;            /**< Id of the sprite being edited. */
  SpriteModel* model;           /**< Sprite model being edited. */
  Quest& quest;                 /**< The quest. */
  QMenu create_context_menu;    /**< The create context menu. */
  QAction* create_animation;    /**< The create animation action. */
  QAction* create_direction;    /**< The create direction action. */
  bool auto_detect_grid;        /**< The grid auto detection option. */

};

}

#endif
