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
#include "entities/entity_model.h"
#include "editor_exception.h"
#include "map_model.h"
#include "quest.h"
#include "point.h"
#include "size.h"
#include "tileset_model.h"
#include <QDebug>
#include <QIcon>

/**
 * @brief Creates a map model.
 * @param quest The quest.
 * @param map_id Id of the map to manage.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
MapModel::MapModel(
    Quest& quest,
    const QString& map_id,
    QObject* parent) :
  QObject(parent),
  quest(quest),
  map_id(map_id),
  tileset_model(nullptr) {

  // Load the map data file.
  QString path = quest.get_map_data_file_path(map_id);

  if (!map.import_from_file(path.toStdString())) {
    throw EditorException(tr("Cannot open map data file '%1'").arg(path));
  }

  // Create the tileset object.
  QString tileset_id = get_tileset_id();
  if (!tileset_id.isEmpty()) {
    tileset_model = new TilesetModel(quest, tileset_id, this);
  }

  for (int i = 0; i < Layer::LAYER_NB; ++i) {
    Layer layer = static_cast<Layer>(i);
    for (int j = 0; j < get_num_entities(layer); ++j) {
      EntityIndex index = { layer, j };
      entities[i].emplace_back(EntityModel::create(*this, index));
    }
  }
}

/**
 * @brief Returns the quest.
 */
const Quest& MapModel::get_quest() const {
  return quest;
}

/**
 * @brief Returns the quest.
 *
 * Non-const version.
 */
Quest& MapModel::get_quest() {
  return quest;
}

/**
 * @brief Returns the id of the map managed by this model.
 * @return The map id.
 */
QString MapModel::get_map_id() const {
  return map_id;
}

/**
 * @brief Saves the map to its data file.
 * @throws EditorException If the file could not be saved.
 */
void MapModel::save() const {

  QString path = quest.get_map_data_file_path(map_id);

  if (!map.export_to_file(path.toStdString())) {
    throw EditorException(tr("Cannot save map data file '%1'").arg(path));
  }
}

/**
 * @brief Returns the size of the map.
 * @return The size of the map in pixels.
 */
QSize MapModel::get_size() const {

  return Size::to_qsize(map.get_size());
}

/**
 * @brief Sets the size of this map.
 *
 * Emits size_changed() if there is a change.
 *
 * @param size The size to set.
 */
void MapModel::set_size(const QSize& size) {

  const Solarus::Size solarus_size = Size::to_solarus_size(size);
  if (solarus_size == map.get_size()) {
    return;
  }

  map.set_size(solarus_size);
  emit size_changed(size);
}

/**
 * @brief Returns whether a world is set.
 * @return @c true if this map is in a world.
 */
bool MapModel::has_world() const {

  return map.has_world();
}

/**
 * @brief Returns the world this maps belongs to.
 * @return The world name or an empty string.
 */
QString MapModel::get_world() const {

  return QString::fromStdString(map.get_world());
}

/**
 * @brief Sets the world of this map.
 *
 * Emits world_changed() if there is a change.
 *
 * @param world The world name or an empty string.
 */
void MapModel::set_world(const QString& world) {

  const std::string& std_world = world.toStdString();
  if (std_world == map.get_world()) {
    return;
  }

  map.set_world(std_world);
  emit world_changed(world);
}

/**
 * @brief Returns whether a floor is set.
 * @return @c true if this map has a floor value.
 */
bool MapModel::has_floor() const {

  return map.has_floor();
}

/**
 * @brief Returns the floor of this map.
 * @return The floor or NO_FLOOR.
 */
int MapModel::get_floor() const {

  return map.get_floor();
}

/**
 * @brief Sets the floor of this map.
 *
 * Emits floor_changed() if there is a change.
 *
 * @param floor The floor or NO_FLOOR.
 */
void MapModel::set_floor(int floor) {

  if (floor == map.get_floor()) {
    return;
  }

  map.set_floor(floor);
  emit floor_changed(floor);
}

/**
 * @brief Returns the location of this map in its world.
 * @return The location.
 */
QPoint MapModel::get_location() const {

  return Point::to_qpoint(map.get_location());
}

/**
 * @brief Sets the location of this map in its world.
 *
 * Emits location_changed() if there is a change.
 *
 * @return The location.
 */
