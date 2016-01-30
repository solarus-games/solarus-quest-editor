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
#include "widgets/entity_selector.h"
#include "editor_exception.h"
#include "map_model.h"
#include "quest.h"

/**
 * @brief Creates an empty entity selector.
 *
 * Call the setter functions and then build to fill the selector.
 *
 * @param parent The parent widget or nullptr.
 */
EntitySelector::EntitySelector(QWidget* parent) :
  QComboBox(parent),
  quest(nullptr),
  map_id(),
  special_values(),
  filtered_by_entity_type(false),
  entity_type_filter(EntityType()) {

}

/**
 * @brief Returns the id of the map where entities come from.
 * @return The map id or an empty string if it is not set yet.
 */
const QString& EntitySelector::get_map_id() const {
  return map_id;
}

/**
 * @brief Sets the map where entities should come from.
 * @param quest The quest the map belongs to.
 * @param map_id Id of the map.
 */
void EntitySelector::set_map_id(Quest& quest, const QString& map_id) {

  this->quest = &quest;
  this->map_id = map_id;
}

/**
 * @brief Returns whether only one type of entity is shown.
 * @return @c true if entities are filtered by type.
 */
bool EntitySelector::is_filtered_by_entity_type() const {
  return filtered_by_entity_type;
}

/**
 * @brief Sets whether only one type of entity is shown.
 *
 * Use set_entity_type_filter to indicate which one.
 *
 * @param filtered @c true to filter entities by type.
 */
void EntitySelector::set_filtered_by_entity_type(bool filtered) {
  this->filtered_by_entity_type = filtered;
}

/**
 * @brief Returns the type used to filter entities if any.
 * @return The type shown.
 */
EntityType EntitySelector::get_entity_type_filter() const {
  return entity_type_filter;
}

/**
 * @brief Sets the type used to filter entities.
 *
 * This setting only has an effect if is_filtered_by_entity_type() is @c true.
 *
 * @param type The type to show.
 */
void EntitySelector::set_entity_type_filter(EntityType type) {
  this->entity_type_filter = type;
}

/**
 * @brief Adds a non-entity item to the beginning of the combobox.
 * @param name Internal name to set instead of the name of an entity.
 * @param text Text to display to the user.
 */
void EntitySelector::add_special_value(const QString& name, const QString& text) {
  special_values.append(qMakePair(name, text));
}

/**
 * @brief Returns the name in the selected item.
 *
 * It can be the name of an entity of the name of a special value item.
 *
 * @return The selected name.
 */
QString EntitySelector::get_selected_name() const {

  return currentData().toString();
}

/**
 * @brief Selects the specified item.
 * @return The name to make selected.
 * It can be the name of an entity of the name of a special value item.
 */
void EntitySelector::set_selected_name(const QString& name) {

  int index = findData(name, Qt::UserRole);
  if (index == -1) {
    return;
  }

  setCurrentIndex(index);
}

/**
 * @brief Builds or rebuilds the combobox using the parameters previously set.
 */
void EntitySelector::build() {

  clear();

  if (quest == nullptr || map_id.isEmpty()) {
    return;
  }

  try {
    MapModel map(*quest, map_id);

    // Add special value items first.
    for (const SpecialValue& special_value : special_values) {
      addItem(special_value.second, special_value.first);
    }

    // Add entities.
    const QMap<QString, EntityIndex>& indexes = map.get_named_entities();
    for (auto it = indexes.begin(); it != indexes.end(); ++it) {
      const QString& name = it.key();
      const EntityIndex& index = it.value();
      if (is_filtered_by_entity_type() &&
          map.get_entity_type(index) != get_entity_type_filter()) {
        // Not the wanted entity type.
        continue;
      }
      addItem(name, name);
    }
  }
  catch (const EditorException& ex) {
    // The map file could not be opened: the map id is probably unset or incorrect.
    Q_UNUSED(ex);
  }
}
