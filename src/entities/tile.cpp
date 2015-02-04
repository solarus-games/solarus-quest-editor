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
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param entity The entity data to represent.
 */
Tile::Tile(MapModel& map, const Solarus::EntityData& entity) :
  EntityModel(map, entity) {

  Q_ASSERT(entity.get_type() == EntityType::TILE);
}

/**
 * @brief Returns the pattern id used by this tile.
 * @return The pattern id.
 */
QString Tile::get_pattern_id() const {
  return get_field("pattern").toString();
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
      int pattern_width = tileset->get_pattern_frame(pattern_index).width();
      pattern_image = tileset->get_pattern_image(pattern_index).scaledToWidth(pattern_width * 2);
    }
  }

  painter.drawTiledPixmap(0, 0, get_width() * 2, get_height() * 2, pattern_image);
}
