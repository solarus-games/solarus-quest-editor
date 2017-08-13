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
#ifndef SOLARUSEDITOR_WHICH_BORDER_H
#define SOLARUSEDITOR_WHICH_BORDER_H

namespace SolarusEditor {

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

}

#endif
