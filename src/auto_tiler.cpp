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
#include "auto_tiler.h"
#include "tileset_model.h"
#include <QDebug>

namespace SolarusEditor {

/**
 * @brief Creates an autotiler.
 * @param map The map.
 * @param entity_indexes Indexes of entities where to create a border.
 */
AutoTiler::AutoTiler(MapModel& map, const EntityIndexes& entity_indexes) :
  map(map),
  entity_indexes(entity_indexes) {

  for (const EntityIndex& index : entity_indexes) {
    entity_rectangles.append(map.get_entity_bounding_box(index));
  }
}

/**
 * @brief Returns the number of cells in the 8x8 grid.
 * @return The number of cells.
 */
int AutoTiler::get_num_cells() const {

  return grid_size.width() * grid_size.height();
}

/**
 * @brief Converts map coordinates to an index in the bounding box 8x8 grid.
 * @param x X coordinate on the map.
 * @param y Y coordinate on the map.
 * @return The corresponding grid index.
 */
int AutoTiler::to_grid_index(int x, int y) const {

  x -= bounding_box.x();
  y -= bounding_box.y();
  return (y / 8) * grid_size.width() + (x / 8);
}

/**
 * @brief Returns whether a cell of the 8x8 grid is occupied by an entity.
 * @param grid_index An index in the 8x8 grid.
 * @return @c true if this cell is occupied.
 */
bool AutoTiler::is_cell_occupied(int grid_index) const {

  Q_ASSERT(grid_index >= 0);
  Q_ASSERT(grid_index < get_num_cells());

  return occupied_squares[grid_index];
}

/**
 * @brief Returns a bit field indicating the occupied state of 4 cells.
 * @param cell_0 the top-left cell of the 4 cells.
 * @return The occupied state of the 4 cells.
 */
int AutoTiler::get_4_cells_mask(int cell_0) const {

  int cell_1 = cell_0 + 1;
  int cell_2 = cell_0 + grid_size.width();
  int cell_3 = cell_2 + 1;

  int bit_0 = is_cell_occupied(cell_0) ? 1 : 0;
  int bit_1 = is_cell_occupied(cell_1) ? 1 : 0;
  int bit_2 = is_cell_occupied(cell_2) ? 1 : 0;
  int bit_3 = is_cell_occupied(cell_3) ? 1 : 0;

  qDebug() << is_cell_occupied(cell_0) << is_cell_occupied(cell_1) << is_cell_occupied(cell_2) << is_cell_occupied(cell_3);

  return bit_3 | (bit_2 << 1) | (bit_1 << 2) | (bit_0 << 3);
}

/**
 * @brief Determines the bounding box of the entities and extends it of 8 pixels.
 */
void AutoTiler::compute_bounding_box() {

  bounding_box = QRect();
  for (const QRect& rectangle : entity_rectangles) {
    bounding_box |= rectangle;
  }

  // Add a margin of 8 pixels.
  bounding_box.translate(-8, -8);
  bounding_box.setSize(bounding_box.size() + QSize(16, 16));

  grid_size = bounding_box.size() / 8;
}

/**
 * @brief Determines the 8x8 squares that are overlapped by entities.
 */
void AutoTiler::compute_occupied_squares() {

  occupied_squares.clear();
  occupied_squares.assign(get_num_cells(), false);

  for (const QRect& rectangle : entity_rectangles) {

    for (int y = rectangle.y(); y < rectangle.y() + rectangle.height(); y += 8) {
      for (int x = rectangle.x(); x < rectangle.x() + rectangle.width(); x += 8) {
        int grid_index = to_grid_index(x, y);
        occupied_squares[grid_index] = true;
      }
    }
  }
}

/**
 * @brief Determines the border tiles to create.
 */
void AutoTiler::compute_borders() {

  QMap<int, BorderInfo> borders_by_square;

  for (const QRect& rectangle : entity_rectangles) {

    int num_cells_x = rectangle.width() / 8;
    int num_cells_y = rectangle.height() / 8;
    int rectangle_top_left_cell = to_grid_index(rectangle.x(), rectangle.y());
    int initial_position = rectangle_top_left_cell - 1 - grid_size.width();  // 1 cell above and to the left.

    qDebug() << "Rect: " << rectangle << ", num cells " << num_cells_x << num_cells_y;
    for (int i = 0; i < num_cells_x + 1; ++i) {

      int mask = get_4_cells_mask(initial_position + i);

      qDebug() << " -> " << mask;
    }
  }
}

/**
 * @brief Creates border tiles around the given entities.
 * @return The border tiles ready to be added to the map.  1 1
 */
AddableEntities AutoTiler::generate_border_tiles() {

  // Test floor borders.
  QStringList border_pattern_ids = {
    "wall_border.1-4",  // Up.
    "wall_border.3-4",  // Left.
    "wall_border.2-4",  // Down.
    "wall_border.4-4",  // Right.
    "wall_border.corner.2-4",  // Up-left convex corner.
    "wall_border.corner.1-4",
    "wall_border.corner.3-4",
    "wall_border.corner.4-4",
    "wall_border.corner_reverse.2-4",
    "wall_border.corner_reverse.1-4",
    "wall_border.corner_reverse.3-4",
    "wall_border.corner_reverse.4-4",
  };

  /*
  // Test walls.
  QStringList border_pattern_ids = {
    "wall.1-2",  // Up.
    "wall.3-2",  // Left.
    "wall.2-2",  // Down.
    "wall.4-2",  // Right.
    "wall.corner.2-2",  // Up-left convex corner.
    "wall.corner.1-2",
    "wall.corner.3-2",
    "wall.corner.4-2",
    "wall.corner_reverse.2-2",
    "wall.corner_reverse.1-2",
    "wall.corner_reverse.3-2",
    "wall.corner_reverse.4-2",
  };
  */

  if (entity_rectangles.empty()) {
    return AddableEntities();
  }

  // Determine the 8x8 grid.
  compute_bounding_box();
  qDebug() << bounding_box;
  qDebug() << "Grid: " << grid_size << ", cells: " << get_num_cells();

  // Create a list indicating which 8x8 squares are inside the selection.
  compute_occupied_squares();
  for (size_t i = 0; i < occupied_squares.size(); ++i) {
    if (occupied_squares[i]) {
      qDebug() << " " << i;
    }
  }

  // Detects the borders to make
  compute_borders();

//  const TilesetModel& tileset = *map.get_tileset_model();

  const EntityIndex& first_entity_index = entity_indexes.first();
  int layer = first_entity_index.layer;

  EntityModels tiles;

  /*
  QString up_pattern_id = border_pattern_ids[0];
  QRect up_pattern_size = tileset.get_pattern_frame(tileset.id_to_index(up_pattern_id));
  QString left_pattern_id = border_pattern_ids[1];
  QRect left_pattern_size = tileset.get_pattern_frame(tileset.id_to_index(left_pattern_id));
  QString down_pattern_id = border_pattern_ids[2];
  QRect down_pattern_size = tileset.get_pattern_frame(tileset.id_to_index(down_pattern_id));
  QString right_pattern_id = border_pattern_ids[3];
  QRect right_pattern_size = tileset.get_pattern_frame(tileset.id_to_index(right_pattern_id));

  QString up_right_pattern_id = border_pattern_ids[4];
  QSize up_right_pattern_size = tileset.get_pattern_frame(tileset.id_to_index(up_right_pattern_id)).size();
  QString up_left_pattern_id = border_pattern_ids[5];
  QSize up_left_pattern_size = tileset.get_pattern_frame(tileset.id_to_index(up_left_pattern_id)).size();
  QString down_left_pattern_id = border_pattern_ids[6];
  QSize down_left_pattern_size = tileset.get_pattern_frame(tileset.id_to_index(down_left_pattern_id)).size();
  QString down_right_pattern_id = border_pattern_ids[7];
  QSize down_right_pattern_size = tileset.get_pattern_frame(tileset.id_to_index(down_right_pattern_id)).size();
*/

  /*
  EntityModelPtr tile;

  // Up.
  tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", up_pattern_id);
  tile->set_xy(QPoint(box.left() + left_pattern_size.width(), box.top()));
  tile->set_size(QSize(box.width() - 2 * left_pattern_size.width(), up_pattern_size.height()));
  tile->set_layer(layer);
  tiles.emplace_back(std::move(tile));

  // Left.
  tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", left_pattern_id);
  tile->set_xy(QPoint(box.left(), box.top() + up_pattern_size.height()));
  tile->set_size(QSize(left_pattern_size.width(), box.height() - 2 * up_pattern_size.height()));
  tile->set_layer(layer);
  tiles.emplace_back(std::move(tile));

  // Down.
  tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", down_pattern_id);
  tile->set_xy(QPoint(box.left() + left_pattern_size.width(), box.top() + box.height() - down_pattern_size.height()));
  tile->set_size(QSize(box.width() - 2 * left_pattern_size.width(), down_pattern_size.height()));
  tile->set_layer(layer);
  tiles.emplace_back(std::move(tile));

  // Right.
  tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", right_pattern_id);
  tile->set_xy(QPoint(box.left() + box.width() - right_pattern_size.width(), box.top() + up_pattern_size.height()));
  tile->set_size(QSize(right_pattern_size.width(), box.height() - 2 * up_pattern_size.height()));
  tile->set_layer(layer);
  tiles.emplace_back(std::move(tile));

  // Up-right corner.
  tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", up_right_pattern_id);
  tile->set_xy(QPoint(box.left() + box.width() - up_right_pattern_size.width(), box.top()));
  tile->set_size(up_right_pattern_size);
  tile->set_layer(layer);
  tiles.emplace_back(std::move(tile));

  // Up-left corner.
  tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", up_left_pattern_id);
  tile->set_xy(QPoint(box.left(), box.top()));
  tile->set_size(up_left_pattern_size);
  tile->set_layer(layer);
  tiles.emplace_back(std::move(tile));

  // Down-left corner.
  tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", down_left_pattern_id);
  tile->set_xy(QPoint(box.left(), box.top() + box.height() - down_left_pattern_size.height()));
  tile->set_size(down_left_pattern_size);
  tile->set_layer(layer);
  tiles.emplace_back(std::move(tile));

  // Down-right corner.
  tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", down_right_pattern_id);
  tile->set_xy(QPoint(
      box.left() + box.width() - down_right_pattern_size.width(),
      box.top() + box.height() - down_right_pattern_size.height()));
  tile->set_size(down_right_pattern_size);
  tile->set_layer(layer);
  tiles.emplace_back(std::move(tile));
  */

  int order = map.get_num_tiles(layer);
  AddableEntities addable_tiles;
  for (EntityModelPtr& tile : tiles) {
    EntityIndex index = { layer, order };
    addable_tiles.emplace_back(std::move(tile), index);
    ++order;
  }

  return addable_tiles;
}

}
