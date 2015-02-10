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
#include "entities/chest.h"
#include "entities/destination.h"
#include "entities/destructible.h"
#include "entities/entity_model.h"
#include "entities/tile.h"
#include "map_model.h"
#include "point.h"
#include <QPainter>

/**
 * @brief Creates an entity model.
 * @param map The map containing the entity.
 * @param entity The entity data to represent.
 */
EntityModel::EntityModel(MapModel& map, const Solarus::EntityData& entity) :
  map(&map),
  entity(entity),
  origin(0, 0),
  size(16, 16),
  icon() {

  // If the entity has explicit size information in its properties, use it.
  if (entity.is_integer("width") && entity.is_integer("height")) {
    set_size(QSize(entity.get_integer("width"), entity.get_integer("height")));
  }
}

/**
 * @brief Destructor.
 */
EntityModel::~EntityModel() {
}

/**
 * @brief Creates an entity model of the appropriate concrete type from data.
 * @param map The map containing the entity.
 * @param entity The entity data to represent.
 * @return The created model.
 */
std::unique_ptr<EntityModel> EntityModel::create(
    MapModel& map, const Solarus::EntityData& entity_data) {

  EntityModel* entity = nullptr;

  switch (entity_data.get_type()) {

  case EntityType::CHEST:
    entity = new Chest(map, entity_data);
    break;

  case EntityType::DESTINATION:
    entity = new Destination(map, entity_data);
    break;

  case EntityType::DESTRUCTIBLE:
    entity = new Destructible(map, entity_data);
    break;

  case EntityType::TILE:
    entity = new Tile(map, entity_data);
    break;

  default:
    // TODO other types
    entity = new EntityModel(map, entity_data);
    break;

  }

  return std::unique_ptr<EntityModel>(entity);
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
 * @brief Returns the tileset of the map.
 * @return The tileset or nullptr if no tileset is set.
 */
const TilesetModel* EntityModel::get_tileset() const {
  return get_map().get_tileset_model();
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
 * @brief Returns the layer where this entity is on the map.
 * @return The layer.
 */
Layer EntityModel::get_layer() const {
  return entity.get_layer();
}

/**
 * @brief Sets the layer where this entity is on the map.
 * @param layer The layer.
 */
void EntityModel::set_layer(Layer layer) {
  entity.set_layer(layer);
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
 * @brief Returns the value of a field of this entity.
 * @param key Key of the field to get.
 * @return The corresponding value.
 * It can be a string, an integer or a boolean, or
 * an invalid QVariant if the field does not exist.
 */
QVariant EntityModel::get_field(const QString& key) const {

  std::string std_key = key.toStdString();

  if (entity.is_string(std_key)) {
    return QString::fromStdString(entity.get_string(std_key));
  }

  if (entity.is_integer(std_key)) {
    return entity.get_integer(std_key);
  }

  if (entity.is_boolean(std_key)) {
    return entity.get_boolean(std_key);
  }

  return QVariant();
}

/**
 * @brief Draws this entity.
 *
 * The default implementation draws an icon representing the type of entity.
 *
 * @param painter The painter to draw.
 */
void EntityModel::draw(QPainter& painter) const {

  if (icon.isNull()) {
    // Lazily create the image.
    icon = QPixmap(QString(":/images/entity_%1.png").arg(get_type_name()));
  }

  painter.drawTiledPixmap(0, 0, get_width() * 2, get_height() * 2, icon);
}
