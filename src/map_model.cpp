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
#include <QIcon>
#include <QSet>

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

  if (!index.is_valid()) {
    return false;
  }

  bool exists_in_solarus = map.entity_exists(index);
  bool exists_in_model = (index.order >= 0 && index.order < (int) entities[index.layer].size());
  Q_ASSERT(exists_in_model == exists_in_solarus);

  return exists_in_model;
}

/**
 * @brief Returns the model of entity at the given index.
 * @param index A map entity index.
 * @return The corresponding entity model.
 */
const EntityModel& MapModel::get_entity(const EntityIndex& index) const {
  return *entities[index.layer].at(index.order);
}

/**
 * @overload
 *
 * Non-const version.
 */
EntityModel& MapModel::get_entity(const EntityIndex& index) {
  return *entities[index.layer].at(index.order);
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
 * @brief Returns the name of an entity.
 * @param index Index of a map entity.
 * @return The name or an empty string if the entity has no name.
 * Returns an empty string if there is no entity at this index.
 */
QString MapModel::get_entity_name(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    // No such entity.
    return QString();
  }

  return get_entity(index).get_name();
}

/**
 * @brief Sets the name of an entity.
 *
 * Emits entity_name_changed() if there is a change.
 *
 * @param index Index of a map entity.
 * @param name The new name or an empty string to set no name.
 * @return @c true in case of success, @c false if the name was already in use.
 * Does nothing and returns @c false if there is no entity with this index.
 */
bool MapModel::set_entity_name(const EntityIndex& index, const QString& name) {

  if (!entity_exists(index)) {
    // No such entity.
    return false;
  }

  // Make the change on the engine side.
  if (!map.set_entity_name(index, name.toStdString())) {
    return false;
  }

  // Update the entity from the editor side.
  get_entity(index).set_name(name);

  emit entity_name_changed(name);
  return true;
}

/**
 * @brief Returns whether there exists an entity with the given name.
 * @param name The name to check
 * @return @c true if this name is already in use.
 */
bool MapModel::entity_name_exists(const QString& name) const {

  return map.entity_exists(name.toStdString());
}

/**
 * @brief Find the entity with the specified name.
 * @param name The name to get.
 * @return The index of corresponding entity or an invalid index.
 */
