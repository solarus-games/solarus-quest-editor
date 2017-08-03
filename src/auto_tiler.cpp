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
 * @return The corresponding kind of border (0 to 11) or -1
 * if there is no border to create here.
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
int AutoTiler::get_which_border(int four_cells_mask) const {

  switch (four_cells_mask) {

  // 0 0
  // 0 0
  case 0:
  return -1;

  // 0 0
  // 0 1
  case 1:
  return 5;

  // 0 0
  // 1 0
  case 2:
  return 4;

  // 0 0
  // 1 1
  case 3:
  return 1;

  // 0 1
  // 0 0
  case 4:
  return 6;

  // 0 1
  // 0 1
  case 5:
  return 2;

  // 0 1
  // 1 0
  case 6:
  return 5;  // TODO

  // 0 1
  // 1 1
  case 7:
  return 9;

  // 1 0
  // 0 0
  case 8:
  return 7;

  // 1 0
  // 0 1
  case 9:
  return 7;  // TODO

  // 1 0
  // 1 0
  case 10:
  return 0;

  // 1 0
  // 1 1
  case 11:
  return 8;

  // 1 1
  // 0 0
  case 12:
  return 3;

  // 1 1
  // 0 1
  case 13:
  return 10;

  // 1 1
  // 1 0
  case 14:
  return 11;

  // 1 1
  // 1 1
  case 15:
  return -1;

  }

  return -1;
}

/**
 * @brief Returns whether a border type is a side or a corner.
 * @param which_border A border type between 0 and 11 or -1.
 * @return @c true if this is a side.
 */
bool AutoTiler::is_side_border(int which_border) const {

  return which_border < 4;
}

/**
 * @brief Creates a tile depending on its border type and the
 * current position in the grid.
 * @param which_border Kind of border to create.
 * @param cell_0 Top-left cell of the current 8 cells being analyzed.
 * @param num_cells_repeat On how many cells of the 8x8 grid the pattern should be repeated
 * (ignored for corners).
 */
