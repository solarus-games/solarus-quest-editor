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
#include <iostream>  // TODO remove
#include <iomanip>

namespace SolarusEditor {

namespace {

// Test floor borders.
QStringList border_pattern_ids = {
  "wall_border.4-4",  // Right.
  "wall_border.1-4",  // Up.
  "wall_border.3-4",  // Left.
  "wall_border.2-4",  // Down.
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
  "wall.4-2",  // Right.
  "wall.1-2",  // Up.
  "wall.3-2",  // Left.
  "wall.2-2",  // Down.
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

}

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
 * @param xy Coordinates on the map.
 * @return The corresponding grid index.
 */
int AutoTiler::to_grid_index(const QPoint& xy) const {

  int x = xy.x() - bounding_box.x();
  int y = xy.y() - bounding_box.y();
  return (y / 8) * grid_size.width() + (x / 8);
}

/**
 * @brief Converts an index in the bounding box 8x8 grid to map coordinates.
 * @param grid_index A grid index.
 * @return The corresponding map coordinates.
 */
QPoint AutoTiler::to_map_xy(int grid_index) const {

  int grid_x = grid_index % grid_size.width();
  int grid_y = grid_index / grid_size.width();
  return QPoint(grid_x * 8 + bounding_box.x(), grid_y * 8 + bounding_box.y());
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
int AutoTiler::get_four_cells_mask(int cell_0) const {

  int cell_1 = cell_0 + 1;
  int cell_2 = cell_0 + grid_size.width();
  int cell_3 = cell_2 + 1;

  int bit_0 = is_cell_occupied(cell_0) ? 1 : 0;
  int bit_1 = is_cell_occupied(cell_1) ? 1 : 0;
  int bit_2 = is_cell_occupied(cell_2) ? 1 : 0;
  int bit_3 = is_cell_occupied(cell_3) ? 1 : 0;

  return bit_3 | (bit_2 << 1) | (bit_1 << 2) | (bit_0 << 3);
}

/**
 * @brief Returns the which border side or corner to create
 * given a mask of 4 cells in the 8x8 grid.
 * @param four_cells_mask A 4 cells mask representing occupied cells.
 * @return The corresponding kind of border.
 *
 * 0: Right side.
 * 1: Top side.
 * 2: Left side.
 * 3: Bottom side.
 *
 * 4: Top-right convex corner.
 * 5: Top-left convex corner.
 * 6: Bottom-left convex corner.
 * 7: Bottom-right convex corner.
 *
 * 8: Top-right concave corner.
 * 9: Top-left concave corner.
 * 10: Bottom-left concave corner.
 * 11: Bottom-right concave corner.
 */
AutoTiler::WhichBorder AutoTiler::get_which_border_from_mask(int four_cells_mask) const {

  switch (four_cells_mask) {

  // 0 0
  // 0 0
  case 0:
  return WhichBorder::NONE;

  // 0 0
  // 0 1
  case 1:
  return WhichBorder::TOP_LEFT_CONVEX;

  // 0 0
  // 1 0
  case 2:
  return WhichBorder::TOP_RIGHT_CONVEX;

  // 0 0
  // 1 1
  case 3:
  return WhichBorder::TOP;

  // 0 1
  // 0 0
  case 4:
  return WhichBorder::BOTTOM_LEFT_CONVEX;

  // 0 1
  // 0 1
  case 5:
  return WhichBorder::LEFT;

  // 0 1
  // 1 0
  case 6:
  return WhichBorder::TOP_RIGHT_CONVEX;  // TODO

  // 0 1
  // 1 1
  case 7:
  return WhichBorder::TOP_LEFT_CONCAVE;

  // 1 0
  // 0 0
  case 8:
  return WhichBorder::BOTTOM_RIGHT_CONVEX;

  // 1 0
  // 0 1
  case 9:
  return WhichBorder::BOTTOM_RIGHT_CONVEX;  // TODO

  // 1 0
  // 1 0
  case 10:
  return WhichBorder::RIGHT;

  // 1 0
  // 1 1
  case 11:
  return WhichBorder::TOP_RIGHT_CONCAVE;

  // 1 1
  // 0 0
  case 12:
  return WhichBorder::BOTTOM;

  // 1 1
  // 0 1
  case 13:
  return WhichBorder::BOTTOM_LEFT_CONCAVE;

  // 1 1
  // 1 0
  case 14:
  return WhichBorder::BOTTOM_RIGHT_CONCAVE;

  // 1 1
  // 1 1
  case 15:
  return WhichBorder::NONE;

  }

  return WhichBorder::NONE;
}

/**
 * @brief Returns whether a border type is a side or a corner.
 * @param which_border A border type.
 * @return @c true if this is a side.
 */
bool AutoTiler::is_side_border(WhichBorder which_border) const {

  return which_border == WhichBorder::RIGHT ||
      which_border == WhichBorder::TOP ||
      which_border == WhichBorder::LEFT||
      which_border == WhichBorder::BOTTOM;
}

/**
 * @brief Returns whether a square of the 8x8 grid is marked with a border value.
 * @param grid_index An index in the 8x8 grid.
 * @return @c true if there is a border.
 */
bool AutoTiler::has_border(int grid_index) const {

  return get_which_border(grid_index) != WhichBorder::NONE;
}

/**
 * @brief Returns the kind of border in a cell of the 8x8 grid.
 * @param grid_index An index in the 8x8 grid.
 * @return The kind of border in this cell (-1 means none).
 */
AutoTiler::WhichBorder AutoTiler::get_which_border(int grid_index) const {

  Q_ASSERT(grid_index >= 0);
  Q_ASSERT(grid_index < get_num_cells());

  const auto& it = which_borders.find(grid_index);
  if (it == which_borders.end()) {
    return WhichBorder::NONE;
  }
  return it->second;
}

/**
 * @brief Sets the kind of border in a cell of the 8x8 grid.
 * @param grid_index An index in the 8x8 grid.
 * @param which_border The kind of border in this cell.
 */
void AutoTiler::set_which_border(int grid_index, WhichBorder which_border) {

  Q_ASSERT(grid_index >= 0);
  Q_ASSERT(grid_index < get_num_cells());

  which_borders[grid_index] = which_border;
}

/**
 * @brief Marks squares of the 8x8 grid with their border info.
 * @param cell_0 Index of the top-left cell of a four square mask in the 8x8 grid.
 *
 * When there is already a border value in a cell, corners are prioritary.
 */
void AutoTiler::detect_border_info(int cell_0) {

  int cell_1 = cell_0 + 1;
  int cell_2 = cell_0 + grid_size.width();
  int cell_3 = cell_2 + 1;

  int mask = get_four_cells_mask(cell_0);
  WhichBorder which_border = get_which_border_from_mask(mask); // TODO remove this function

  if (which_border == WhichBorder::NONE) {
    // No border here.
    return;
  }

  switch (which_border) {

  case WhichBorder::RIGHT:
    if (!has_border(cell_0)) {
      set_which_border(cell_0, which_border);
    }
    if (!has_border(cell_2)) {
      set_which_border(cell_2, which_border);
    }
    break;

  case WhichBorder::TOP:
    if (!has_border(cell_2)) {
      set_which_border(cell_2, which_border);
    }
    if (!has_border(cell_3)) {
      set_which_border(cell_3, which_border);
    }
    break;

  case WhichBorder::LEFT:
    if (!has_border(cell_1)) {
      set_which_border(cell_1, which_border);
    }
    if (!has_border(cell_3)) {
      set_which_border(cell_3, which_border);
    }
    break;

  case WhichBorder::BOTTOM:
    if (!has_border(cell_0)) {
      set_which_border(cell_0, which_border);
    }
    if (!has_border(cell_1)) {
      set_which_border(cell_1, which_border);
    }
    break;

  case WhichBorder::TOP_RIGHT_CONVEX:
  case WhichBorder::TOP_RIGHT_CONCAVE:
    set_which_border(cell_2, which_border);
    break;

  case WhichBorder::TOP_LEFT_CONVEX:
  case WhichBorder::TOP_LEFT_CONCAVE:
    set_which_border(cell_3, which_border);
    break;

  case WhichBorder::BOTTOM_LEFT_CONVEX:
  case WhichBorder::BOTTOM_LEFT_CONCAVE:
    set_which_border(cell_1, which_border);
    break;

  case WhichBorder::BOTTOM_RIGHT_CONVEX:
  case WhichBorder::BOTTOM_RIGHT_CONCAVE:
    set_which_border(cell_0, which_border);
    break;

  }

}

/**
 * @brief Creates a tile with the given position in the 8x8 grid.
 * @param which_border Kind of border to create.
 * @param grid_index Index in the 8x8 grid of the first cell occupied by the tile.
 * @param num_cells_repeat On how many cells of the 8x8 grid the pattern should be repeated
 * (ignored for corners).
 */
void AutoTiler::make_tile(WhichBorder which_border, int grid_index, int num_cells_repeat) {

  if (which_border == WhichBorder::NONE) {
    return;
  }
  Q_ASSERT(num_cells_repeat > 0);

  QPoint xy = to_map_xy(grid_index);
  QSize size;
  const QString& pattern_id = border_pattern_ids[static_cast<int>(which_border)];

  const TilesetModel& tileset = *map.get_tileset_model();
  const QSize& pattern_size = tileset.get_pattern_frame(tileset.id_to_index(pattern_id)).size();
  Q_ASSERT(!pattern_size.isEmpty());

  int size_repeated = num_cells_repeat * 8;

  switch (which_border) {

  case WhichBorder::RIGHT:  // Right side.
  case WhichBorder::LEFT:  // Left side.
    size = { pattern_size.width(), size_repeated };
    break;

  case WhichBorder::TOP:  // Top side.
  case WhichBorder::BOTTOM:  // Bottom side.
    size = { size_repeated, pattern_size.height() };
    break;

  default:
    // Corner.
    size = pattern_size;
    break;
  }

  Q_ASSERT(!size.isEmpty());

  const EntityIndex& first_entity_index = entity_indexes.first();
  int layer = first_entity_index.layer;  // TODO choose the lowest layer.

  EntityModelPtr tile = EntityModel::create(map, EntityType::TILE);
  tile->set_field("pattern", pattern_id);
  tile->set_xy(xy);
  tile->set_size(size);
  tile->set_layer(layer);

  tiles.emplace_back(std::move(tile));
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
        int grid_index = to_grid_index(QPoint(x, y));
        occupied_squares[grid_index] = true;
      }
    }
  }
}

