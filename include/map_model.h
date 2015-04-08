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
#ifndef SOLARUSEDITOR_MAP_MODEL_H
#define SOLARUSEDITOR_MAP_MODEL_H

#include "entities/entity_model.h"
#include "layer_traits.h"
#include "sprite_model.h"
#include <array>
#include <memory>

struct AddableEntity;
class Quest;
class QuestResources;
class TilesetModel;
class ViewSettings;

using AddableEntities = std::deque<AddableEntity>;

/**
 * @brief Model that wraps a map.
 *
 * It makes the link between the editor and the map data of the
 * Solarus library.
 * Signals are sent when something changes in the wrapped map.
 */
class MapModel : public QObject {
  Q_OBJECT

public:

  static constexpr int NO_FLOOR = Solarus::MapData::NO_FLOOR;

  // Creation.
  MapModel(Quest& quest, const QString& map_id, QObject* parent = nullptr);

  const Quest& get_quest() const;
  Quest& get_quest();
  QString get_map_id() const;

  // Map properties.
  QSize get_size() const;
  void set_size(const QSize& size);
  bool has_world() const;
  QString get_world() const;
  void set_world(const QString& world);
  bool has_floor() const;
  int get_floor() const;
  void set_floor(int floor);
  QPoint get_location() const;
  void set_location(const QPoint& location);
  TilesetModel* get_tileset_model() const;
  QString get_tileset_id() const;
  void set_tileset_id(const QString& tileset_id);
  QString get_music_id() const;
  void set_music_id(const QString& music_id);

  // Entities.
  int get_num_entities() const;
  int get_num_entities(Layer layer) const;
  bool entity_exists(const EntityIndex& index) const;
  EntityType get_entity_type(const EntityIndex& index) const;
  QString get_entity_type_name(const EntityIndex& index) const;
  QString get_entity_name(const EntityIndex& index) const;
  bool set_entity_name(const EntityIndex& index, const QString& name);
  bool entity_name_exists(const QString& name) const;
  EntityIndex find_entity_by_name(const QString& name) const;
  Layer get_entity_layer(const EntityIndex& index) const;
  QPoint get_entity_xy(const EntityIndex& index) const;
  void set_entity_xy(const EntityIndex& index, const QPoint& xy);
  void add_entity_xy(const EntityIndex& index, const QPoint& translation);
  QPoint get_entity_top_left(const EntityIndex& index) const;
  void set_entity_top_left(const EntityIndex& index, const QPoint& top_left);
  QPoint get_entity_origin(const EntityIndex& index) const;
  QSize get_entity_size(const EntityIndex& index) const;
  void set_entity_size(const EntityIndex& index, const QSize& size);
  QRect get_entity_bounding_box(const EntityIndex& index) const;
  void set_entity_bounding_box(const EntityIndex& index, const QRect& bounding_box);
  QVariant get_entity_field(const EntityIndex& index, const QString& key) const;
  void set_entity_field(const EntityIndex& index, const QString& key, const QVariant& value);
  void add_entities(AddableEntities&& entities);
  AddableEntities remove_entities(const EntityIndexes& indexes);

  const Solarus::EntityData& get_internal_entity(const EntityIndex& index) const;
  Solarus::EntityData& get_internal_entity(const EntityIndex& index);

  const EntityModel& get_entity(const EntityIndex& index) const;
  EntityModel& get_entity(const EntityIndex& index);

signals:

  void size_changed(const QSize& size);
  void world_changed(const QString& world);
  void floor_changed(int floor);
  void location_changed(const QPoint& location);
  void tileset_id_changed(const QString& tileset_id);
  void music_id_changed(const QString& music_id);

  void entities_about_to_be_added(const EntityIndexes& indexes);
  void entities_added(const EntityIndexes& indexes);
  void entities_about_to_be_removed(const EntityIndexes& indexes);
  void entities_removed(const EntityIndexes& indexes);
  void entity_name_changed(const QString& name);
  void entity_xy_changed(const EntityIndex& index, const QPoint& xy);
  void entity_size_changed(const EntityIndex& index, const QSize& size);
  void entity_field_changed(const EntityIndex& index, const QString& key, const QVariant& value);

public slots:

  void save() const;

private:

  void rebuild_entity_indexes(Layer layer);

  Quest& quest;                   /**< The quest the tileset belongs to. */
  const QString map_id;           /**< Id of the map. */
  Solarus::MapData map;           /**< Map data wrapped by this model. */
  TilesetModel* tileset_model;    /**< Tileset of this map. nullptr if not set. */
  std::array<EntityModels, Layer::LAYER_NB>
      entities;                   /**< All entities. */

};

/**
 * @brief Wraps an entity ready to be added to a map and its future index.
 */
struct AddableEntity {

public:

  AddableEntity(EntityModelPtr&& entity, const EntityIndex& index) :
    entity(std::move(entity)),
    index(index) {
  }

  // Comparison operators useful to sort lists.
  bool operator<=(const AddableEntity& other) const { return index <= other.index; }
  bool operator<(const AddableEntity& other) const  { return index < other.index; }
  bool operator>=(const AddableEntity& other) const { return index >= other.index; }
  bool operator>(const AddableEntity& other) const  { return index > other.index; }

  EntityModelPtr entity;
  EntityIndex index;

};

#endif
