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
#include <set>

namespace SolarusEditor {

class MapModel;

/**
 * @brief Generates border tiles around some entities.
 */
class AutoTiler {

public:

  AutoTiler(MapModel& map, const EntityIndexes& entity_indexes);

  AddableEntities generate_border_tiles();

private:

  /**
   * @brief Tells where to create a border tile.
   */
  struct BorderInfo {

    BorderInfo():
      xy(),
      which_border(-1),
      tile_size() {

    }

    BorderInfo(const QPoint& xy, int which_border, int tile_size = 0):
      xy(xy),
      which_border(which_border),
      tile_size(tile_size) {

    }

    QPoint xy;         /**< Where to put the border tile in map coordinates. */
    int which_border;  /**< Which border to put (0 to 11). */
    int tile_size;     /**< Width (for top/bottom borders) or height
                        * (for left/right borders) of the border tile. */
  };

  int get_num_cells() const;
  int to_grid_index(const QPoint& xy) const;
  QPoint to_map_xy(int grid_index) const;
  bool is_cell_occupied(int grid_index) const;
  int get_four_cells_mask(int cell_0) const;
  int get_which_border(int four_cells_mask) const;
  BorderInfo determine_border_info(int cell_0, int which_border);

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
  QList<BorderInfo> borders;           /**< Where to create border tiles. */
  EntityModels tiles;                  /**< Border tiles created. */
};

}

#endif