/**
 * @brief Detect the borders.
 */
void AutoTiler::compute_borders() {

  which_borders.clear();

  for (const QRect& rectangle : entity_rectangles) {

    int num_cells_x = rectangle.width() / 8;
    int num_cells_y = rectangle.height() / 8;

    // Top side.
    int rectangle_top_left_cell = to_grid_index(rectangle.topLeft());
    int initial_position = rectangle_top_left_cell - 1 - grid_size.width();  // 1 cell above and to the left.
    int cell_0 = initial_position;

    for (int i = 0; i < num_cells_x; ++i) {
      detect_border_info(cell_0);
      ++cell_0;
    }

    // Right side.
    for (int i = 0; i < num_cells_y; ++i) {
      detect_border_info(cell_0);
      cell_0 += grid_size.width();
    }

    // Bottom side.
    for (int i = 0; i < num_cells_x; ++i) {
      detect_border_info(cell_0);
      --cell_0;
    }

    // Left side.
    for (int i = 0; i < num_cells_y; ++i) {
      detect_border_info(cell_0);
      cell_0 -= grid_size.width();
    }

  }
}

/**
 * @brief Outputs the grid of border types for debugging.
 */
void AutoTiler::print_which_borders() const {

  int index = 0;
  for (int i = 0; i < grid_size.height(); ++i) {
    for (int j = 0; j < grid_size.width(); ++j) {
      WhichBorder which_border = get_which_border(index);
      if (which_border != WhichBorder::NONE) {
        std::cout << std::setw(2) << static_cast<int>(which_border) << " ";
      }
      else {
        std::cout << "   ";
      }
      ++index;
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

/**
 * @brief Creates the border tiles from the border info previously detected.
 */
void AutoTiler::compute_tiles() {

  // Generate sides first.
  for (const auto& it : which_borders) {

    int start_index = it.first;
    WhichBorder which_border = it.second;

    int grid_x = start_index % grid_size.width();
    int grid_y = start_index / grid_size.width();
    int num_cells_repeat = 1;
    int current_index = start_index;

    if (which_border == WhichBorder::NONE) {
      continue;
    }

    if (!is_side_border(which_border)) {
      continue;
    }

    set_which_border(start_index, WhichBorder::NONE);  // Mark visited.

    switch (which_border) {

    case WhichBorder::RIGHT:
    case WhichBorder::LEFT:
      // Left or right border.
      for (int i = grid_y + 1; i < grid_size.height(); ++i) {
        current_index += grid_size.width();
        if (get_which_border(current_index) != which_border) {
          break;
        }
        ++num_cells_repeat;
        set_which_border(current_index, WhichBorder::NONE);
      }
      make_tile(which_border, start_index, num_cells_repeat);
      break;

    case WhichBorder::TOP:
    case WhichBorder::BOTTOM:
      // Top or bottom border.
      for (int j = grid_x + 1; j < grid_size.width(); ++j) {
        ++current_index;
        if (get_which_border(current_index) != which_border) {
          break;
        }
        ++num_cells_repeat;
        set_which_border(current_index, WhichBorder::NONE);
      }
      make_tile(which_border, start_index, num_cells_repeat);
      break;
    }
  }

  // Generate corners.
  for (const auto& it : which_borders) {
    int start_index = it.first;
    WhichBorder which_border = it.second;
    make_tile(which_border, start_index, 1);
  }

  which_borders.clear();
}

/**
 * @brief Creates border tiles around the given entities.
 * @return The border tiles ready to be added to the map.  1 1
 */
AddableEntities AutoTiler::generate_border_tiles() {

  if (entity_rectangles.empty()) {
    return AddableEntities();
  }

  // Determine the 8x8 grid.
  compute_bounding_box();
  qDebug() << bounding_box;
  qDebug() << "Grid: " << grid_size << ", cells: " << get_num_cells();

  // Create a list indicating which 8x8 squares are inside the selection.
  compute_occupied_squares();

  // Detect the borders.
  compute_borders();
  qDebug() << "Detected borders";

  // Create the corresponding tiles.
  compute_tiles();
  qDebug() << "Created " << tiles.size() << " tiles";
  if (tiles.empty()) {
    return AddableEntities();
  }

  const EntityModelPtr& first_tile = *tiles.begin();
  int layer = first_tile->get_layer();
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