void AutoTiler::make_tile(int which_border, int cell_0, int num_cells_repeat) {

  if (which_border == -1) {
    return;
  }
  Q_ASSERT(which_border >= 0);
  Q_ASSERT(which_border < 12);
  Q_ASSERT(num_cells_repeat > 0);

  int cell_1 = cell_0 + 1;
  int cell_2 = cell_0 + grid_size.width();
  int cell_3 = cell_2 + 1;

  QPoint xy;
  QSize size;
  const QString& pattern_id = border_pattern_ids[which_border];

  const TilesetModel& tileset = *map.get_tileset_model();
  const QSize& pattern_size = tileset.get_pattern_frame(tileset.id_to_index(pattern_id)).size();
  Q_ASSERT(!pattern_size.isEmpty());

  int size_repeated = num_cells_repeat * 8;

  switch (which_border) {

  case 0:  // Right side.
    size = { pattern_size.width(), size_repeated };
    xy = to_map_xy(cell_0);
    break;

  case 1:  // Top side.
    size = { size_repeated, pattern_size.height() };
    xy = to_map_xy(cell_2);
    break;

  case 2:  // Left side.
    size = { pattern_size.width(), size_repeated };
    xy = to_map_xy(cell_1);
    break;

  case 3:  // Bottom side.
    size = { size_repeated, pattern_size.height() };
    xy = to_map_xy(cell_0);
    break;

  case 4:  // Top-right convex corner.
  case 8:  // Top-right concave corner.
    xy = to_map_xy(cell_2);
    size = pattern_size;
    break;

  case 5:  // Top-left convex corner.
  case 9:  // Top-left concave corner.
    xy = to_map_xy(cell_3);
    size = pattern_size;
    break;

  case 6:  // Bottom-left convex corner.
  case 10:  // Bottom-left concave corner.
    xy = to_map_xy(cell_1);
    size = pattern_size;
    break;

  case 7:  // Bottom-right convex corner.
  case 11:  // Bottom-right concave corner.
    xy = to_map_xy(cell_0);
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
 * @brief Creates the border tiles.
 */
void AutoTiler::compute_borders() {

  tiles.clear();

  for (const QRect& rectangle : entity_rectangles) {

    int num_cells_x = rectangle.width() / 8;
    int num_cells_y = rectangle.height() / 8;

    // Top side.
    int rectangle_top_left_cell = to_grid_index(rectangle.topLeft());
    int initial_position = rectangle_top_left_cell - 1 - grid_size.width();  // 1 cell above and to the left.
    int cell_0 = initial_position;

    int num_cells_of_side = 0;
    for (int i = 0; i < num_cells_x; ++i) {

      int mask = get_four_cells_mask(cell_0);
      int which_border = get_which_border(mask);

      if (which_border != -1) {
        if (is_side_border(which_border)) {
          // This is a side border: count how many there are.
          ++num_cells_of_side;
        }
        else {
          // This is a corner.
          if (num_cells_of_side > 0) {
            // First, generate the side tile now that we know its final size.
            make_tile(1, cell_0 - num_cells_of_side, num_cells_of_side + 1);
            num_cells_of_side = 0;
          }

          // Now generate the corner.
          make_tile(which_border, cell_0, 1);
        }
      }
      ++cell_0;
    }
    if (num_cells_of_side > 0) {
      make_tile(1, cell_0 - num_cells_of_side, num_cells_of_side + 1);
      num_cells_of_side = 0;
    }

    // Right side.
    for (int i = 0; i < num_cells_y; ++i) {

      int mask = get_four_cells_mask(cell_0);
      int which_border = get_which_border(mask);

      if (which_border != -1) {
        if (is_side_border(which_border)) {
          // This is a side border: count how many there are.
          ++num_cells_of_side;
        }
        else {
          // This is a corner.
          // First, generate the side tile now that we know its final size.
          if (num_cells_of_side > 0) {
            make_tile(0, cell_0 - num_cells_of_side * grid_size.width(), num_cells_of_side + 1);
            num_cells_of_side = 0;
          }

          // Now generate the corner.
          make_tile(which_border, cell_0, 1);
        }
      }

      cell_0 += grid_size.width();
    }
    if (num_cells_of_side > 0) {
      make_tile(0, cell_0 - num_cells_of_side * grid_size.width(), num_cells_of_side + 1);
      num_cells_of_side = 0;
    }

    // Bottom side.
    for (int i = 0; i < num_cells_x; ++i) {

      int mask = get_four_cells_mask(cell_0);
      int which_border = get_which_border(mask);

      if (which_border != -1) {
        if (is_side_border(which_border)) {
          // This is a side border: count how many there are.
          ++num_cells_of_side;
        }
        else {
          // This is a corner.
          // First, generate the side tile now that we know its final size.
          if (num_cells_of_side > 0) {
            make_tile(3, cell_0 + 1, num_cells_of_side + 1);
            num_cells_of_side = 0;
          }

          // Now generate the corner.
          make_tile(which_border, cell_0, 1);
        }
      }

      --cell_0;
    }
    if (num_cells_of_side > 0) {
      make_tile(3, cell_0 + 1, num_cells_of_side + 1);
      num_cells_of_side = 0;
    }

    // Left side.
    for (int i = 0; i < num_cells_y; ++i) {

      int mask = get_four_cells_mask(cell_0);
      int which_border = get_which_border(mask);

      if (which_border != -1) {
        if (is_side_border(which_border)) {
          // This is a side border: count how many there are.
          ++num_cells_of_side;
        }
        else {
          // This is a corner.
          // First, generate the side tile now that we know its final size.
          if (num_cells_of_side > 0) {
            make_tile(2, cell_0 + grid_size.width(), num_cells_of_side + 1);
            num_cells_of_side = 0;
          }

          // Now generate the corner.
          make_tile(which_border, cell_0, 1);
        }
      }

      cell_0 -= grid_size.width();
    }
    if (num_cells_of_side > 0) {
      make_tile(2, cell_0 + grid_size.width(), num_cells_of_side + 1);
      num_cells_of_side = 0;
    }
  }
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

  // Detects the borders to make.
  compute_borders();
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
