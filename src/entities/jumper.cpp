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
#include "entities/jumper.h"
#include <QPainter>

/**
 * @brief Constructor.
 * @param map The map containing the entity.
 * @param index Index of the entity in the map.
 */
Jumper::Jumper(MapModel& map, const EntityIndex& index) :
  EntityModel(map, index, EntityType::JUMPER) {

  DrawShapeInfo info;
  info.enabled = true;
  info.background_color = QColor(48, 184, 208);
  info.between_border_color = QColor(144, 224, 240);
  set_draw_shape_info(info);

  // TODO change the resizable setting when the direction is changed.
  set_resizable(true);
}

/**
 * @brief Returns whether the jump is horizontal.
 *
 * If yes, the jumper's shape is vertical.
 *
 * @return @c true if the jump is horizontal.
 */
bool Jumper::is_jump_horizontal() const {

  const int direction = get_direction();
  if (direction == -1) {
    // No value is set.
    return false;
  }

  return direction == 0 || direction == 4;
}

/**
 * @brief Returns whether the jump is vertical.
 *
 * If yes, the jumper's shape is horizontal.
 *
 * @return @c true if the jump is vertical.
 */
bool Jumper::is_jump_vertical() const {

  const int direction = get_direction();
  if (direction == -1) {
    // No value is set.
    return false;
  }

  return direction == 2 || direction == 6;
}

/**
 * @brief Returns whether this jumper is a diagonal one.
 * @return @c true if the jumper is diagonal.
 */
bool Jumper::is_jump_diagonal() const {

  const int direction = get_direction();
  if (direction == -1) {
    // No value is set.
    return false;
  }

  return direction % 2 != 0;
}

/**
 * @copydoc EntityModel::notify_field_changed
 */
void Jumper::notify_field_changed(const QString& key, const QVariant& value) {

  Q_UNUSED(value);
  if (key == "direction") {

    // Update the resizing rules.
    if (is_jump_diagonal()) {
      set_resize_mode(ResizeMode::SQUARE);
      set_size(QSize(32, 32));
    }
    else {
      if (is_jump_horizontal()) {
        set_resize_mode(ResizeMode::VERTICAL_ONLY);
        set_size(QSize(8, 32));
      }
      else {
        set_resize_mode(ResizeMode::HORIZONTAL_ONLY);
        set_size(QSize(32, 0));
      }
    }
  }
}

/**
 * @copydoc EntityModel::draw
 *
 * Reimplemented because diagonal jumpers have a special shape.
 */
void Jumper::draw(QPainter& painter) const {

  const int w = get_width();
  const int h = get_height();

  if (!is_jump_diagonal() || w != h) {
    // Horizontal or vertical, or diagonal but with wrong size.
    EntityModel::draw(painter);
    return;
  }

  // Diagonal jumper.
  const QColor background_color(48, 184, 208);
  const QColor between_border_color(144, 224, 240);
  const QColor border_color(Qt::black);
  QPainterPath outer_path;
  QPainterPath inner_path;

  switch (get_direction()) {

  case 1:  // North-East.
    /*  __
     *  \ \
     *   \ \
     *    \ \
     *     \ \
     *      \|
     *
     */

    outer_path.lineTo(QPoint(8, 0));
    outer_path.lineTo(QPoint(w, h - 8));
    outer_path.lineTo(QPoint(w, h));
    outer_path.closeSubpath();

    inner_path.moveTo(QPoint(4, 2));
    inner_path.lineTo(QPoint(8, 2));
    inner_path.lineTo(QPoint(w - 2, h - 8));
    inner_path.lineTo(QPoint(w - 2, h - 4));
    inner_path.closeSubpath();

    break;

  case 3:  // North-West.
    /*      __
     *     / /
     *    / /
     *   / /
     *  / /
     *  |/
     *
     */

    outer_path.moveTo(QPoint(w - 8, 0));
    outer_path.lineTo(QPoint(w, 0));
    outer_path.lineTo(QPoint(0, h));
    outer_path.lineTo(QPoint(0, h - 8));
    outer_path.closeSubpath();

    inner_path.moveTo(QPoint(w - 8, 2));
    inner_path.lineTo(QPoint(w - 4, 2));
    inner_path.lineTo(QPoint(2, h - 4));
    inner_path.lineTo(QPoint(2, h - 8));
    inner_path.closeSubpath();

    break;

  case 5:  // South-West.
    /*
     *  |\
     *  \ \
     *   \ \
     *    \ \
     *     \_\
     *
     */

    outer_path.lineTo(QPoint(w, h));
    outer_path.lineTo(QPoint(w - 8, h));
    outer_path.lineTo(QPoint(0, 8));
    outer_path.closeSubpath();

    inner_path.moveTo(QPoint(2, 4));
    inner_path.lineTo(QPoint(w - 4, h - 2));
    inner_path.lineTo(QPoint(w - 8, h - 2));
    inner_path.lineTo(QPoint(2, 8));
    inner_path.closeSubpath();

    break;

  case 7:  // South-East.
    /*
     *      /|
     *     / /
     *    / /
     *   / /
     *  /_/
     *
     */

    outer_path.moveTo(QPoint(w, 0));
    outer_path.lineTo(QPoint(w, 8));
    outer_path.lineTo(QPoint(8, h));
    outer_path.lineTo(QPoint(0, h));
    outer_path.closeSubpath();

    inner_path.moveTo(QPoint(w - 2, 4));
    inner_path.lineTo(QPoint(w - 2, 8));
    inner_path.lineTo(QPoint(8, h - 2));
    inner_path.lineTo(QPoint(4, h - 2));
    inner_path.closeSubpath();

    break;

  }

  painter.setPen(QPen(border_color, 0, Qt::SolidLine));
  painter.fillPath(outer_path, between_border_color);
  painter.fillPath(inner_path, background_color);
  painter.drawPath(outer_path);
  painter.drawPath(inner_path);

}
