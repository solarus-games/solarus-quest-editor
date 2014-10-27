/*
 * Copyright (C) 2014 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_TILESET_EDITOR_H
#define SOLARUSEDITOR_TILESET_EDITOR_H

#include "gui/editor.h"
#include "ui_tileset_editor.h"

class TilesetModel;

/**
 * \brief A widget to edit graphically a tileset file.
 */
class TilesetEditor : public Editor {
  Q_OBJECT

public:

  TilesetEditor(Quest& quest, const QString& path, QWidget* parent = nullptr);

  TilesetModel& get_model();

  virtual void save() override;

public slots:

  void update();

  void update_tileset_id_field();
  void update_num_patterns_field();
  void update_background_color();
  void background_button_clicked();

  void update_description_to_gui();
  void set_description_from_gui();

  void update_pattern_view();
  void update_pattern_id_field();
  void change_selected_pattern_position_requested(const QPoint& position);
  void update_ground_field();
  void ground_selector_activated();
  void change_selected_patterns_ground_requested(Ground ground);
  void update_animation_type_field();
  void animation_type_selector_activated();
  void change_selected_patterns_animation_requested(PatternAnimation animation);
  void update_animation_separation_field();
  void animation_separation_selector_activated();
  void change_selected_patterns_separation_requested(PatternSeparation separation);
  void update_default_layer_field();
  void default_layer_selector_activated();
  void change_selected_patterns_default_layer_requested(Layer layer);

  void create_pattern_requested(
      const QString& pattern_id, const QRect& frame, Ground ground);
  void delete_selected_patterns_requested();
  void change_selected_pattern_id_requested();

private:

  Ui::TilesetEditor ui;         /**< The tileset editor widgets. */
  QString tileset_id;           /**< Id of the tileset being edited. */
  TilesetModel* model;          /**< Tileset model being edited. */

};

#endif
