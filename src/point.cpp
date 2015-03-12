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
#include "point.h"

namespace Point {

/**
 * @brief Converts a Solarus point to a Qt point.
 * @param solarus_point A point for the Solarus library.
 * @return The equivalent QPoint.
 */
QPoint to_qpoint(const Solarus::Point& solarus_point) {

  return QPoint(solarus_point.x, solarus_point.y);
}

/**
 * @brief Converts a Qt point to a Solarus point.
 * @param qt_point A Qt point.
 * @return The equivalent Solarus point.
 */
Solarus::Point to_solarus_point(const QPoint& qt_point) {

  return Solarus::Point(qt_point.x(), qt_point.y());
}

/**
 * @brief Rounds the coordinates of a point to the nearest multiples of 8.
 * @param point A point.
 * @return The rounded point.
 */
QPoint round_8(const QPoint& point) {

  // The division operator of QPoint already implements rounding to the
  // nearest integer.
  return point / 8 * 8;
}

/**
 * @overload
 */
QPoint round_8(const QPointF& point) {

  // The division operator of QPoint already implements rounding to the
  // nearest integer.
  return round_8(point.toPoint());
}

}
