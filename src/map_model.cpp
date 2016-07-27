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
#include "entities/entity_model.h"
#include "editor_exception.h"
#include "map_model.h"
#include "quest.h"
#include "point.h"
#include "size.h"
#include "tileset_model.h"
#include <QIcon>
#include <QSet>

namespace SolarusEditor {

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
  tileset_model(nullptr),
  entities() {

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

  // Create entities.
  for (int layer = map.get_min_layer(); layer <= map.get_max_layer(); ++layer) {
    for (int i = 0; i < get_num_entities(layer); ++i) {
      EntityIndex index = { layer, i };
      entities[layer].emplace_back(EntityModel::create(*this, index));
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
 * @brief Returns the lowest layer of the map.
 * @return The lowest layer (0 or less).
 */
int MapModel::get_min_layer() const {

  return map.get_min_layer();
}

/**
 * @brief Sets the lowest layer of the map.
 *
 * Entities that are on removed layers are removed from the map.
 *
 * Emits max_layer_changed() if there is a change.
 *
 * @param min_layer The new lowest layer.
 * @return The entities that were on removed layers.
 */
AddableEntities MapModel::set_min_layer(int min_layer) {

  Q_ASSERT(min_layer <= 0);

  if (min_layer == get_min_layer()) {
    return AddableEntities();
  }

  // Delete entities on removed layers.
  AddableEntities removed_entities;
  if (min_layer > get_min_layer()) {
    EntityIndexes indexes_to_remove;
    for (int layer = get_min_layer(); layer < min_layer; ++layer) {
      for (int i = 0; i < get_num_entities(layer); ++i) {
        indexes_to_remove.append({ layer, i });
      }
    }
    removed_entities = remove_entities(indexes_to_remove);
  }

  map.set_min_layer(min_layer);

  emit layer_range_changed(min_layer, get_max_layer());

  return removed_entities;
}

/**
 * @brief Returns the highest layer of the map.
 * @return The highest layer (0 or more).
 */
int MapModel::get_max_layer() const {

  return map.get_max_layer();
}

/**
 * @brief Sets the highest layer of the map.
 *
 * Entities that are on removed layers are removed from the map.
 *
 * Emits max_layer_changed() if there is a change.
 *
 * @param max_layer The new highest layer.
 * @return The entities that were on removed layers.
 */
AddableEntities MapModel::set_max_layer(int max_layer) {

  Q_ASSERT(max_layer >= 0);

  if (max_layer == get_max_layer()) {
    return AddableEntities();
  }

  // Delete entities on removed layers.
  AddableEntities removed_entities;
  if (max_layer < get_max_layer()) {
    EntityIndexes indexes_to_remove;
    for (int layer = max_layer + 1; layer <= get_max_layer(); ++layer) {
      for (int i = 0; i < get_num_entities(layer); ++i) {
        indexes_to_remove.append({ layer, i });
      }
    }
    removed_entities = remove_entities(indexes_to_remove);
  }

  map.set_max_layer(max_layer);

  emit layer_range_changed(get_min_layer(), max_layer);

  return removed_entities;
}

/**
 * @brief Returns whether a layer exists on this map.
 * @param layer A layer value to check.
 * @return @c true if there is a layer with this index.
 */
bool MapModel::is_valid_layer(int layer) const {
  return map.is_valid_layer(layer);
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

  reload_tileset();

  emit tileset_id_changed(tileset_id);
}

/**
 * @brief Notifies this map that its tileset files may have changed.
 *
 * The tileset is refreshed.
 *
 * Emis tileset_reloaded().
 */
void MapModel::reload_tileset() {

  const QString& tileset_id = get_tileset_id();

  if (tileset_id.isEmpty()) {
    tileset_model = nullptr;
  }
  else {
    tileset_model = new TilesetModel(quest, tileset_id, this);
  }

  // Notify children.
  for (auto& kvp : entities) {
    EntityModels& layer_entities = kvp.second;
    for (EntityModelPtr& entity : layer_entities) {
      entity->notify_tileset_changed(tileset_id);
    }
  }

  emit tileset_reloaded();
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
 * @brief Returns the total number of entities on a layer of the map.
 * @param layer A layer.
 * @return The total number of entities on that layer (tiles and dynamic entities).
 */
int MapModel::get_num_entities(int layer) const {
  return map.get_num_entities(layer);
}

/**
 * @brief Returns the number of tiles on a layer of the map.
 * @param layer A layer.
 * @return The number of tiles on that layer.
 */
int MapModel::get_num_tiles(int layer) const {
  return map.get_num_tiles(layer);
}

/**
 * @brief Returns the number of dynamic entities on a layer of the map.
 * @param layer A layer.
 * @return The number of dynamic entities on that layer.
 */
int MapModel::get_num_dynamic_entities(int layer) const {
  return map.get_num_dynamic_entities(layer);
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
  bool exists_in_model = (index.order >= 0 && index.order < (int) entities.at(index.layer).size());
  Q_UNUSED(exists_in_solarus);
  Q_ASSERT(exists_in_model == exists_in_solarus);

  return exists_in_model;
}

/**
 * @brief Returns the model of entity at the given index.
 * @param index A map entity index.
 * @return The corresponding entity model.
 */
const EntityModel& MapModel::get_entity(const EntityIndex& index) const {
  return *entities.at(index.layer).at(index.order);
}

/**
 * @overload
 *
 * Non-const version.
 */
EntityModel& MapModel::get_entity(const EntityIndex& index) {
  return *entities.at(index.layer).at(index.order);
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
 * @brief Returns whether the given entities all have the same type.
 * @param[in] indexes Indexes of the entities to check.
 * @param[out] type The common type found if any.
 * @return @c true if they all have the same type.
 */
bool MapModel::is_common_type(const EntityIndexes& indexes, EntityType& type) const {

  if (indexes.isEmpty()) {
    return false;
  }

  type = get_entity_type(indexes.first());
  Q_FOREACH (const EntityIndex& index, indexes) {
    if (get_entity_type(index) != type) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Returns all entities of a given type.
 * @param type The type to get.
 * @return All entities of this type on the map.
 */
EntityIndexes MapModel::find_entities_of_type(EntityType type) const {

  EntityIndexes result;
  for (const auto& kvp : entities) {
    const EntityModels& layer_entities = kvp.second;
    for (const EntityModelPtr& entity : layer_entities) {
      if (entity->get_type() == type) {
        result << entity->get_index();
      }
    }
  }
  return result;
}

/**
 * @brief Returns the index of the default destination.
 * @return The default destination or an invalid index.
 */
EntityIndex MapModel::find_default_destination_index() const {

  EntityIndexes destination_indexes = find_entities_of_type(EntityType::DESTINATION);
  Q_FOREACH (const EntityIndex& index, destination_indexes) {
    if (get_entity_field(index, "default").toBool()) {
      return index;
    }
  }

  return EntityIndex();
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
 * Emits entity_name_changed().
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

  emit entity_name_changed(index, name);
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
 * @brief Returns all entity indexes of the map that have a non-empty name.
 * @return The named entities.
 */
QMap<QString, EntityIndex> MapModel::get_named_entities() const {

  const std::map<std::string, EntityIndex>& solarus_indexes = map.get_named_entities_indexes();
  QMap<QString, EntityIndex> indexes;
  for (const auto& kvp : solarus_indexes) {
    indexes.insert(QString::fromStdString(kvp.first), kvp.second);
  }
  return indexes;
}

/**
 * @brief Returns the layer where an entity is on the map.
 * @param index Index of a map entity.
 * @return The layer.
 * Returns -1 if there is no entity at this index.
 */
int MapModel::get_entity_layer(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    // No such entity.
    return -1;
  }

  return get_entity(index).get_layer();
}

/**
 * @brief Sets the layer of an entity on the map.
 *
 * Emits entity_layer_changed() if there is a change.
 *
 * @param index Index of the entity to change.
 * @param layer The new layer. The entity will be on top of other entities
 * of that layer.
 * @return The new index of the entity.
 * Returns an invalid index in case of error.
 */
EntityIndex MapModel::set_entity_layer(const EntityIndex& index_before, int layer_after) {

  if (!entity_exists(index_before)) {
    return EntityIndex();
  }

  int layer_before = index_before.layer;
  int order_before = index_before.order;
  if (layer_after == layer_before) {
    // No change.
    return index_before;
  }

  EntityIndex index_after = map.set_entity_layer(index_before, layer_after);
  int order_after = index_after.order;
  Q_ASSERT(index_after.layer == layer_after);

  auto it_before = entities[layer_before].begin() + order_before;
  EntityModelPtr entity = std::move(*it_before);
  Q_ASSERT(entity != nullptr);
  EntityModel* entity_before = entity.get();
  entities[layer_before].erase(it_before);

  auto it_after = entities[layer_after].begin() + order_after;
  entities[layer_after].insert(it_after, std::move(entity));

  Q_ASSERT(entity_exists(index_after));
  EntityModel* entity_after = &get_entity(index_after);
  Q_ASSERT(entity_after == entity_before);
  Q_UNUSED(entity_before);
  entity_after->index_changed(index_after);

  // FIXME set_entities_layer() for performance
  rebuild_entity_indexes(layer_before);
  rebuild_entity_indexes(layer_after);

  emit entity_layer_changed(index_before, index_after);

  return index_after;
}

/**
 * @brief Returns whether the given entities all have the same layer.
 * @param[in] indexes Indexes of the entities to check.
 * @param[out] layer The common layer found if any.
 * @return @c true if they all have the same layer.
 */
bool MapModel::is_common_layer(const EntityIndexes& indexes, int& layer) const {

  if (indexes.isEmpty()) {
    return false;
  }

  layer = indexes.first().layer;
  Q_FOREACH (const EntityIndex& index, indexes) {
    if (index.layer != layer) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Sets the layer of some entities on the map.
 *
 * Emits entity_layer_changed() for each change.
 *
 * @param indexes_before Sorted indexes of the entities to change.
 * @param layers_after The new layer for each entity.
 * Each entity will be placed on top of other entities of that layer.
 * @return The new indexes of the entities.
 */
EntityIndexes MapModel::set_entities_layer(const EntityIndexes& indexes_before, const QList<int>& layers_after) {

  // TODO possible improvement: entities whose layer do not change should also
  // be moved to keep the relative order of the whole group.

  Q_ASSERT(layers_after.size() == indexes_before.size());

  // Work on entities instead of indexes, because indexes change during the traversal.
  QList<EntityModel*> entities;
  Q_FOREACH (const EntityIndex& index_before, indexes_before) {
    entities.append(&get_entity(index_before));
  }

  int i = 0;
  Q_FOREACH (const EntityModel* entity, entities) {
    Q_ASSERT(entity != nullptr);
    const int layer_after = layers_after[i];
    if (entity->get_layer() != layer_after) {
      set_entity_layer(entity->get_index(), layer_after);
    }
    ++i;
  }

  // Now all indexes have finished their changes.
  EntityIndexes indexes_after;
  Q_FOREACH (const EntityModel* entity, entities) {
    indexes_after.append(entity->get_index());
  }

  return indexes_after;
}

/**
 * @brief Reverts what was done by a call to set_entities_layer().
 *
 * No other index change should have happened in the meantime.
 * The initial stacking order is restored.
 *
 * @param indexes_after Indexes after the change, as returned by set_entities_layer().
 * @param indexes_before Indexes before the change, as passed to set_entities_layer().
 */
void MapModel::undo_set_entities_layer(const EntityIndexes& indexes_after, const EntityIndexes& indexes_before) {

  Q_ASSERT(indexes_after.size() == indexes_before.size());

  // Work on entities instead of indexes, because indexes change during the traversal.
  QList<EntityModel*> entities;
  Q_FOREACH (const EntityIndex& index_after, indexes_after) {
    entities.append(&get_entity(index_after));
  }

  for (int i = 0; i < entities.size(); ++i) {
    EntityModel* entity = entities.at(i);
    const EntityIndex& index_before = indexes_before.at(i);
    const EntityIndex& index_after = entity->get_index();  // The entity knows its own updated index.
    if (index_before.layer == index_after.layer) {
      // Nothing to do for this entity.
      continue;
    }
    EntityIndex tmp_index = set_entity_layer(index_after, index_before.layer);
    set_entity_order(tmp_index, index_before.order);
  }
}

/**
 * @brief Changes the order of an entity in its layer.
 *
 * Emits entity_order_changed() if there is a change.
 *
 * @param index The current index of the entity to change.
 * @param order The new order to set.
 * It must be valid: in particular, tiles must remain before dynamic entities.
 * Does nothing if there is no entity with this index or if the requested
 * order is invalid.
 */
void MapModel::set_entity_order(const EntityIndex& index_before, int order_after) {

  if (!entity_exists(index_before)) {
    return;
  }

  int order_before = index_before.order;
  if (order_after == order_before) {
    // No change.
    return;
  }

  int layer = index_before.layer;
  auto it = entities[layer].begin() + order_before;
  EntityModelPtr entity = std::move(*it);
  EntityModel* entity_before = entity.get();
  Q_ASSERT(entity != nullptr);
  bool dynamic = entity->is_dynamic();
  int min_order = dynamic ? get_num_tiles(layer) : 0;
  int max_order = dynamic ? (get_num_entities(layer) - 1) : (get_num_tiles(layer) - 1);
  Q_ASSERT(order_after >= min_order);
  Q_ASSERT(order_after <= max_order);
  Q_UNUSED(min_order);
  Q_UNUSED(max_order);

  map.set_entity_order(index_before, order_after);

  entities[layer].erase(it);
  it = entities[layer].begin() + order_after;
  entities[layer].emplace(it, std::move(entity));

  EntityIndex index_after(layer, order_after);
  EntityModel* entity_after = &get_entity(index_after);
  Q_ASSERT(entity_exists(index_after));
  Q_ASSERT(entity_after == entity_before);
  Q_UNUSED(entity_before);
  entity_after->index_changed(index_after);

  rebuild_entity_indexes(layer);

  emit entity_order_changed(index_before, order_after);
}

/**
 * @brief Moves an entity to the front in its layer.
 *
 * This function makes sure that tiles remain before dynamic entities.
 * Emits entity_order_changed() if there is a change.
 *
 * @param index The current index of the entity to change.
 * @return The new index of the entity.
 * Does nothing if there is no entity with this index.
 */
EntityIndex MapModel::bring_entity_to_front(const EntityIndex& index_before) {

  if (!entity_exists(index_before)) {
    return EntityIndex();
  }

  int layer = index_before.layer;
  bool dynamic = get_entity(index_before).is_dynamic();
  int order_after = dynamic ? (get_num_entities(layer) - 1) : (get_num_tiles(layer) - 1);
  set_entity_order(index_before, order_after);

  return { layer, order_after };
}

/**
 * @brief Moves an entity to the back in its layer.
 *
 * This function makes sure that tiles remain before dynamic entities.
 * Emits entity_order_changed() if there is a change.
 *
 * @param index The current index of the entity to change.
 * @return The new index of the entity.
 * Does nothing if there is no entity with this index.
 */
EntityIndex MapModel::bring_entity_to_back(const EntityIndex& index_before) {

  if (!entity_exists(index_before)) {
    return EntityIndex();
  }

  int layer = index_before.layer;
  bool dynamic = get_entity(index_before).is_dynamic();
  int order_after = dynamic ? get_num_tiles(layer) : 0;
  set_entity_order(index_before, order_after);

  return { layer, order_after };
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
 * @brief Returns whether an entity has a legal size.
 * @param index Index of the entity to check.
 * @return @c true if its size is valid.
 * Returns @c false if there is no entity with this index.
 */
bool MapModel::is_entity_size_valid(const EntityIndex& index) const {

  return is_entity_size_valid(index, get_entity_size(index));
}

/**
 * @brief Returns whether the given size is valid for an entity.
 * @param index Index of the entity to check.
 * @param size The size to check.
 * @return @c true if its size is valid.
 * Returns @c false if there is no entity with this index.
 */
bool MapModel::is_entity_size_valid(const EntityIndex& index, const QSize& size) const {

  if (!entity_exists(index)) {
    return false;
  }

  return get_entity(index).is_size_valid(size);
}

/**
 * @brief Returns a legal size for an entity.
 * @param index Index of an entity.
 * @return A valid size that can be set.
 * Returns @c QSize() if there is no entity with this index.
 */
QSize MapModel::get_entity_valid_size(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return QSize();
  }

  return get_entity(index).get_valid_size();
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
 * @brief Returns whether an entity has a direction field.
 * @param index Index of an entity.
 * @return @c true if a direction field exists, even if it is unset.
 * Returns @c false if there is no entity with this index.
 */
bool MapModel::has_entity_direction_field(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return false;
  }

  return get_entity(index).has_direction_field();
}

/**
 * @brief Returns whether an entity allows the special direction value -1.
 * @param index Index of an entity.
 * @return @c true if the entity can have no direction.
 * Returns @c false if there is no entity with this index or if the entity has
 * no direction field.
 */
bool MapModel::is_entity_no_direction_allowed(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return false;
  }

  return get_entity(index).is_no_direction_allowed();
}

/**
 * @brief Returns the text to show in a GUI for the special no-direction value
 * -1 of an entity.
 * @param index Index of an entity.
 * @return The no-direction text.
 * Returns an empty string if there is no entity with this index.
 */
QString MapModel::get_entity_no_direction_text(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return "";
  }

  return get_entity(index).get_no_direction_text();
}

/**
 * @brief Returns the number of possible directions of an entity.
 *
 * This does not include the special value -1 (if it is allowed).
 *
 * @param index Index of an entity.
 * @return The number of possible directions for this entity.
 * Returns 0 if there is no entity with this index or if the entity has
 * no direction field.
 */
int MapModel::get_entity_num_directions(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return 0;
  }

  return get_entity(index).get_num_directions();
}

/**
 * @brief Returns whether the given entities all have the same direction rules.
 *
 * It means that they have the same number of possible directions,
 * the same setting of allowing the special no-direction value
 * and the same no-direction text.
 *
 * @param[in] indexes Indexes of the entities to check.
 * @param[out] num_directions The common number of directions found if any.
 * @param[out] no_direction_text The text corresponding to the special
 * no-direction value if allowed (an empty string means not allowed).
 * @return @c true if they all have the same direction rules.
 */
bool MapModel::is_common_direction_rules(
    const EntityIndexes& indexes,
    int& num_directions,
    QString& no_direction_text) const {

  if (indexes.isEmpty()) {
    return false;
  }

  const EntityIndex& first = indexes.first();
  num_directions = get_entity_num_directions(first);
  bool no_direction_allowed = is_entity_no_direction_allowed(first);
  no_direction_text = get_entity_no_direction_text(first);

  Q_FOREACH (const EntityIndex& index, indexes) {
    if (get_entity_num_directions(index) != num_directions ||
        is_entity_no_direction_allowed(index) != no_direction_allowed ||
        get_entity_no_direction_text(index) != no_direction_text) {
      return false;
    }
  }

  if (!no_direction_allowed) {
    no_direction_text.clear();
  }
  return true;
}

/**
 * @brief Returns the direction of an entity.
 * @param index Index of an entity.
 * @return The direction of this entity, or -1 if the entity has no direction
 * field or if it is unset.
 * Returns -1 if there is no entity with this index.
 */
int MapModel::get_entity_direction(const EntityIndex& index) const {

  if (!entity_exists(index)) {
    return -1;
  }

  return get_entity(index).get_direction();
}

/**
 * @brief Sets the direction of an entity.
 * @param index Index of an entity.
 * @param direction The direction to set, or -1 to set no direction.
 * Does nothing if there is no entity with this index or if the entity has no
 * direction field.
 */
void MapModel::set_entity_direction(const EntityIndex& index, int direction) {

  if (!entity_exists(index)) {
    return;
  }

  EntityModel& entity = get_entity(index);
  if (direction == entity.get_direction()) {
    // No change.
    return;
  }

  get_entity(index).set_direction(direction);
  emit entity_direction_changed(index, direction);
}

/**
 * @brief Returns whether the given entities all have the same direction.
 *
 * This is only possible when the direction rules match: the number of
 * possible directions should be the same and the setting of allowing the
 * special no-direction value -1 should be the same.
 *
 * @param[in] indexes Indexes of the entities to check.
 * @param[out] direction The common direction found if any.
 * It can be -1 and still be legal if all entities have the special
 * no-direction value -1.
 * @return @c true if they all have the same direction value and the same
 * direction rules.
 */
bool MapModel::is_common_direction(const EntityIndexes& indexes, int& direction) const {

  if (indexes.isEmpty()) {
    return false;
  }

  int num_directions = 0;
  QString no_direction_text;
  if (!is_common_direction_rules(indexes, num_directions, no_direction_text)) {
    return false;
  }

  const EntityIndex& first = indexes.first();
  direction = get_entity_direction(first);

  Q_FOREACH (const EntityIndex& index, indexes) {
    if (get_entity_direction(index) != direction) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Returns whether an entity has a field with the given name.
 * @param index Index of an entity.
 * @param key Key of the field to check.
 * @return @c true if such a field exists.
 * Returns @c false if there is no entity with this index.
 */
bool MapModel::has_entity_field(const EntityIndex& index, const QString& key) const {

  if (!entity_exists(index)) {
    return false;
  }

  return get_entity(index).has_field(key);
}

/**
 * @brief Returns a field of an entity on the map.
 * @param index Index of an entity.
 * @param key Key of the field to get.
 * @return The corresponding value.
 * Returns an invalid QVariant if there is no entity with this index
 * or no such field.
 */
QVariant MapModel::get_entity_field(const EntityIndex& index, const QString& key) const {

  if (!entity_exists(index)) {
    return QVariant();
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

  EntityIndexes indexes;
  for (const AddableEntity& addable_entity : entities) {
    indexes.append(addable_entity.index);
  }
  emit entities_about_to_be_added(indexes);

  // Add each entity in ascending order.
  QSet<int> layers_with_dirty_indexes;
  for (AddableEntity& addable_entity : entities) {

    EntityModelPtr& entity(addable_entity.entity);

    // Sanity checks.
    Q_ASSERT(entity != nullptr);
    Q_ASSERT(!entity->is_on_map());

    const EntityIndex& index = addable_entity.index;
    Q_ASSERT(index.is_valid());

    // Check the name uniqueness.
    entity->ensure_valid_on_map();
    const std::string& std_name = entity->get_entity().get_name();
    Q_UNUSED(std_name);
    Q_ASSERT(entity->get_name().toStdString() == std_name);
    Q_ASSERT(!map.entity_exists(std_name));

    // Add the entity on the Solarus side.
    bool inserted = map.insert_entity(entity->get_entity(), index);
    Q_UNUSED(inserted);
    Q_ASSERT(inserted);

    // Update the entity model and the entity list in the map editor.
    int layer = index.layer;
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
  for (int layer : layers_with_dirty_indexes) {
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
AddableEntities MapModel::remove_entities(const EntityIndexes& indexes) {

  if (indexes.empty()) {
    return AddableEntities();
  }

  emit entities_about_to_be_removed(indexes);

  QSet<int> layers_with_dirty_indexes;

  AddableEntities entities;
  // Remove entities in descending order so that indexes to remove don't shift.
  for (auto it = indexes.end(); it != indexes.begin(); ) {
    --it;

    const EntityIndex& index = *it;

    // Sanity checks.
    Q_ASSERT(index.is_valid());
    Q_ASSERT(entity_exists(index));

    // Update the entity model and the entity list in the map editor.
    int layer = index.layer;
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
  Q_FOREACH (int layer, layers_with_dirty_indexes) {
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
 * All entities of the layer should already know their correct layer,
 * only the order on this layer is updated.
 *
 * @param layer Layer to update.
 */
void MapModel::rebuild_entity_indexes(int layer) {

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

}
