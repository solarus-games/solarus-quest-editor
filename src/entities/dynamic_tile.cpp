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
#include "entities/dynamic_tile.h"
#include "map_model.h"

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
DynamicTile::DynamicTile(MapModel& map, const EntityIndex& index) :
  Tile(map, index, EntityType::DYNAMIC_TILE) {

  set_resizable(true);

}

/**
 * @brief Creates a dynamic tile from a normal one.
 * @param map The map.
 * @param tile_index index of the normal tile to clone.
 * @return The dynamic tile created. It is not on the map yet.
 */
EntityModelPtr DynamicTile::create_from_normal_tile(MapModel& map, const EntityIndex& tile_index) {

  EntityModelPtr dynamic_tile = EntityModel::create(map, EntityType::DYNAMIC_TILE);
  dynamic_tile->set_field("pattern", map.get_entity_field(tile_index, "pattern"));
  dynamic_tile->set_xy(map.get_entity_xy(tile_index));
  dynamic_tile->set_size(map.get_entity_size(tile_index));
  return dynamic_tile;
}
