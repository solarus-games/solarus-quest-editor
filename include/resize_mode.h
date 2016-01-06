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
#ifndef SOLARUSEDITOR_RESIZE_MODE_H
#define SOLARUSEDITOR_RESIZE_MODE_H

/**
 * @brief How an entity can be resized.
 */
enum class ResizeMode {
  NONE,                      /**< Cannot be resized. */
  HORIZONTAL_ONLY,           /**< Can only be resized horizontally. */
  VERTICAL_ONLY,             /**< Can only be resized vertically. */
  SQUARE,                    /**< Can only stay square. */
  SINGLE_DIMENSION,          /**< Can be resized horizontally or vertically
                              * but not both. */
  MULTI_DIMENSION_ONE,       /**< Can be resized horizontally, vertically
                              * or both but not at the same time. */
  MULTI_DIMENSION_ALL        /**< Can be resized horizontally, vertically
                              * or both even at the same time. */
};

#endif
