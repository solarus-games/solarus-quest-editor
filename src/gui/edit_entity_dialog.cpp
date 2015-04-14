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
 * @param map The map of the entity to edit.
 * @param index Index of the entity to edit on the map.
 * @param parent The parent widget or nullptr.
 */
EditEntityDialog::EditEntityDialog(MapModel& map, const EntityIndex& index, QWidget* parent) :
  QDialog(parent),
  map(map),
  index(index) {

  ui.setupUi(this);

  ui.sprite_field->set_quest(get_quest());
  ui.sprite_field->set_resource_type(ResourceType::SPRITE);
  ui.treasure_name_field->set_quest(get_quest());
  ui.treasure_name_field->set_resource_type(ResourceType::ITEM);
}

/**
 * @brief Returns the quest the map belongs to.
 * @return The quest.
 */
Quest& EditEntityDialog::get_quest() const {
  return get_map().get_quest();
}

/**
 * @brief Returns the map the entity edited belongs to.
 * @return The map.
 */
MapModel& EditEntityDialog::get_map() const {
  return map;
}

/**
 * @brief Returns the index of the entity being edited.
 * @return The entity index on the map.
 */
EntityIndex EditEntityDialog::get_entity_index() const {
  return index;
}

/**
 * @brief Creates and returns an entity representing the new input data.
 *
 * The created entity is not on the map.
 *
 * @return The new data.
 */
EntityModelPtr EditEntityDialog::get_entity_after() const {

  // TODO
  return EntityModelPtr();
}
