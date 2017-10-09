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
#include "entities/pickable.h"

namespace SolarusEditor {

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
Pickable::Pickable(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::PICKABLE) {

  set_origin(QPoint(8, 13));
}

/**
 * @copydoc EntityModel::notify_field_changed
 */
void Pickable::notify_field_changed(const QString& key, const QVariant& value) {

  EntityModel::notify_field_changed(key, value);

  if (key == "treasure_name" ||
      key == "treasure_variant") {
    update_treasure();
  }
}

/**
 * @brief Updates the representation of the pickable.
 *
 * This function should be called when the treasure changes.
 */
void Pickable::update_treasure() {

  QString treasure_name = get_field("treasure_name").toString();
  int treasure_variant = get_field("treasure_variant").toInt();
  if (!treasure_name.isEmpty()) {
    DrawSpriteInfo info;
    info.sprite_id = "entities/items";
    info.animation = treasure_name;
    info.direction = treasure_variant - 1;
    set_draw_sprite_info(info);
  }
}

}