EntityIndex MapModel::find_entity_by_name(const QString& name) const {

  return map.get_entity_index(name.toStdString());
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
 *
 * Emits entity_xy_changed() if there is a change.
 *
 * @param index Index of the entity to change.
 * @param xy The coordinates to add.
 * Does nothing if there is no entity with this index.
 */
void MapModel::add_entity_xy(const EntityIndex& index, const QPoint& translation) {

  if (!entity_exists(index)) {
    return;
  }

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
 * @brief Sets the coordinates of the upper-left corner of an entity.
 *
 * Emits entity_xy_changed() if there is a change.
 *
 * @param index Index of the entity to change.
 * @param top_left The new coordinates of the entity's upper-left corner.
 * Does nothing if there is no entity with this index.
 */
void MapModel::set_entity_top_left(const EntityIndex& index, const QPoint& top_left) {

  if (!entity_exists(index)) {
    return;
  }

  set_entity_xy(index, top_left + get_entity_origin(index));
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
 * @brief Sets the size of an entity on the map.
 *
 * Emits entity_size_changed() if there is a change.
 *
 * @param index Index of the entity to set.
 * @param size The new size of this entity.
 * Does nothing if there is no entity with this index.
 */
void MapModel::set_entity_size(const EntityIndex& index, const QSize& size) {

  if (!entity_exists(index)) {
    return;
  }

  EntityModel& entity = get_entity(index);

  if (size == entity.get_size()) {
    // No change.
    return;
  }

  entity.set_size(size);
  emit entity_size_changed(index, size);
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
 * @brief Sets the position and size of an entity for the editor.
 *
 * Emits entity_xy_changed() if the coordinates change,
 * and entity_size_changed() if the size changes.
 *
 * @param index Index of the entity to change.
 * @param bounding_box The new bounding box.
 * Does nothing if there is no entity with this index.
 */
void MapModel::set_entity_bounding_box(const EntityIndex& index, const QRect& bounding_box) {

  if (!entity_exists(index)) {
    return;
  }

  set_entity_top_left(index, bounding_box.topLeft());
  set_entity_size(index, bounding_box.size());
}

/**
 * @brief Returns a field of an entity on the map.
 * @param index Index of an entity.
 * @param key Key of the field to get.
 * @return The corresponding value.
 * Returns an invalid QVariant() if there is no entity with this index
 * or no such field.
 */
QVariant MapModel::get_entity_field(const EntityIndex& index, const QString& key) const {

  if (!entity_exists(index)) {
    return QPoint();
  }

  return get_entity(index).get_field(key);
}

/**
 * @brief Sets a field of an entity on the map.
 *
 * Emits entity_field_changed() if there is a change.
 *
 * @param index Index of the entity to change.
 * @param key Key of the field to set.
 * @param value The new value.
 * Does nothing if there is no entity with this index, no such field or if the
 * value has an incorrect type.
 */
void MapModel::set_entity_field(const EntityIndex& index, const QString& key, const QVariant& value) {

  if (!entity_exists(index)) {
    return;
  }

  if (!value.isValid()) {
    return;
  }

  EntityModel& entity = get_entity(index);

  if (value == entity.get_field(key)) {
    // No change.
    return;
  }

  entity.set_field(key, value);
  emit entity_field_changed(index, key, value);
}

/**
 * @brief Adds entities to the map.
 *
 * They should have a invalid index (not be on the map) before the call.
 * After this call, they belong to this map object.
 * Emits entities_added() before added them and
 * entities_added() after.
 *
 * @param entities The entities to add and their future indexes.
 * They must be sorted in ascending order of indexes.
 */
void MapModel::add_entities(AddableEntities&& entities) {

  if (entities.empty()) {
    return;
  }

  QList<EntityIndex> indexes;
  for (const AddableEntity& addable_entity : entities) {
    indexes.append(addable_entity.index);
  }
  emit entities_about_to_be_added(indexes);

  // Add each entity in ascending order.
  QSet<Layer> layers_with_dirty_indexes;
  for (AddableEntity& addable_entity : entities) {

    EntityModelPtr& entity(addable_entity.entity);

    // Sanity checks.
    Q_ASSERT(entity != nullptr);
    Q_ASSERT(!entity->is_on_map());

    const EntityIndex& index = addable_entity.index;
    Q_ASSERT(index.is_valid());

    // Check the name uniqueness.
    entity->ensure_name_unique();
    const std::string& std_name = entity->get_entity().get_name();
    Q_ASSERT(entity->get_name().toStdString() == std_name);
    Q_ASSERT(!map.entity_exists(std_name));

    // Add the entity on the Solarus side.
    bool inserted = map.insert_entity(entity->get_entity(), index);
    Q_ASSERT(inserted);

    // Update the entity model and the entity list in the map editor.
    Layer layer = index.layer;
    int i = index.order;
    auto it = this->entities[layer].begin() + i;
    this->entities[layer].emplace(it, std::move(entity));
    get_entity(index).added_to_map(index);

    // Other indexes are now dirty, unless the entity was appended.
    if (i < (int) this->entities[layer].size() - 1) {
      layers_with_dirty_indexes.insert(layer);
    }
  }

  // Each entity stores its own index, so they might get shifted.
  for (Layer layer : layers_with_dirty_indexes) {
    rebuild_entity_indexes(layer);
  }

  // Notify people now that indexes are clean.
  emit entities_added(indexes);
}

/**
 * @brief Remove entities from the map.
 *
 * They should have a valid index (be on the map) before the call.
 * After this call, they have an invalid index and belong to the caller.
 * Emits entities_about_to_be_removed() before removing them
 * and entities_removed() after.
 *
 * @param indexes Indexes of the entities to remove.
 * @return The removed entities and their old indexes.
 * They must be sorted in ascending order of indexes.
 */
AddableEntities MapModel::remove_entities(const QList<EntityIndex>& indexes) {

  if (indexes.empty()) {
    return AddableEntities();
  }

  emit entities_about_to_be_removed(indexes);

  QSet<Layer> layers_with_dirty_indexes;

  AddableEntities entities;
  // Remove entities in descending order so that indexes to remove don't shift.
  for (auto it = indexes.end(); it != indexes.begin(); ) {
    --it;

    const EntityIndex& index = *it;

    // Sanity checks.
    Q_ASSERT(index.is_valid());
    Q_ASSERT(entity_exists(index));

    // Update the entity model and the entity list in the map editor.
    Layer layer = index.layer;
    int i = index.order;
    auto it2 = this->entities[layer].begin() + i;
    EntityModelPtr entity = std::move(*it2);
    entity->about_to_be_removed_from_map();
    this->entities[layer].erase(it2);

    // Remove the entity on the Solarus side.
    map.remove_entity(index);

    // Other indexes are now dirty, unless the entity was the last element.
    if (i < (int) this->entities[layer].size()) {
      layers_with_dirty_indexes.insert(layer);
    }

    // Return the removed entity to the caller.
    entities.emplace_front(std::move(entity), index);
  }

  // Each entity stores its own index, so they might get shifted.
  for (Layer layer : layers_with_dirty_indexes) {
    rebuild_entity_indexes(layer);
  }

  // Notify people now that indexes are clean.
  emit entities_removed(indexes);

  return entities;
}

/**
 * @brief Sets the indexes of all entities on a layer from their rank in the
 * entities list.
 *
 * This function should be called when entities are added, moved or removed
 * because each entity stores its own index.
 *
 * @param layer Layer to update.
 */
void MapModel::rebuild_entity_indexes(Layer layer) {

  int i = 0;
  for (auto it = entities[layer].begin(); it != entities[layer].end(); ++it) {

    const EntityModelPtr& entity = *it;
    Q_ASSERT(entity != nullptr);
    EntityIndex index = entity->get_index();
    index.order = i;
    entity->index_changed(index);
    ++i;
  }
}
