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
#include "entity_model.h"
#include "map_model.h"
#include "point.h"

/**
 * @brief Creates an entity model.
 * @param map The map containing the entity.
 * @param index Index of the tile pattern to represent.
 */
EntityModel::EntityModel(MapModel& map, const Solarus::EntityData& entity) :
  map(&map),
  entity(entity),
  origin(0, 0),
  size(16, 16),
  image() {
}

/**
 * @brief Destructor.
 */
EntityModel::~EntityModel() {
}

/**
 * @brief Returns the map this entity belongs to.
 * @return The map.
 */
const MapModel& EntityModel::get_map() const {
  return *map;
}

/**
 * @brief Returns the map this entity belongs to.
 *
 * Non-const version.
 *
 * @return The map.
 */
MapModel& EntityModel::get_map() {
  return *map;
}

/**
 * @brief Returns the type of this entity.
 * @return The entity type.
 */
EntityType EntityModel::get_type() const {
  return entity.get_type();
}

/**
 * @brief Returns the Lua type name of this entity.
 * @return The entity type name.
*/
QString EntityModel::get_type_name() const {
  return QString::fromStdString(entity.get_type_name());
}

/**
 * @brief Returns the coordinates of this entity on the map.
 * @return The coordinates of the entity's origin point.
 */
QPoint EntityModel::get_xy() const {
  return Point::to_qpoint(entity.get_xy());
}

/**
 * @brief Sets the coordinates of this entity on the map.
 * @param xy The coordinates of the entity's origin point.
 */
void EntityModel::set_xy(const QPoint& xy) {
  entity.set_xy(Point::to_solarus_point(xy));
}

/**
 * @brief Returns the coordinates of the upper-left corner of this entity.
 * @return The coordinates of the entity's upper-left corner.
 */
QPoint EntityModel::get_top_left() const {
  return get_xy() - get_origin();
}

/**
 * @brief Sets the coordinates of the upper-left corner of this entity.
 * @param top_left The coordinates of the entity's upper-left corner.
 */
void EntityModel::set_top_left(const QPoint& top_left) {
  set_xy(top_left + get_origin());
}

/**
 * @brief Returns the origin point of this entity.
 * @return The origin point.
 */
QPoint EntityModel::get_origin() const {
  return origin;
}

/**
 * @brief Sets the origin point of this entity.
 *
 * The origin point defines the relation between
 * get_top_left() and get_xy().
 * If you change the origin point, get_xy() is preserved, but
 * get_top_left() and get_bounding_box() change.
 *
 * @return The origin point.
 */
void EntityModel::set_origin(const QPoint& origin) {
  this->origin = origin;
}

/**
 * @brief Returns the width of this entity's bounding box.
 * @return The width.
 */
int EntityModel::get_width() const {
  return get_size().width();
}

/**
 * @brief Sets the width of this entity's bounding box.
 * @param width The width.
 */
void EntityModel::set_width(int width) {
  set_size(QSize(width, get_height()));
}

/**
 * @brief Returns the height of this entity's bounding box.
 * @return The height.
 */
int EntityModel::get_height() const {
  return get_size().height();
}

/**
 * @brief Sets the height of this entity's bounding box.
 * @param height The height.
 */
void EntityModel::set_height(int height) {
  set_size(QSize(get_width(), height));
}

/**
 * @brief Returns the size of this entity.
 * @return The size of this entity.
 */
QSize EntityModel::get_size() const {
  return size;
}

/**
 * @brief Sets the size of this entity.
 * @param size The size of this entity.
 */
void EntityModel::set_size(const QSize& size) {
  this->size = size;
}

/**
 * @brief Returns the bounding box of this entity.
 * @return The bounding box.
 */
QRect EntityModel::get_bounding_box() const {
  return QRect(get_top_left(), get_size());
}

/**
 * @brief Returns the image representing the entity in the editor.
 * @return The entity's image.
 */
const QPixmap& EntityModel::get_image() const {

  if (image.isNull()) {
    // Lazily create the image.
    image = QPixmap(QString(":/images/entity_%1.png").arg(get_type_name())).scaledToHeight(get_height());  // TODO
  }

  return image;
}
