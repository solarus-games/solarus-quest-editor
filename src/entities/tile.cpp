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
#include "entities/tile.h"
#include "map_model.h"
#include "tileset_model.h"
#include <QPainter>

/**
 * @brief Creates a normal tile.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
Tile::Tile(MapModel& map, const EntityIndex& index) :
  Tile(map, index, EntityType::TILE) {

}

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 * @param type Concrete type of entity: TILE or DYNAMIC_TILE.
 */
Tile::Tile(MapModel& map, const EntityIndex& index, EntityType type) :
  EntityModel(map, index, type) {

}

/**
 * @brief Returns the pattern id used by this tile.
 * @return The pattern id.
 */
QString Tile::get_pattern_id() const {
  return get_field("pattern").toString();
}

/**
 * @brief Sets the pattern id used by this tile.
 * @param pattern_id The new pattern id.
 */
void Tile::set_pattern_id(const QString& pattern_id) {
  set_field("pattern", pattern_id);
}

/**
 * @copydoc EntityModel::draw
 */
void Tile::draw(QPainter& painter) const {

  if (pattern_image.isNull()) {
    // Lazily create the image.
    const TilesetModel* tileset = get_tileset();
    if (tileset != nullptr) {
      int pattern_index = tileset->id_to_index(get_pattern_id());
      if (pattern_index == -1) {
        // The pattern no longer exists: fallback to a generic tile icon.
        EntityModel::draw(painter);
        return;
      }
      int pattern_width = tileset->get_pattern_frame(pattern_index).width();
      pattern_image = tileset->get_pattern_image(pattern_index).scaledToWidth(pattern_width);
    }
  }

  painter.drawTiledPixmap(0, 0, get_width(), get_height(), pattern_image);
}