void MapModel::set_location(const QPoint& location) {

  const Solarus::Point solarus_location = Point::to_solarus_point(location);
  if (solarus_location == map.get_location()) {
    return;
  }

  map.set_location(solarus_location);
  emit location_changed(location);
}

/**
 * @brief Returns the id of the tileset of this map.
 * @return The tileset id. Returns an empty string if no tileset is set.
 */
QString MapModel::get_tileset_id() const {
  return QString::fromStdString(map.get_tileset_id());
}

/**
 * @brief Sets the tileset of this map.
 *
 * Emits tileset_id_changed() if there is a change.
 *
 * @param tileset_id The tileset id.
 */
void MapModel::set_tileset_id(const QString& tileset_id) {

  const std::string std_tileset_id = tileset_id.toStdString();
  if (std_tileset_id == map.get_tileset_id()) {
    return;
  }
  map.set_tileset_id(std_tileset_id);

  if (!tileset_id.isEmpty()) {
    tileset_model = new TilesetModel(quest, tileset_id, this);
  }

  emit tileset_id_changed(tileset_id);
}

/**
 * @brief Returns the tileset of this map.
 * @return The tileset. Returns nullptr if no tileset is set.
 */
TilesetModel* MapModel::get_tileset_model() const {
  return tileset_model;
}

/**
 * @brief Returns the id of the music of this map.
 * @return The music id or "none" or "same".
 */
QString MapModel::get_music_id() const {
  return QString::fromStdString(map.get_music_id());
}

/**
 * @brief Sets the music of this map.
 *
 * Emits music_id_changed() if there is a change.
 *
 * @param music_id The music id or "none" or "same".
 */
void MapModel::set_music_id(const QString& music_id) {

  const std::string std_music_id = music_id.toStdString();
  if (std_music_id == map.get_music_id()) {
    return;
  }
  map.set_music_id(std_music_id);
  emit music_id_changed(music_id);
}

/**
 * @brief Returns the total number of entities on the map.
 * @return The number of entities.
 */
int MapModel::get_num_entities() const {
  return map.get_num_entities();
}

/**
 * @brief Returns the number of entities on a layer of the map.
 * @param layer A layer.
 * @return The number of entities on that layer.
 */
int MapModel::get_num_entities(Layer layer) const {
  return map.get_num_entities(layer);
}

/**
 * @brief Returns whether there is an entity at the given index.
 * @param index An index.
 * @return @c true if there is an entity at this index.
 */
bool MapModel::entity_exists(const EntityIndex& index) const {
  return map.entity_exists(index);
}

/**
 * @brief Returns the model of entity at the given index.
 * @param index A map entity index.
 * @return The corresponding entity model.
 */
const EntityModel& MapModel::get_entity(const EntityIndex& index) const {
  return *entities[index.layer].at(index.index);
}

/**
 * @overload
 *
 * Non-const version.
 */
EntityModel& MapModel::get_entity(const EntityIndex& index) {
  return *entities[index.layer].at(index.index);
}

/**
 * @brief Returns the underlying Solarus entity data at the given index.
 *
 * External classes should not directly use this function, they should use the
 * other functions to have Qt-friendly types and signals.
 *
 * @param index A map entity index.
 * @return The entity at this index.
 */
const Solarus::EntityData& MapModel::get_internal_entity(const EntityIndex& index) const {
  return map.get_entity(index);
}

/**
 * @overload
 *
 * Non-const version.
 */
Solarus::EntityData& MapModel::get_internal_entity(const EntityIndex& index) {
  return map.get_entity(index);
}

/**
 * @brief Returns the type of an entity.
 * @param index Index of a map entity.
 * @return The corresponding type.
 * Returns a default-constructed type if there is no entity at this index.
 */
EntityType MapModel::get_entity_type(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    // No such entity.
    return EntityType();
  }

  return get_entity(index).get_type();
}

/**
 * @brief Returns the Lua type name of an entity.
 * @param index Index of a map entity.
 * @return The corresponding type name.
 * Returns an empty string if there is no entity at this index.
 */
QString MapModel::get_entity_type_name(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    // No such entity.
    return QString();
  }

  return get_entity(index).get_type_name();
}

/**
 * @brief Returns the layer where an entity is on the map.
 * @param index Index of a map entity.
 * @return The layer.
 * Returns a default-constructed layer if there is no entity at this index.
 */
