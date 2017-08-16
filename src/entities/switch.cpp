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
#include "entities/switch.h"
#include "map_model.h"

namespace SolarusEditor {

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
Switch::Switch(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::SWITCH) {

  SubtypeList subtypes = {
    { "walkable", MapModel::tr("Walkable") },
    { "arrow_target", MapModel::tr("Arrow target") },
    { "solid", MapModel::tr("Solid") }
  };
  set_existing_subtypes(subtypes);
}

/**
 * @copydoc EntityModel::notify_field_changed
 */
void Switch::notify_field_changed(const QString& key, const QVariant& value) {

  EntityModel::notify_field_changed(key, value);

  if (key == "subtype") {
    update_subtype();
  }
}

/**
 * @brief Updates the representation of the crystal block.
 *
 * This function should be called when the subtype changes.
 */
void Switch::update_subtype() {

  bool solid = get_subtype() == "solid";
  set_traversable(!solid);
}

}
