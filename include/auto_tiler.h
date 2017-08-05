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
#ifndef SOLARUSEDITOR_AUTO_TILER_H
#define SOLARUSEDITOR_AUTO_TILER_H

#include "entities/entity_traits.h"
#include "map_model.h"
#include <QList>
#include <map>
#include <set>

namespace SolarusEditor {

class MapModel;

/**
 * @brief Generates border tiles around some entities.
 */
class AutoTiler {

public:

  enum class WhichBorder {
    NONE = -1,
    RIGHT,
    TOP,
    LEFT,
    BOTTOM,
    TOP_RIGHT_CONVEX,
    TOP_LEFT_CONVEX,
    BOTTOM_LEFT_CONVEX,
    BOTTOM_RIGHT_CONVEX,
    TOP_RIGHT_CONCAVE,
    TOP_LEFT_CONCAVE,
    BOTTOM_LEFT_CONCAVE,
    BOTTOM_RIGHT_CONCAVE,
  };

  AutoTiler(MapModel& map, const EntityIndexes& entity_indexes);

  AddableEntities generate_border_tiles();

private:

  int get_num_cells() const;
  int to_grid_index(const QPoint& xy) const;
  QPoint to_map_xy(int grid_index) const;
  bool is_cell_occupied(int grid_index) const;
  int get_four_cells_mask(int cell_0) const;
  bool is_side_border(WhichBorder which_border) const;
  bool has_border(int grid_index) const;
  WhichBorder get_which_border(int grid_index) const;
  void set_which_border(int grid_index, WhichBorder which_border);
  void detect_border_info(int cell_0);
  void print_which_borders() const;
  void make_tile(WhichBorder which_border, int grid_index, int num_cells_repeat);

  void compute_bounding_box();
  void compute_occupied_squares();
  void compute_borders();
  void compute_tiles();

  MapModel& map;                       /**< The map that will be modified. */
  EntityIndexes entity_indexes;        /**< Entities where to create a border. */
  QList<QRect> entity_rectangles;      /**< Rectangles of entities where to create a border. */
  QRect bounding_box;                  /**< Rectangle containing the entities plus 8 pixels of margin. */
  QSize grid_size;                     /**< Number of cells in the 8x8 grid in X and Y. */
  std::vector<bool> occupied_squares;  /**< Squares of the 8x8 grid that are occupied by an entity. */
  std::map<int, WhichBorder>
      which_borders;                   /**< Which kind of border to create in each square of the 8x8 grid. */
  EntityModels tiles;                  /**< Border tiles created. */
};

}

#endif
