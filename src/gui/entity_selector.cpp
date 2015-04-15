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
#include "gui/entity_selector.h"

/**
 * @brief Creates an empty entity selector.
 *
 * Call the setter functions and then build to fill the selector.
 *
 * @param parent The parent widget or nullptr.
 */
EntitySelector::EntitySelector(QWidget* parent) :
  QComboBox(parent),
  map(nullptr),
  special_values(),
  filtered_by_entity_type(false),
  entity_type_filter(EntityType()) {

}

/**
 * @brief Returns the map where entities come from.
 * @return The map or nullptr if it is not set yet.
 */
const MapModel* EntitySelector::get_map() const {
  return map;
}

/**
 * @brief Sets the map where entities should come from.
 * @param map
 */
void EntitySelector::set_map(const MapModel* map) {

  if (map == this->map) {
    return;
  }

  this->map = map;
  clear();  // The map has changed, don't keep old entiites.
}

bool EntitySelector::is_filtered_by_entity_type() const {
  return filtered_by_entity_type;
}

void EntitySelector::set_filtered_by_entity_type(bool filtered) {
  this->filtered_by_entity_type = filtered;
}

EntityType EntitySelector::get_entity_type_filter() const {
  return entity_type_filter;
}

void EntitySelector::set_entity_type_filter(EntityType type) {
  this->entity_type_filter = type;
}

void EntitySelector::add_special_value(const QString& name, const QString& text) {
  special_values.emplace_back(std::make_pair(name, text));
}

QString EntitySelector::get_selected_name() const {
  // TODO
  return QString();
}

void EntitySelector::set_selected_name(const QString& name) {
  // TODO
  Q_UNUSED(name);
}

/**
 * @brief Builds or rebuilds the combobox using the parameters previously set.
 */
void EntitySelector::build() {

  clear();

  if (map == nullptr) {
    return;
  }
}