Layer MapModel::get_entity_layer(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    // No such entity.
    return Layer();
  }

  return get_entity(index).get_layer();
}

/**
 * @brief Returns the coordinates of an entity on the map.
 * @param index Index of the entity to get.
 * @return The coordinates of the entity's origin point.
 * Returns @c QPoint() if there is no entity with this index.
 */
QPoint MapModel::get_entity_xy(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return QPoint();
  }

  return get_entity(index).get_xy();
}

/**
 * @brief Sets the coordinates of an entity on the map.
 *
 * Emits entity_xy_changed() if there is a change.
 *
 * @param index Index of the entity to change.
 * @param xy The new coordinates of the entity's origin point.
 * Does nothing if there is no entity with this index.
 */
void MapModel::set_entity_xy(const EntityIndex& index, const QPoint& xy) {

  if (!entity_exists(index)) {
    return;
  }

  EntityModel& entity = get_entity(index);

  if (xy == entity.get_xy()) {
    return;
  }

  entity.set_xy(xy);
  emit entity_xy_changed(index, xy);
}

/**
 * @brief Applies a translation to an entity on the map.
 * @param index Index of the entity to change.
 * @param xy The coordinates to add.
 * Does nothing if there is no entity with this index.
 */
void MapModel::add_entity_xy(const EntityIndex& index, const QPoint& translation) {

  set_entity_xy(index, get_entity_xy(index) + translation);
}

/**
 * @brief Returns the coordinates of the upper-left corner of an entity.
 * @param index Index of the entity to get.
 * @return The coordinates of the entity's upper-left corner.
 * Returns @c QPoint() if there is no entity with this index.
 */
QPoint MapModel::get_entity_top_left(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return QPoint();
  }

  return get_entity(index).get_top_left();
}

/**
 * @brief Returns the origin point of an entity on the map.
 * @param index Index of the entity to get.
 * @return The entity's origin point relative to its upper-left corner.
 * Returns @c QPoint() if there is no entity with this index.
 */
QPoint MapModel::get_entity_origin(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return QPoint();
  }

  return get_entity(index).get_origin();
}

/**
 * @brief Returns the size of an entity on the map.
 * @param index Index of the entity to get.
 * @return The size of this entity.
 * Returns @c QSize() if there is no entity with this index.
 */
QSize MapModel::get_entity_size(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return QSize();
  }

  return get_entity(index).get_size();
}

/**
 * @brief Returns the bounding box of an entity for the editor.
 * @param index Index of the entity to get.
 * @return The bounding box, or an empty rectangle if there is no entity
 * with this index.
 */
QRect MapModel::get_entity_bounding_box(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return QRect();
  }

  return get_entity(index).get_bounding_box();
}

/**
 * @brief Adds entities to the map.
 *
 * They should have a invalid index before the call.
 * After this call, they belong to this object.
 * Emits entity_added() for each entity.
 *
 * @param entities The entities to add.
 * @param view_settings View settings to use to determine the layer.
 * @return The created indexes.
 */
QList<EntityIndex> MapModel::add_entities(EntityModels& entities, const ViewSettings& view_settings) {

  if (entities.empty()) {
    return QList<EntityIndex>();
  }

  // Sanity check.
  for (const std::unique_ptr<EntityModel>& entity : entities) {
    if (entity->is_on_map()) {
      qCritical() << "This entity is already on the map";
      return QList<EntityIndex>();
    }
  }

  // Determine the best layer.
  Layer layer = Layer::LAYER_LOW;
  Q_UNUSED(view_settings);  // TODO determine the best layer

  // Add each entity.
  QList<EntityIndex> indexes;
  for (std::unique_ptr<EntityModel>& entity : entities) {

    // Add the entity on the Solarus side.
    entity->set_layer(layer);
    EntityIndex index = map.add_entity(entity->get_entity());
    if (!index.is_valid()) {
      qCritical() << "Failed to add entity";
      continue;
    }

    // Update the entity model and the entity list in the map editor.
    entity->set_index(index);
    this->entities[layer].emplace_back(std::move(entity));

    // Provide the newly created index to the caller.
    indexes.push_back(index);

    emit entity_added(index);
  }

  return indexes;
}
