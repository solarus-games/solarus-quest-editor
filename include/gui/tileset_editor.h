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
  virtual void cut() override;
  virtual void copy() override;
  virtual void paste() override;

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
  void update_ground_field();
  void ground_selector_activated();
  void update_animation_type_field();
  void update_animation_separation_field();
  void update_default_layer_field();

private:

  Ui::TilesetEditor ui;         /**< The tileset editor widgets. */
  QString tileset_id;           /**< Id of the tileset being edited. */
  TilesetModel* model;          /**< Tileset model being edited. */

};

#endif
