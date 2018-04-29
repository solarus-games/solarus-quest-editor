/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#include "ground_traits.h"
#include "map_model.h"
#include "quest.h"
#include "tileset_model.h"
#include <QPainter>

namespace SolarusEditor {

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

  set_resizable(true);
  set_has_preferred_layer(true);
}

/**
 * @brief Creates a normal tile from a dynamic one.
 * @param map The map.
 * @param tile_index index of the dynamic tile to clone.
 * @return The tile created. It is not on the map yet.
 */
EntityModelPtr Tile::create_from_dynamic_tile(MapModel& map, const EntityIndex& dynamic_tile_index) {

  Q_ASSERT(map.get_entity_type(dynamic_tile_index) == EntityType::DYNAMIC_TILE);

  EntityModelPtr tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", map.get_entity_field(dynamic_tile_index, "pattern"));
  tile->set_xy(map.get_entity_xy(dynamic_tile_index));
  tile->set_size(map.get_entity_size(dynamic_tile_index));
  return tile;
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
 * @brief Returns the tileset used by this tile.
 * @return The tileset.
 */
const TilesetModel* Tile::get_tileset() const {

  QString tileset_id = get_field("tileset").toString();
  if (tileset_id.isEmpty()) {
    return get_map().get_tileset_model();
  }

  return get_quest().get_tileset(tileset_id);
}

/**
 * @copydoc EntityModel::notify_field_changed
 */
void Tile::notify_field_changed(const QString& key, const QVariant& value) {

  EntityModel::notify_field_changed(key, value);

  if (key == "pattern" || key == "tileset") {
    update_pattern();
  }
}

/**
 * @brief Updates the representation of the tile.
 *
 * This function should be called when the pattern changes.
 */
void Tile::update_pattern() {

  const TilesetModel* tileset = get_tileset();
  if (tileset != nullptr) {
    int pattern_index = tileset->id_to_index(get_pattern_id());
    if (pattern_index != -1) {
      // Update the resizing rules.
      set_base_size(tileset->get_pattern_frame(pattern_index).size());
      set_resize_mode(get_pattern_resize_mode());

      // Update the preferred initial layer.
      set_preferred_layer(tileset->get_pattern_default_layer(pattern_index));

      // Update the traversable property.
      Ground ground = tileset->get_pattern_ground(pattern_index);
      set_traversable(GroundTraits::is_traversable(ground));
    }
  }

  // Invalidate the cached image.
  pattern_image = QPixmap();
}

/**
 * @brief Computes the resize mode for this tile from its pattern.
 * @return The appropriate resize mode.
 */
ResizeMode Tile::get_pattern_resize_mode() const {

  const TilesetModel* tileset = get_tileset();
  if (tileset == nullptr) {
    return ResizeMode::MULTI_DIMENSION_ALL;
  }

  int pattern_index = tileset->id_to_index(get_pattern_id());
  if (pattern_index == -1) {
    return ResizeMode::MULTI_DIMENSION_ALL;
  }

  switch (tileset->get_pattern_repeat_mode(pattern_index)) {

  case TilePatternRepeatMode::ALL:
    return ResizeMode::MULTI_DIMENSION_ALL;

  case TilePatternRepeatMode::HORIZONTAL:
    return ResizeMode::HORIZONTAL_ONLY;

  case TilePatternRepeatMode::VERTICAL:
    return ResizeMode::VERTICAL_ONLY;

  case TilePatternRepeatMode::NONE:
    return ResizeMode::NONE;

  }

  return ResizeMode::MULTI_DIMENSION_ALL;
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

/**
 * @copydoc EntityModel::notify_tileset_changed
 */
void Tile::notify_tileset_changed(const QString& tileset_id) {

  EntityModel::notify_tileset_changed(tileset_id);
  update_pattern();
}

}
