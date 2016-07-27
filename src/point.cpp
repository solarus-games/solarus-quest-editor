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
#include <QSize>
#include <QtMath>

namespace SolarusEditor {

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

  return round_8(point.toPoint());
}

/**
 * @brief Floors the coordinates of a point to the lower multiples of 8.
 * @param point A point.
 * @return The resulting point.
 */
QPoint floor_8(const QPoint& point) {

  return floor(point, QSize(8, 8));
}

/**
 * @overload
 */
QPoint floor_8(const QPointF& point) {

  return floor(point, QSize(8, 8));
}

/**
 * @brief Floors the coordinates of a point to the lower multiples of a size.
 * @param point A point.
 * @param size A size.
 * @return The resulting point.
 */
QPoint floor(const QPoint& point, const QSize& size) {

  return QPoint(point.x() - point.x() % size.width(),
                point.y() - point.y() % size.height());
}

/**
 * @overload
 */
QPoint floor(const QPointF& point, const QSize& size) {

  return floor(
      QPoint(qFloor(point.x()), qFloor(point.y())),
      size
  );
}

/**
 * @brief Ceils the coordinates of a point to the upper multiples of a size.
 * @param point A point.
 * @param size A size.
 * @return The resulting point.
 */
QPoint ceil(const QPoint& point, const QSize& size) {

  int mod_x = point.x() % size.width();
  int mod_y = point.y() % size.height();

  QPoint result = point;
  if (mod_x != 0) {
    if (point.x() > 0) {
      result.setX(point.x() - mod_x + size.width());
    }
    else {
      result.setX(point.x() - mod_x - size.width());
    }
  }
  if (mod_y != 0) {
    if (point.y() > 0) {
      result.setY(point.y() - mod_y + size.height());
    }
    else {
      result.setY(point.y() - mod_y - size.height());
    }
  }
  return result;
}

/**
 * @overload
 */
QPoint ceil(const QPointF& point, const QSize& size) {

  return ceil(
      QPoint(qCeil(point.x()), qCeil(point.y())),
      size
  );
}

/**
 * @brief Rounds the coordinates of a point down to the nearest lower multiple
 * of given coordinates.
 *
 * round_down(QPoint(15, -2), 8, 8) is QPoint(8, -8).
 *
 * @param point A point.
 * @param step_x Value where to round the x coordinate.
 * @param step_y Value where to round the y coordinate.
 * @return The resulting point.
 */
QPoint round_down(const QPoint& point, const QSize& size) {

  int mod_x = point.x() % size.width();
  int mod_y = point.y() % size.height();
  QPoint result(point.x() - mod_x,
                point.y() - mod_y);

  // Fix the bias of negative values going towards zero.
  if (point.x() < 0 && mod_x != 0) {
    result.setX(result.x() - size.width());
  }
  if (point.y() < 0 && mod_y != 0) {
    result.setY(result.y() - size.height());
  }
  return result;
}

/**
 * @overload
 */
QPoint round_down(const QPointF& point, const QSize& size) {

  return round_down(point.toPoint(), size);
}

}

}
