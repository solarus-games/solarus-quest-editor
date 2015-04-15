/*
 * Copyright (C) 2014-2015 Christopho, Solarus - http://www.solarus-games.org
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
#ifndef SOLARUSEDITOR_EDIT_ENTITY_DIALOG_H
#define SOLARUSEDITOR_EDIT_ENTITY_DIALOG_H

#include "ui_edit_entity_dialog.h"
#include "entities/entity_traits.h"

class MapModel;

class EditEntityDialog : public QDialog {
  Q_OBJECT

public:

  explicit EditEntityDialog(EntityModel& entity_before, QWidget* parent = nullptr);

  const Quest& get_quest() const;
  MapModel& get_map() const;
  EntityIndex get_entity_index() const;
  EntityModel& get_entity_before() const;
  EntityModelPtr get_entity_after();

private:

  void initialize();
  void apply();
  void initialize_default();
  void apply_default();
  void initialize_destination();
  void apply_destination();
  void initialize_destination_map();
  void apply_destination_map();
  void initialize_direction();
  void apply_direction();
  void initialize_enabled_at_start();
  void apply_enabled_at_start();
  void initialize_layer();
  void apply_layer();
  void initialize_name();
  void apply_name();
  void initialize_size();
  void apply_size();
  void initialize_sound();
  void apply_sound();
  void initialize_sprite();
  void apply_sprite();
  void initialize_subtype();
  void apply_subtype();
  void initialize_transition();
  void apply_transition();
  void initialize_treasure();
  void apply_treasure();
  void initialize_type();
  void apply_type();
  void initialize_xy();
  void apply_xy();

  void remove_field(QWidget* label, QWidget* field);

  Ui::EditEntityDialog ui;             /**< The widgets. */
  EntityModel& entity_before;          /**< The entity to edit (remains unchanged). */
  EntityModelPtr entity_after;         /**< A copy of the entity with the modified data. */

};

#endif
