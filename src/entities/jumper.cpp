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
 * @param entity The entity data to represent.
 */
Jumper::Jumper(MapModel& map, const Solarus::EntityData& entity) :
  EntityModel(map, entity) {

  Q_ASSERT(entity.get_type() == EntityType::JUMPER);

  DrawShapeInfo info;
  info.enabled = true;
  info.background_color = QColor(48, 184, 208);
  info.between_border_color = QColor(144, 224, 240);
  set_draw_shape_info(info);
}

/**
 * @brief Returns whether this jumper is a diagonal one.
 * @return @c true if the jumper is diagonal.
 */
bool Jumper::is_diagonal() const {

  const int direction = get_direction();
  if (direction == -1) {
    // No value is set.
    return false;
  }

  return direction % 2 != 0;
}

/**
 * @copydoc EntityModel::draw
 *
 * Reimplemented because diagonal jumpers have a special shape.
 */
void Jumper::draw(QPainter& painter) const {

  if (!is_diagonal()) {
    // Horizontal or vertical.
    EntityModel::draw(painter);
  }

  // Diagonal jumper.
  const QColor background_color(48, 184, 208);
  //const QColor between_border_color(144, 224, 240);

  QPainterPath path;
  switch (get_direction()) {

  case 1:

    break;

  case 3:

    break;

  case 5:

    break;

  case 7:

    break;

  }

  painter.fillPath(path, background_color);

}
