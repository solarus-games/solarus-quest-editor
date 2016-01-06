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
#ifndef SOLARUSEDITOR_GRID_STYLE_H
#define SOLARUSEDITOR_GRID_STYLE_H

#include "enum_traits.h"

/**
 * @brief Grid Styles.
 */
enum class GridStyle {
  PLAIN = 0,
  DASHED,
  INTERSECT_CROSS,
  INTERSECT_POINT
};

using GridStyleTraits = EnumTraits<GridStyle>;

/**
 * \brief Provides useful properties of grid style enum values.
 */
template<>
class EnumTraits<GridStyle> {

public:

  static QList<GridStyle> get_values();
  static QString get_friendly_name(GridStyle value);
  static QIcon get_icon(GridStyle value);

};

#endif
