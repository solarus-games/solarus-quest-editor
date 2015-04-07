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
#include "entities/block.h"
#include "entities/chest.h"
#include "entities/custom_entity.h"
#include "entities/crystal.h"
#include "entities/crystal_block.h"
#include "entities/destination.h"
#include "entities/destructible.h"
#include "entities/door.h"
#include "entities/dynamic_tile.h"
#include "entities/enemy.h"
#include "entities/entity_model.h"
#include "entities/jumper.h"
#include "entities/pickable.h"
#include "entities/npc.h"
#include "entities/sensor.h"
#include "entities/separator.h"
#include "entities/shop_treasure.h"
#include "entities/stairs.h"
#include "entities/stream.h"
#include "entities/switch.h"
#include "entities/teletransporter.h"
#include "entities/tile.h"
#include "entities/wall.h"
#include "gui/gui_tools.h"
#include "map_model.h"
#include "point.h"
#include "quest.h"
#include "quest_resources.h"
#include "sprite_model.h"
#include <QDebug>
#include <QPainter>

using EntityData = Solarus::EntityData;

/**
 * @brief Creates an entity model.
 * @param map The map that contains or will contain the entity.
 * @param index Index of the entity in the map, or an invalid index if the
 * entity is not in the map yet.
 * @param type Type of entity to create (only needed if the entity is not
 * in the map yet).
 */
