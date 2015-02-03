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
#include "entity_model.h"
#include "map_model.h"

/**
 * @brief Creates an entity model.
 * @param map The map containing the entity.
 * @param index Index of the tile pattern to represent.
 */
EntityModel::EntityModel(MapModel& map, const EntityIndex& index) :
  map(&map),
  index(index) {
}

/**
 * @brief Destructor.
 */
EntityModel::~EntityModel() {
}

/**
 * @brief Returns the map this entity belongs to.
 * @return The map.
 */
const MapModel& EntityModel::get_map() const {
  return *map;
}

/**
 * @brief Returns the map this entity belongs to.
 *
 * Non-const version.
 *
 * @return The map.
 */
MapModel& EntityModel::get_map() {
  return *map;
}

/**
 * @brief Returns the image representing the entity in the editor.
 * @return The entity's image.
 */
const QPixmap& EntityModel::get_image() const {

  if (image.isNull()) {
    // Lazily create the image.
    QString type_name = get_map().get_entity_type_name(index);
    image = QPixmap(QString(":/images/entity_%1.png").arg(type_name)).scaledToHeight(16);  // TODO
  }

  return image;
}
