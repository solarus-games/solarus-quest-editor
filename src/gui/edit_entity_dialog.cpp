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
#include "include/gui/edit_entity_dialog.h"
#include "map_model.h"

/**
 * @brief Creates an edit entity dialog.
 * @param entity_before The entity to edit. It may or may not already be on the map.
 * @param parent The parent widget or nullptr.
 */
EditEntityDialog::EditEntityDialog(EntityModel& entity_before, QWidget* parent) :
  QDialog(parent),
  entity_before(entity_before) {

  ui.setupUi(this);

  ui.sprite_field->set_quest(get_quest());
  ui.sprite_field->set_resource_type(ResourceType::SPRITE);
  ui.treasure_name_field->set_quest(get_quest());
  ui.treasure_name_field->set_resource_type(ResourceType::ITEM);
  ui.sound_field->set_quest(get_quest());
  ui.sound_field->set_resource_type(ResourceType::SOUND);
  ui.destination_map_field->set_quest(get_quest());
  ui.destination_map_field->set_resource_type(ResourceType::MAP);

  initialize_values();
}

/**
 * @brief Returns the quest the map belongs to.
 * @return The quest.
 */
const Quest& EditEntityDialog::get_quest() const {
  return get_map().get_quest();
}

/**
 * @brief Returns the map the entity edited belongs to.
 * @return The map.
 */
MapModel& EditEntityDialog::get_map() const {
  return get_entity_before().get_map();
}

/**
 * @brief Returns the index of the entity being edited.
 * @return The entity index on the map before any change.
 */
EntityIndex EditEntityDialog::get_entity_index() const {
  return entity_before.get_index();
}

/**
 * @brief Returns the entity being edited, before any change.
 * @return The entity edited.
 */
EntityModel& EditEntityDialog::get_entity_before() const {
  return entity_before;
}

/**
 * @brief Creates and returns an entity representing the new input data.
 *
 * The created entity is not on the map.
 *
 * @return The new data.
 */
EntityModelPtr EditEntityDialog::get_entity_after() const {

  EntityModelPtr entity_after = EntityModel::create(get_map(), entity_before.get_type());
  entity_after->set_name(ui.name_field->text());
  entity_after->set_layer(ui.layer_field->get_selected_value());
  entity_after->set_xy(QPoint(ui.x_field->value(), ui.y_field->value()));

  if (entity_after->has_size_fields()) {
    entity_after->set_size(QSize(ui.width_field->value(), ui.height_field->value()));
  }

  if (entity_after->has_direction_field()) {
    entity_after->set_direction(ui.direction_field->value());
  }

  if (entity_after->has_field("sprite")) {
    entity_after->set_field("sprite", ui.sprite_field->get_selected_id());
  }

  if (entity_after->has_field("transition")) {
    TransitionStyle transition = ui.transition_field->get_selected_value();
    entity_after->set_field("transition", TransitionTraits::get_lua_name(transition));
  }

  return entity_after;
}

/**
 * @brief Fills the fields from the existing entity.
 */
void EditEntityDialog::initialize_values() {

  initialize_transition();
}

/**
 * @brief Initialize the transition field.
 */
void EditEntityDialog::initialize_transition() {

  if (entity_before.has_field("transition")) {
    QString transition_name = entity_before.get_field("transition").toString();
    ui.transition_field->set_selected_value(TransitionTraits::get_by_lua_name(transition_name));
  }
  else {
    delete ui.transition_label;
    ui.transition_label = nullptr;
    delete ui.transition_field;
    ui.transition_field = nullptr;
  }
}
