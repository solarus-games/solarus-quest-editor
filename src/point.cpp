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
#include "point.h"
#include <QtMath>

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

/**
 * @brief Floors the coordinates of a point to the lower multiples of 8.
 * @param point A point.
 * @return The resulting point.
 */
QPoint floor_8(const QPoint& point) {

  return QPoint(point.x() - point.x() % 8,
                point.y() - point.y() % 8);
}

/**
 * @overload
 */
QPoint floor_8(const QPointF& point) {

  return floor_8(point.toPoint());
}

/**
 * @brief Rounds the coordinates of a point down to the nearest multiple
 * of given coordinates.
 *
 * round_down(QPoint(15, -2), 8, 8) is QPoint(8, -8).
 *
 * @param point A point.
 * @param step_x Value where to round the x coordinate.
 * @param step_y Value where to round the y coordinate.
 * @return The resulting point.
 */
QPoint round_down(const QPoint& point, int step_x, int step_y) {

  int mod_x = point.x() % step_x;
  int mod_y = point.y() % step_y;
  QPoint result(point.x() - mod_x,
                point.y() - mod_y);
  if (mod_x > 0) {
    result.setX(result.x() + step_x);
  }
  if (mod_y > 0) {
    result.setY(result.y() + step_y);
  }
  return result;
}

/**
 * @overload
 */
QPoint round_down(const QPointF& point, int step_x, int step_y) {

  return round_down(point.toPoint(), step_x, step_y);
}

}