EntityModel::EntityModel(
    MapModel& map,
    const EntityIndex& index,
    EntityType type) :
  map(&map),
  index(index),
  stub(type),
  name(),
  origin(0, 0),
  size(16, 16),
  base_size(16, 16),
  resize_mode(ResizeMode::NONE),
  has_preferred_layer(false),
  preferred_layer(Layer::LAYER_LOW),
  draw_sprite_info(),
  sprite_model(nullptr),
  sprite_image(),
  draw_shape_info(),
  draw_image_info(),
  icon() {

  // If the entity has explicit size information in its properties, use it.
  const EntityData& entity = get_entity();
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
 * @brief Creates an entity model for a new entity of the given type.
 * @param map The map that will contain the entity.
 * @param type Type of entity to create.
 * @return The created model.
 */
EntityModelPtr EntityModel::create(
    MapModel& map, EntityType type) {

  return create(map, EntityIndex(), type);
}

/**
 * @brief Creates an entity model for a new entity from a string.
 *
 * The created clone is not on the map yet.
 *
 * @param map The map that will contain the entity.
 * @param string The string representation of the entity.
 * @return The created model or nullptr if the string is invalid.
 */
EntityModelPtr EntityModel::create(
    MapModel& map, const QString& entity_string) {

  Solarus::EntityData data;
  if (entity_string.simplified().isEmpty()) {
    // Empty string or only whitespaces.
    return nullptr;
  }

  if (!data.import_from_buffer(entity_string.toStdString())) {
    return nullptr;
  }

  EntityModelPtr entity = create(map, EntityIndex(), data.get_type());
  entity->set_entity(data);
  entity->index = EntityIndex();
  entity->name = QString::fromStdString(data.get_name());

  return entity;
}

/**
 * @brief Creates an entity model of the appropriate concrete type
 * from the an existing entity of the map.
 * @param map The map that contains the entity.
 * @param index Index of the entity in the map.
 * @return The created model.
 */
EntityModelPtr EntityModel::create(
    MapModel& map, const EntityIndex& index) {

  return create(map, index, map.get_internal_entity(index).get_type());
}

/**
 * @brief Clones an existing entity of the map.
 *
 * The created clone is not on the map.
 *
 * @param map The map that contains the existing entity and will contain the
 * new one.
 * @param index Index of the entity to clone.
 * @return The created model.
 */
EntityModelPtr EntityModel::clone(
    MapModel& map, const EntityIndex& index) {

  const Solarus::EntityData& existing_data = map.get_internal_entity(index);
  EntityModelPtr clone = create(map, EntityIndex(), existing_data.get_type());
  clone->set_entity(existing_data);
  clone->index = EntityIndex();
  clone->name = map.get_entity_name(index);
  return clone;
}

/**
 * @brief Creates an entity model of the appropriate concrete type.
 * @param map The map that contains or will contain the entity.
 * @param index Index of the entity in the map, or an invalid index if the
 * entity is not in the map yet.
 * @param type Type of entity to create (only needed if the entity is not
 * in the map yet).
 * @return The created model.
 */
EntityModelPtr EntityModel::create(
    MapModel& map, const EntityIndex& index, EntityType type) {

  EntityModel* entity = nullptr;

  if (index.is_valid()) {
    type = map.get_internal_entity(index).get_type();
  }

  switch (type) {

  case EntityType::BLOCK:
    entity = new Block(map, index);
    break;

  case EntityType::CHEST:
    entity = new Chest(map, index);
    break;

  case EntityType::CRYSTAL:
    entity = new Crystal(map, index);
    break;

  case EntityType::CRYSTAL_BLOCK:
    entity = new CrystalBlock(map, index);
    break;

  case EntityType::CUSTOM:
    entity = new CustomEntity(map, index);
    break;

  case EntityType::DESTINATION:
    entity = new Destination(map, index);
    break;

  case EntityType::DESTRUCTIBLE:
    entity = new Destructible(map, index);
    break;

  case EntityType::DOOR:
    entity = new Door(map, index);
    break;

  case EntityType::DYNAMIC_TILE:
    entity = new DynamicTile(map, index);
    break;

  case EntityType::ENEMY:
    entity = new Enemy(map, index);
    break;

  case EntityType::JUMPER:
    entity = new Jumper(map, index);
    break;

  case EntityType::NPC:
    entity = new Npc(map, index);
    break;

  case EntityType::PICKABLE:
    entity = new Pickable(map, index);
    break;

  case EntityType::SENSOR:
    entity = new Sensor(map, index);
    break;

  case EntityType::SEPARATOR:
    entity = new Separator(map, index);
    break;

  case EntityType::SHOP_TREASURE:
    entity = new ShopTreasure(map, index);
    break;

  case EntityType::STAIRS:
    entity = new Stairs(map, index);
    break;

  case EntityType::STREAM:
    entity = new Stream(map, index);
    break;

  case EntityType::SWITCH:
    entity = new Switch(map, index);
    break;

  case EntityType::TELETRANSPORTER:
    entity = new Teletransporter(map, index);
    break;

  case EntityType::TILE:
    entity = new Tile(map, index);
    break;

  case EntityType::WALL:
    entity = new Wall(map, index);
    break;

  case EntityType::ARROW:
  case EntityType::BOMB:
  case EntityType::BOOMERANG:
  case EntityType::CARRIED_ITEM:
  case EntityType::EXPLOSION:
  case EntityType::FIRE:
  case EntityType::HERO:
  case EntityType::HOOKSHOT:
    qCritical() << MapModel::tr("Unexpected entity type (not allowed in map files): %1").arg(
                      EntityTraits::get_lua_name(type));
    break;
  }

  if (!index.is_valid()) {
    // New entity: initialize aligned x and y.
    entity->set_top_left(QPoint(0, 0));
  }
  else {
    entity->name = QString::fromStdString(map.get_internal_entity(index).get_name());
  }

  for (const auto& kvp : entity->get_entity().get_fields()) {
    QString key = QString::fromStdString(kvp.first);
    QVariant value = entity->get_field(key);
    entity->notify_field_changed(key, value);
  }

  return EntityModelPtr(entity);
}

/**
 * @brief Returns the map this entity belongs to.
 * @return The map.
 */
const MapModel& EntityModel::get_map() const {
  return *map;
}

/**
 * @overload
 *
 * Non-const version.
 */
MapModel& EntityModel::get_map() {
  return *map;
}

/**
 * @brief Returns the index of this entity on the map.
 * @return The index, or an invalid index if the entity is not on the map.
 */
EntityIndex EntityModel::get_index() const {
  return index;
}

/**
 * @brief Returns whether this entity is present on the map.
 * @return @c true if the entity is on the map.
 */
bool EntityModel::is_on_map() const {
  return get_index().is_valid();
}

/**
 * @brief Notifies this entity that it was added to the map.
 * @param The index where it was added.
 */
void EntityModel::added_to_map(const EntityIndex& index) {

  if (!index.is_valid()) {
    qCritical() << MapModel::tr("Missing index of entity added");
    return;
  }

  if (this->index.is_valid()) {
    qCritical() << MapModel::tr("This entity already has a map index");
    return;
  }

  set_layer(index.layer);
  this->index = index;
  map->get_internal_entity(index) = stub;  // Copy the data.
  bool name_ok = map->set_entity_name(index, name);
  Q_ASSERT(name_ok);
  Q_UNUSED(name_ok);
}

/**
 * @brief Notifies this entity that it is about to be removed from the map.
 *
 * It can still be added again later.
 */
void EntityModel::about_to_be_removed_from_map() {

  if (!index.is_valid()) {
    qCritical() << MapModel::tr("Entity being removed is not on the map");
    return;
  }

  stub = map->get_internal_entity(index);  // Save the data.
  index = EntityIndex();  // Set an invalid index.
}

/**
 * @brief Notifies this entity that its index on the map has changed.
 * @param The new index.
 */
void EntityModel::index_changed(const EntityIndex& index) {

  if (!this->index.is_valid()) {
    qCritical() << MapModel::tr("This entity is not on the map");
    return;
  }

  if (!index.is_valid()) {
    qCritical() << MapModel::tr("Missing new index of entity");
    return;
  }

  this->index = index;
  set_layer(index.layer);
}

/**
 * @brief Returns the Solarus entity wrapped.
 * @return The entity from the map, or the entity stub if it does not belong
 * to the map yet.
 */
const Solarus::EntityData& EntityModel::get_entity() const {

  if (index.is_valid()) {
    return map->get_internal_entity(index);
  }
  return stub;
}

/**
 * @overload
 *
 * Non-const version.
 */
Solarus::EntityData& EntityModel::get_entity() {

  if (index.is_valid()) {
    return map->get_internal_entity(index);
  }
  return stub;
}

/**
 * @brief Changes the entity data wrapped.
 * @param entity The new data.
 */
void EntityModel::set_entity(const EntityData& entity) {

  Q_ASSERT(!is_on_map());
  Q_ASSERT(entity.get_type() == get_type());

  this->stub = entity;

  if (entity.is_integer("width") && entity.is_integer("height")) {
    set_size(QSize(entity.get_integer("width"), entity.get_integer("height")));
  }

  for (const auto& kvp : entity.get_fields()) {
    QString key = QString::fromStdString(kvp.first);
    QVariant value = get_field(key);
    notify_field_changed(key, value);
  }
}

/**
 * @brief Returns the id of the tileset of the map.
 * @return The tileset id or an empty string if no tileset is set.
 */
QString EntityModel::get_tileset_id() const {
  return get_map().get_tileset_id();
}

/**
 * @brief Returns the tileset of the map.
 * @return The tileset or nullptr if no tileset is set.
 */
const TilesetModel* EntityModel::get_tileset() const {
  return get_map().get_tileset_model();
}

/**
 * @brief Returns the quest the map of this entity belongs to.
 * @return The quest.
 */
const Quest& EntityModel::get_quest() const {
  return get_map().get_quest();
}

/**
 * @brief Returns the quest resources.
 * @return The resources.
 */
const QuestResources& EntityModel::get_resources() const {
  return get_quest().get_resources();
}

/**
 * @brief Returns the type of this entity.
 * @return The entity type.
 */
EntityType EntityModel::get_type() const {
  return get_entity().get_type();
}

/**
 * @brief Returns the Lua type name of this entity.
 * @return The entity type name.
*/
QString EntityModel::get_type_name() const {
  return QString::fromStdString(get_entity().get_type_name());
}

/**
 * @brief Returns whether this entity has a name.
 * @return @c true if the name is not empty.
 */
bool EntityModel::has_name() const {
  return !name.isEmpty();
}

/**
 * @brief Returns the name of this entity.
 * @return The name or an empty string if the entity has no name.
 */
QString EntityModel::get_name() const {
  return name;
}

/**
 * @brief Sets the name of this entity.
 * @param name The name to set.
 */
void EntityModel::set_name(const QString& name) {

  this->name = name;
  get_entity().set_name(name.toStdString());
}

/**
 * @brief Renames this entity if necessary so that its name is unique on the map.
 *
 * This function should be called before adding the entity on the map.
 */
void EntityModel::ensure_name_unique() {

  QString name = get_name();
  if (name.isEmpty()) {
    // No name is always okay.
    return;
  }

  EntityIndex index = map->find_entity_by_name(name);
  if (!index.is_valid()) {
    // The name is not used.
    return;
  }

  if (index == get_index()) {
    // The name is used but only by this entity.
    return;
  }

  int counter = 2;
  QStringList words = name.split('_');
  if (words.size() == 1) {
    name = name + "_";
  }
  else {
    QString last_word = words.last();
    bool is_int = false;
    counter = last_word.toInt(&is_int);
    if (!is_int) {
      counter = 2;
      name = name + "_";
    }
    else {
      words.removeLast();
      name = "";
      for (QString word : words) {
        name = name + word + "_";
      }
    }
  }

  QString counter_string = QString::number(counter);
  while (map->entity_name_exists(name + counter_string)) {
    ++counter;
    counter_string = QString::number(counter);
  }
  name = name + counter_string;
  set_name(name);
}

/**
 * @brief Returns the layer where this entity is on the map.
 * @return The layer.
 */
Layer EntityModel::get_layer() const {
  return get_entity().get_layer();
}

/**
 * @brief Sets the layer where this entity is on the map.
 * @param layer The layer.
 * @warning This does not update the index of the entity.
 */
void EntityModel::set_layer(Layer layer) {
  get_entity().set_layer(layer);
}

/**
 * @brief Returns the coordinates of this entity on the map.
 * @return The coordinates of the entity's origin point.
 */
QPoint EntityModel::get_xy() const {
  return Point::to_qpoint(get_entity().get_xy());
}

/**
 * @brief Sets the coordinates of this entity on the map.
 * @param xy The coordinates of the entity's origin point.
 */
void EntityModel::set_xy(const QPoint& xy) {
  get_entity().set_xy(Point::to_solarus_point(xy));
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
 * @brief Returns the coordinates of the bottom-right corner of this entity.
 * @return The coordinates of the entity's bottom-right corner.
 */
QPoint EntityModel::get_bottom_right() const {

  return get_top_left() + QPoint(get_width(), get_height());
}

/**
 * @brief Sets the coordinates of the bottom-right corner of this entity.
 * @param bottom_right The coordinates of the entity's bottom-right corner.
 */
void EntityModel::set_bottom_right(const QPoint& bottom_right) {

  set_top_left(bottom_right - QPoint(get_width(), get_height()));
}

/**
 * @brief Returns the coordinates of the center point of this entity.
 * @return The coordinates of the entity's center point.
 */
QPoint EntityModel::get_center() const {
  return get_top_left() + QPoint(get_width() / 2, get_height() / 2);
}

/**
 * @brief Sets the coordinates of the center point of this entity.
 * @param center The coordinates of the entity's center point.
 */
void EntityModel::set_center(const QPoint& center) {

  QPoint top_left = center - QPoint(get_width() / 2, get_height() / 2);
  set_top_left(top_left);
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

  // Set the size specific to the editor.
  this->size = size;

  // If there is size field in the map file, change it as well.
  if (has_field("width") && has_field("height")) {
    set_field("width", size.width());
    set_field("height", size.height());
  }
}

/**
 * @brief Returns the bounding box of this entity.
 * @return The bounding box.
 */
QRect EntityModel::get_bounding_box() const {
  return QRect(get_top_left(), get_size());
}

/**
 * @brief Returns whether the entity has a preferred initial layer when added to the map.
 * @return @c true if there is a preferred layer.
 */
bool EntityModel::get_has_preferred_layer() const {
  return has_preferred_layer;
}

/**
 * @brief Sets whether the entity has a preferred initial layer when added to the map.
 * @param has_preferred_layer @c true if there is a preferred layer.
 */
void EntityModel::set_has_preferred_layer(bool has_preferred_layer) {
  this->has_preferred_layer = has_preferred_layer;
}

/**
 * @brief Returns the preferred initial layer if there is one.
 * @return The preferred layer.
 */
Layer EntityModel::get_preferred_layer() const {
  return preferred_layer;
}

/**
 * @brief Sets the preferred initial layer if there is one.
 *
 * This function only makes sense if get_has_preferred_layer() is @c true.
 *
 * @param preferred_layer The new preferred layer.
 */
void EntityModel::set_preferred_layer(Layer preferred_layer) {
  this->preferred_layer = preferred_layer;
}

/**
 * @brief Returns whether this entity has a "direction" field.
 * @return @c true if a direction property exists.
 */
bool EntityModel::has_direction_property() const {

  return has_field("direction");
}

/**
 * @brief Returns the direction of this entity.
 *
 * If the entity has no direction field, or if the direction is unset,
 * returns -1.
 *
 * @return The direction or -1.
 */
int EntityModel::get_direction() const {

  if (!has_direction_property()) {
    return -1;
  }

  bool ok;
  int direction = get_field("direction").toInt(&ok);
  if (!ok) {
    return -1;
  }

  return direction;
}

/**
 * @brief Sets the direction of this entity.
 *
 * Nothing is done if the entity has no direction field.
 *
 * @param direction The new direction or -1 to set no direction.
 */
void EntityModel::set_direction(int direction) {

  if (!has_direction_property()) {
    return;
  }

  set_field("direction", direction);
}

/**
 * @brief Returns if the entity has a field with the given name.
 * @param key Key of the field to get.
 * @return @c true if there is this field eixsts.
 */
bool EntityModel::has_field(const QString& key) const {

  std::string std_key = key.toStdString();
  return get_entity().get_field(std_key).value_type != EntityData::EntityFieldType::NIL;
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

  const Solarus::EntityData& entity = get_entity();
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
 * @brief Sets the value of a field of this entity.
 * @param key Key of the field to set.
 * @param value The corresponding value.
 * It must have the expected type for the field.
 */
void EntityModel::set_field(const QString& key, const QVariant& value) {

  std::string std_key = key.toStdString();

  Solarus::EntityData& entity = get_entity();
  if (entity.is_string(std_key)) {
    entity.set_string(std_key, value.toString().toStdString());
  }

  else if (entity.is_integer(std_key)) {
    bool ok = false;
    entity.set_integer(std_key, value.toInt(&ok));
  }

  else if (entity.is_boolean(std_key)) {
    entity.set_boolean(std_key, value.toBool());
  }

  notify_field_changed(key, value);
}

/**
 * @brief Returns the string representing this entity in map files or scripts.
 * @return The string form of this entity.
 * Returns an empty string in case of error.
 */
QString EntityModel::to_string() const {

  std::string buffer;
  if (!get_entity().export_to_buffer(buffer)) {
    return QString();
  }
  return QString::fromStdString(buffer);
}

/**
 * @brief Returns whether this entity can be resized.
 * @return @c true if the entity is resizable.
 */
bool EntityModel::is_resizable() const {

  return get_resize_mode() != ResizeMode::NONE;
}

/**
 * @brief Sets whether this entity can be resized.
 *
 * If you need finer control of how it should be able to be resized,
 * use set_resize_mode().
 *
 * @param @c true to make the entity resizable (with mode
 * Resizable::MULTI_DIMENSION_ALL, the most usual one), @c false to forbid
 * resizing.
 */
void EntityModel::set_resizable(bool resizable) {

  set_resize_mode(resizable ? ResizeMode::MULTI_DIMENSION_ALL : ResizeMode::NONE);
}

/**
 * @brief Returns how this entity can be resized.
 * @return The resize mode.
 */
ResizeMode EntityModel::get_resize_mode() const {
  return resize_mode;
}

/**
 * @brief Sets how this entity can be resized.
 * @param resize_mode The resize mode.
 */
void EntityModel::set_resize_mode(ResizeMode resize_mode) {
  this->resize_mode = resize_mode;
}

/**
 * @brief Returns the base size to consider when resizing this entity.
 *
 * In all modes, dimensions that can be resized remain multiple of the ones
 * of this base size.
 *
 * In mode ResizeMode::HORIZONTAL_ONLY, the width remains equal to the
 * width of this base size.
 * In mode ResizeMode::VERTICAL_ONLY, the height remains equal to the
 * height of this base size.
 * In mode ResizeMode::SINGLE_DIMENSION, at least one of both dimensions
 * has to be equal to the one of this base size.
 *
 * @return The base size.
 */
QSize EntityModel::get_base_size() const {
  return base_size;
}

/**
 * @brief Sets the base size to consider when resizing with modes where a
 * direction is fixed.
 *
 * See get_base_size() for details about how the base size exactly works.
 *
 * @return The base size.
 */
void EntityModel::set_base_size(const QSize& base_size) {
  this->base_size = base_size;
}

/**
 * @brief This function is called when a field of this entity was just set.
 *
 * This function is also called at initialization time when creating an entity
 * model from data.
 * Subclasses can reimplement it to react to the change.
 *
 * @param key Key of the field.
 * @param value The new value.
 */
void EntityModel::notify_field_changed(const QString& key, const QVariant& value) {

  // Nothing done by default.
  Q_UNUSED(key);
  Q_UNUSED(value);
}

/**
 * @brief Returns the sprite description of the entity.
 * @return The sprite description that was set with set_draw_sprite_info().
 */
const EntityModel::DrawSpriteInfo& EntityModel::get_draw_sprite_info() const {
  return this->draw_sprite_info;
}

/**
 * @brief Sets how to draw this entity as a sprite.
 * @param draw_sprite_info Description of the sprite to draw.
 */
void EntityModel::set_draw_sprite_info(const DrawSpriteInfo& draw_sprite_info) {

  this->draw_sprite_info = draw_sprite_info;

  sprite_model = nullptr;
  sprite_image = QPixmap();
}

/**
 * @brief Returns the shape description of the entity.
 * @return The shape description that was set with set_draw_shape_info().
 */
const EntityModel::DrawShapeInfo& EntityModel::get_draw_shape_info() const {
  return this->draw_shape_info;
}

/**
 * @brief Sets how to draw this entity as a shape.
 *
 * Call this function if you want your entity to be drawn as a shape
 * when it has no sprite.
 *
 * @param draw_shape_info Description of the shape to draw.
 */
void EntityModel::set_draw_shape_info(const DrawShapeInfo& draw_shape_info) {
  this->draw_shape_info = draw_shape_info;
}

/**
 * @brief Returns the image description of the entity.
 * @return The image description that was set with set_draw_image_info().
 */
const EntityModel::DrawImageInfo& EntityModel::get_draw_image_info() const {
  return this->draw_image_info;
}

/**
 * @brief Sets how to draw this entity as an image.
 *
 * Call this function if you want your entity to be drawn as a fixed image
 * when it has no sprite and no shape info.
 *
 * @param draw_shape_info Description of the image to draw.
 */
void EntityModel::set_draw_image_info(const DrawImageInfo& draw_image_info) {
  this->draw_image_info = draw_image_info;
}

/**
 * @brief Draws this entity.
 *
 * The default implementation does the following.
 * - If the entity type has a "sprite" field, draws this sprite if the value
 *   is set to a valid sprite.
 * - Otherwise, if a sprite was set by calling set_sprite_id(), draws this
 *   sprite if it is a valid one.
 * - Otherwise, if a shape description was set by calling set_draw_shape_info(),
 *   this shape is drawn.
 * - Otherwise, if an image description was set by calling set_draw_image_info(),
 *   this image is drawn.
 * - Otherwise, draws an icon representing the type of entity.
 *
 * @param painter The painter to draw.
 */
void EntityModel::draw(QPainter& painter) const {

  if (draw_as_sprite(painter)) {
    return;
  }

  if (draw_as_shape(painter)) {
    return;
  }

  if (draw_as_image(painter)) {
    return;
  }

  draw_as_icon(painter);
}

/**
 * @brief Attempts to draw this entity using its sprite if any.
 *
 * Its sprite is the one from the sprite field value if any,
 * and otherwise the one that was set by set_sprite_id().
 *
 * @param painter The painter to draw.
 * @return @c true if the sprite was successfully drawn.
 */
bool EntityModel::draw_as_sprite(QPainter& painter) const {

  // Try to draw the sprite from the sprite field if any.
  const QString& sprite_field_value = get_field("sprite").toString();
  if (draw_as_sprite(painter, sprite_field_value, "")) {
    return true;
  }

  // Otherwise try to draw the one that was set by set_draw_sprite_info().
  return draw_as_sprite(painter, draw_sprite_info.sprite_id, draw_sprite_info.animation);
}

/**
 * @brief Attempts to draw this entity using the specified sprite.
 * @param painter The painter to draw.
 * @param sprite_id Sprite to use.
 * @param animation Animation to use in this sprite.
 * If it does not exists, the default animation will be used.
 * @return @c true if the sprite was successfully drawn.
 */
bool EntityModel::draw_as_sprite(QPainter& painter, const QString& sprite_id, const QString& animation) const {

  if (sprite_id.isEmpty()) {
    // No sprite sheet.
    return false;
  }

  if (!get_resources().exists(ResourceType::SPRITE, sprite_id)) {
    // The sprite does not exist in the quest.
    return false;
  }

  if (sprite_model == nullptr ||
      sprite_model->get_sprite_id() != sprite_id) {
    sprite_model = std::unique_ptr<SpriteModel>(new SpriteModel(get_quest(), sprite_id));
    sprite_model->set_tileset_id(get_tileset_id());
  }

  SpriteModel::Index index(animation, 0);
  if (!sprite_model->animation_exists(index)) {
    // Try the default animation.
    index.animation_name = sprite_model->get_default_animation_name();
    if (!sprite_model->animation_exists(index)) {
      // No animation.
      return false;
    }
  }

  bool has_direction_field;
  int direction = get_field("direction").toInt(&has_direction_field);
  if (!has_direction_field) {
    direction = 0;
  }
  index.direction_nb = direction;

  if (!sprite_model->direction_exists(index)) {
    index.direction_nb = 0;
  }
  if (!sprite_model->direction_exists(index)) {
    // No direction.
    return false;
  }

  // Lazily create the image.
  if (sprite_image.isNull()) {
    sprite_image = sprite_model->get_direction_frame(index);
    if (sprite_image.isNull()) {
      // The sprite model did not give a valid image.
      return false;
    }
  }

  QPoint dst_top_left = get_origin() - sprite_model->get_direction_origin(index);
  if (draw_sprite_info.tiled) {
    painter.drawTiledPixmap(QRect(dst_top_left, get_size()), sprite_image);
  }
  else {
    painter.drawPixmap(QRect(dst_top_left, sprite_image.size()), sprite_image);
  }
  return true;
}

/**
 * @brief Draws this entity using its shape description if any.
 * @param painter The painter to draw.
 * @return @c true if there was a valid shape to draw.
 */
bool EntityModel::draw_as_shape(QPainter& painter) const {

  if (!draw_shape_info.enabled) {
    // The entity does not want to be drawn as a shape.
    return false;
  }

  // Background color.
  if (draw_shape_info.background_color.isValid()) {
    painter.fillRect(0, 0, get_width(), get_height(), draw_shape_info.background_color);
  }

  // Pixmap.
  if (!draw_shape_info.pixmap.isNull()) {

    // We will draw the pixmap with a double resolution.
    painter.scale(0.5, 0.5);

    const QPixmap& pixmap = draw_shape_info.pixmap;
    if (draw_shape_info.tiled_pixmap) {
      // Repeat the pixmap pattern.
      painter.drawTiledPixmap(0, 0, get_width() * 2, get_height() * 2, pixmap);
    }
    else {
      // Draw the pixmap centered in the entity.
      int x = get_width() - pixmap.width() / 2;  // Actually get_width() * 2 / 2 - pixmap.width() / 2
                                                 // because we want the double size.
      int y = get_height() - pixmap.height() / 2;
      painter.drawPixmap(x, y, pixmap.width(), pixmap.height(), pixmap);
    }

    painter.scale(2, 2);
  }

  // Border.
  if (draw_shape_info.between_border_color.isValid()) {
    GuiTools::draw_rectangle_border_double(
          painter,
          QRect(0, 0, get_width(), get_height()),
          draw_shape_info.between_border_color);
  }

  return true;
}

/**
 * @brief Draws this entity using its image description if any.
 * @param painter The painter to draw.
 * @return @c true if there was a valid image to draw.
 */
bool EntityModel::draw_as_image(QPainter& painter) const {

  // First try to draw an image specific to the current direction.
  int direction = get_direction();
  if (direction != -1 &&
      direction < draw_image_info.images_by_direction.size()) {
    if (draw_as_image(painter, draw_image_info.images_by_direction.at(direction))) {
      return true;
    }
  }

  // No direction-specific image was set, or the entity has no direction:
  // use the direction-independent image if one was set.
  return draw_as_image(painter, draw_image_info.image_no_direction);
}

/**
 * @brief Draws this entity using the specified image region.
 * @param painter The painter to draw.
 * @param sub_image Region of image to draw.
 * @return @c true if the image was drawn.
 */
bool EntityModel::draw_as_image(QPainter& painter, const SubImage& sub_image) const {

  if (sub_image.file_name.isEmpty()) {
    return false;
  }

  if (sub_image.pixmap.isNull()) {
    // Lazily load the image.
    sub_image.pixmap = QPixmap(sub_image.file_name).copy(sub_image.src_rect);
    if (sub_image.pixmap.isNull()) {
      return false;
    }
  }

  const double scale = draw_image_info.scale;
  painter.scale(1.0 / scale, 1.0 / scale);
  painter.drawTiledPixmap(0, 0, (int) (get_width() * scale), (int) (get_height() * scale),
                          sub_image.pixmap);
  painter.scale(scale, scale);
  return true;
}

/**
 * @brief Draws this entity using the icon of its entity type.
 * @param painter The painter to draw.
 * @return @c true if the icon was successfully drawn.
 */
bool EntityModel::draw_as_icon(QPainter& painter) const {

  if (icon.isNull()) {
    // Lazily create the icon.
    icon = QPixmap(QString(":/images/entity_%1.png").arg(get_type_name()));
  }

  // We draw a 32x32 icon on a 16x16 square.
  // It will have a better resolution than tiles and sprites.
  painter.scale(0.5, 0.5);
  painter.drawTiledPixmap(0, 0, get_width() * 2, get_height() * 2, icon);
  painter.scale(2, 2);

  return true;
}
