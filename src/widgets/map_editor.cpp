/*
 * Copyright (C) 2014-2018 Christopho, Solarus - http://www.solarus-games.org
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
#include "entities/destination.h"
#include "entities/entity_model.h"
#include "entities/dynamic_tile.h"
#include "widgets/gui_tools.h"
#include "widgets/map_editor.h"
#include "widgets/map_scene.h"
#include "widgets/pattern_picker_dialog.h"
#include "widgets/tileset_scene.h"
#include "audio.h"
#include "editor_exception.h"
#include "editor_settings.h"
#include "file_tools.h"
#include "map_model.h"
#include "point.h"
#include "quest.h"
#include "quest_database.h"
#include "refactoring.h"
#include "tileset_model.h"
#include "view_settings.h"
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QUndoStack>
#include <memory>

namespace SolarusEditor {

namespace {

constexpr int move_entities_command_id = 1;
constexpr int resize_entities_command_id = 2;

/**
 * @brief Parent class of all undoable commands of the map editor.
 */
class MapEditorCommand : public QUndoCommand {

public:

  MapEditorCommand(MapEditor& editor, const QString& text) :
    QUndoCommand(text),
    editor(editor) {
  }

  MapEditor& get_editor() const {
    return editor;
  }

  MapModel& get_map() const {
    return editor.get_map();
  }

  MapView& get_map_view() const {
    return editor.get_map_view();
  }

  ViewSettings& get_view_settings() const {
    return editor.get_view_settings();
  }

private:

  MapEditor& editor;

};

/**
 * @brief Changing the size of the map.
 */
class SetSizeCommand : public MapEditorCommand {

public:
  SetSizeCommand(MapEditor& editor, const QSize& size) :
    MapEditorCommand(editor, MapEditor::tr("Map size")),
    before(get_map().get_size()),
    after(size) { }

  void undo() override { get_map().set_size(before); }
  void redo() override { get_map().set_size(after); }

private:
  QSize before, after;
};

/**
 * @brief Changing the lowest layer of the map.
 */
class SetMinLayerCommand : public MapEditorCommand {

public:
  SetMinLayerCommand(MapEditor& editor, int min_layer) :
    MapEditorCommand(editor, MapEditor::tr("Lowest layer")),
    min_layer_before(get_map().get_min_layer()),
    min_layer_after(min_layer),
    entities_removed() { }

  void undo() override {
    get_map().set_min_layer(min_layer_before);
    // Restore entities.
    get_map().add_entities(std::move(entities_removed));
  }

  void redo() override {
    entities_removed = get_map().set_min_layer(min_layer_after);
  }

private:
  int min_layer_before;
  int min_layer_after;
  AddableEntities entities_removed;  // Entities that were on removed layers.
};

/**
 * @brief Changing the highest layer of the map.
 */
class SetMaxLayerCommand : public MapEditorCommand {

public:
  SetMaxLayerCommand(MapEditor& editor, int max_layer) :
    MapEditorCommand(editor, MapEditor::tr("Highest layer")),
    max_layer_before(get_map().get_max_layer()),
    max_layer_after(max_layer),
    entities_removed() { }

  void undo() override {
    get_map().set_max_layer(max_layer_before);
    // Restore entities.
    get_map().add_entities(std::move(entities_removed));
  }

  void redo() override {
    entities_removed = get_map().set_max_layer(max_layer_after);
  }

private:
  int max_layer_before;
  int max_layer_after;
  AddableEntities entities_removed;  // Entities that were on removed layers.
};

/**
 * @brief Changing the world of the map.
 */
class SetWorldCommand : public MapEditorCommand {

public:
  SetWorldCommand(MapEditor& editor, const QString& world) :
    MapEditorCommand(editor, MapEditor::tr("Map world")),
    before(get_map().get_world()),
    after(world) { }

  void undo() override { get_map().set_world(before); }
  void redo() override { get_map().set_world(after); }

private:
  QString before, after;
};

/**
 * @brief Changing the floor of the map.
 */
class SetFloorCommand : public MapEditorCommand {

public:
  SetFloorCommand(MapEditor& editor, int floor) :
    MapEditorCommand(editor, MapEditor::tr("Map floor")),
    before(get_map().get_floor()),
    after(floor) { }

  void undo() override { get_map().set_floor(before); }
  void redo() override { get_map().set_floor(after); }

private:
  int before, after;
};

/**
 * @brief Changing the location of the map.
 */
class SetLocationCommand : public MapEditorCommand {

public:
  SetLocationCommand(MapEditor& editor, const QPoint& location) :
    MapEditorCommand(editor, MapEditor::tr("Map location")),
    before(get_map().get_location()),
    after(location) { }

  void undo() override { get_map().set_location(before); }
  void redo() override { get_map().set_location(after); }

private:
  QPoint before, after;
};

/**
 * @brief Changing the tileset of the map.
 */
class SetTilesetCommand : public MapEditorCommand {

public:
  SetTilesetCommand(MapEditor& editor, const QString& tileset_id) :
    MapEditorCommand(editor, MapEditor::tr("Tileset")),
    before(get_map().get_tileset_id()),
    after(tileset_id) { }

  void undo() override { get_map().set_tileset_id(before); }
  void redo() override { get_map().set_tileset_id(after); }

private:
  QString before, after;
};

/**
 * @brief Changing the music of the map.
 */
class SetMusicCommand : public MapEditorCommand {

public:
  SetMusicCommand(MapEditor& editor, const QString& music_id) :
    MapEditorCommand(editor, MapEditor::tr("Music")),
    before(get_map().get_music_id()),
    after(music_id) { }

  void undo() override { get_map().set_music_id(before); }
  void redo() override { get_map().set_music_id(after); }

private:
  QString before, after;
};

/**
 * @brief Editing an entity.
 */
class EditEntityCommand : public MapEditorCommand {

public:
  EditEntityCommand(MapEditor& editor, const EntityIndex& index_before, EntityModelPtr entity_after) :
    MapEditorCommand(editor, MapEditor::tr("Edit entity")),
    index_before(index_before),
    entity_after(std::move(entity_after)) { }

  void undo() override {

    MapModel& map = get_map();

    // Remove the new entity created by redo().
    AddableEntities removed_entities = map.remove_entities(EntityIndexes() << index_after);
    AddableEntity& removed_entity = *removed_entities.begin();
    entity_after = std::move(removed_entity.entity);

    // Restore the old one.
    AddableEntities addable_entities;
    addable_entities.emplace_back(std::move(entity_before), index_before);
    map.add_entities(std::move(addable_entities));

    // Restore the previous default destination.
    if (entity_after->get_type() == EntityType::DESTINATION &&
        entity_after->get_field("default").toBool() &&
        default_destination_index_before.is_valid() &&
        default_destination_index_before != index_before) {
      map.set_entity_field(default_destination_index_before, "default", true);
    }

    // Make it selected.
    get_map_view().set_only_selected_entity(index_before);
  }

  void redo() override {

    MapModel& map = get_map();

    // To implement the change, remove the old entity and add the new one.
    index_after = index_before;
    if (entity_after->get_layer() != index_before.layer) {
      // The layer changes: put the entity to the front.
      index_after.layer = entity_after->get_layer();
      index_after.order = entity_after->is_dynamic() ?
            map.get_num_entities(index_after.layer) : map.get_num_tiles(index_after.layer);
    }

    // Make sure there is only one destination.
    default_destination_index_before = map.find_default_destination_index();
    if (entity_after->get_type() == EntityType::DESTINATION &&
        entity_after->get_field("default").toBool() &&
        default_destination_index_before.is_valid() &&
        default_destination_index_before != index_before
    ) {
      map.set_entity_field(default_destination_index_before, "default", false);
    }

    // Remove the initial entity.
    AddableEntities removed_entities = map.remove_entities(EntityIndexes() << index_before);
    AddableEntity& removed_entity = *removed_entities.begin();
    entity_before = std::move(removed_entity.entity);

    // Add the new one to replace it.
    AddableEntities addable_entities;
    addable_entities.emplace_back(std::move(entity_after), index_after);
    map.add_entities(std::move(addable_entities));

    // Make the new one selected.
    get_map_view().set_only_selected_entity(index_after);
  }

  EntityIndex get_index_before() const { return index_before; }
  EntityIndex get_index_after() const { return index_after; }

private:
  EntityIndex index_before;
  EntityIndex index_after;
  EntityModelPtr entity_before;
  EntityModelPtr entity_after;
  EntityIndex default_destination_index_before;
};

/**
 * @brief Moving entities on the map.
 */
class MoveEntitiesCommand : public MapEditorCommand {

public:
  MoveEntitiesCommand(MapEditor& editor, const EntityIndexes& indexes, const QPoint& translation, bool allow_merge_to_previous) :
    MapEditorCommand(editor, MapEditor::tr("Move entities")),
    indexes(indexes),
    translation(translation),
    allow_merge_to_previous(allow_merge_to_previous) { }

  void undo() override {
    for (const EntityIndex& index : indexes) {
      get_map().add_entity_xy(index, -translation);
    }
    // Select impacted entities.
    get_map_view().set_selected_entities(indexes);
  }

  void redo() override {
    for (const EntityIndex& index : indexes) {
      get_map().add_entity_xy(index, translation);
    }
    // Select impacted entities.
    get_map_view().set_selected_entities(indexes);
  }

  int id() const override {
    return move_entities_command_id;
  }

  bool mergeWith(const QUndoCommand* other) override {

    if (other->id() != id()) {
      return false;
    }
    const MoveEntitiesCommand& other_move = *static_cast<const MoveEntitiesCommand*>(other);
    if (!other_move.allow_merge_to_previous) {
      return false;
    }

    translation += other_move.translation;
    return true;
  }

private:
  EntityIndexes indexes;
  QPoint translation;
  bool allow_merge_to_previous;
};

/**
 * @brief Resizing entities on the map.
 */
class ResizeEntitiesCommand : public MapEditorCommand {

public:
  ResizeEntitiesCommand(MapEditor& editor, const QMap<EntityIndex, QRect>& boxes, bool allow_merge_to_previous) :
    MapEditorCommand(editor, MapEditor::tr("Resize entities")),
    boxes_before(),
    boxes_after(boxes),
    allow_merge_to_previous(allow_merge_to_previous) {

    // Remember the old box of each entity to allow undo.
    MapModel& map = get_map();
    for (auto it = boxes.begin(); it != boxes.end(); ++it) {
      const EntityIndex& index = it.key();
      Q_ASSERT(map.entity_exists(index));
      boxes_before.insert(index, map.get_entity_bounding_box(index));
    }
  }

  void undo() override {

    EntityIndexes indexes;
    for (auto it = boxes_before.begin(); it != boxes_before.end(); ++it) {
      const EntityIndex& index = it.key();
      get_map().set_entity_bounding_box(index, it.value());
      indexes.append(index);
    }

    // Select impacted entities.
    get_map_view().set_selected_entities(indexes);
  }

  void redo() override {

    EntityIndexes indexes;
    for (auto it = boxes_after.begin(); it != boxes_after.end(); ++it) {
      const EntityIndex& index = it.key();
      QRect box_after = it.value();
      QSize size = box_after.size();
      if (!get_map().is_entity_size_valid(index, size)) {
        // Invalid size: refuse the change.
        box_after.setSize(boxes_before.value(index).size());
      }
      get_map().set_entity_bounding_box(index, it.value());
      indexes.append(index);
    }

    // Select impacted entities.
    get_map_view().set_selected_entities(indexes);
  }

  int id() const override {
    return resize_entities_command_id;
  }

  bool mergeWith(const QUndoCommand* other) override {

    if (other->id() != id()) {
      return false;
    }
    const ResizeEntitiesCommand& other_resize = *static_cast<const ResizeEntitiesCommand*>(other);
    if (!other_resize.allow_merge_to_previous) {
      return false;
    }

    for (auto it = other_resize.boxes_after.begin();
         it != other_resize.boxes_after.end();
         ++it) {
      const EntityIndex& index = it.key();
      boxes_after[index] = it.value();
    }
    return true;
  }

private:
  QMap<EntityIndex, QRect> boxes_before;
  QMap<EntityIndex, QRect> boxes_after;
  bool allow_merge_to_previous;
};

/**
 * @brief Converting normal tiles to dynamic tiles.
 */
class ConvertTilesToDynamicCommand : public MapEditorCommand {

public:
  ConvertTilesToDynamicCommand(MapEditor& editor, const EntityIndexes& indexes) :
    MapEditorCommand(editor, MapEditor::tr("Convert tiles")),
    indexes_before(indexes) {

    qSort(this->indexes_before);
  }

  void undo() override {
    get_map().remove_entities(indexes_after);
    get_map().add_entities(std::move(removed_tiles));
    get_map_view().set_selected_entities(indexes_before);
  }

  void redo() override {
    MapModel& map = get_map();
    AddableEntities dynamic_tiles;

    // Create the dynamic tiles.
    for (const EntityIndex& index_before : indexes_before) {
      EntityModelPtr dynamic_tile = DynamicTile::create_from_normal_tile(map, index_before);
      int layer = index_before.layer;
      EntityIndex index_after = { layer, -1 };
      dynamic_tiles.emplace_back(std::move(dynamic_tile), index_after);
    }

    // Remove the static ones.
    removed_tiles = map.remove_entities(indexes_before);

    // Determine the indexes where to place the dynamic ones.
    indexes_after.clear();
    std::map<int, int> order_after_by_layer;
    for (int layer = map.get_min_layer(); layer <= map.get_max_layer(); ++layer) {
      order_after_by_layer[layer] = map.get_num_entities(layer);
    }
    for (AddableEntity& addable : dynamic_tiles) {
      addable.index.order = order_after_by_layer[addable.index.layer];
      ++order_after_by_layer[addable.index.layer];
      indexes_after.append(addable.index);
    }

    // Add the dynamic tiles and make them selected.
    map.add_entities(std::move(dynamic_tiles));
    get_map_view().set_selected_entities(indexes_after);
  }

private:
  EntityIndexes indexes_before;
  EntityIndexes indexes_after;
  AddableEntities removed_tiles;
};

/**
 * @brief Converting dynamic tiles to normal tiles.
 */
class ConvertTilesFromDynamicCommand : public MapEditorCommand {

public:
  ConvertTilesFromDynamicCommand(MapEditor& editor, const EntityIndexes& indexes) :
    MapEditorCommand(editor, MapEditor::tr("Convert tiles")),
    indexes_before(indexes) {

    qSort(this->indexes_before);
  }

  void undo() override {
    get_map().remove_entities(indexes_after);
    get_map().add_entities(std::move(removed_tiles));
    get_map_view().set_selected_entities(indexes_before);
  }

  void redo() override {
    MapModel& map = get_map();
    AddableEntities tiles;

    // Create the dynamic tiles.
    for (const EntityIndex& index_before : indexes_before) {
      EntityModelPtr tile = Tile::create_from_dynamic_tile(map, index_before);
      int layer = index_before.layer;
      EntityIndex index_after = { layer, -1 };
      tiles.emplace_back(std::move(tile), index_after);
    }

    // Remove the dynamic ones.
    removed_tiles = map.remove_entities(indexes_before);

    // Determine the indexes where to place the dynamic ones.
    indexes_after.clear();
    std::map<int, int> order_after_by_layer;
    for (int layer = map.get_min_layer(); layer <= map.get_max_layer(); ++layer) {
      order_after_by_layer[layer] = map.get_num_tiles(layer);
    }
    for (AddableEntity& addable : tiles) {
      addable.index.order = order_after_by_layer[addable.index.layer];
      ++order_after_by_layer[addable.index.layer];
      indexes_after.append(addable.index);
    }

    // Add the normal tiles and make them selected.
    map.add_entities(std::move(tiles));
    get_map_view().set_selected_entities(indexes_after);
  }

private:
  EntityIndexes indexes_before;
  EntityIndexes indexes_after;
  AddableEntities removed_tiles;
};

/**
 * @brief Changing the pattern of some tiles.
 */
class ChangeTilesPatternCommand : public MapEditorCommand {

public:
  ChangeTilesPatternCommand(
      MapEditor& editor,
      const EntityIndexes&
      indexes,
      const QString& pattern_id
  ) :
    MapEditorCommand(editor, MapEditor::tr("Change pattern")),
    indexes(indexes),
    pattern_ids_before(),
    pattern_id_after(pattern_id) {

    for (const EntityIndex& index : indexes) {
      const QString& pattern_id_before =
          get_map().get_entity_field(index, "pattern").toString();
      pattern_ids_before << pattern_id_before;
      sizes_before << get_map().get_entity_size(index);
    }
  }

  void undo() override {
    MapModel& map = get_map();
    int i = 0;
    for (const EntityIndex& index : indexes) {
      map.set_entity_field(index, "pattern", pattern_ids_before[i]);
      map.set_entity_size(index, sizes_before[i]);
      ++i;
    }
    get_map_view().set_selected_entities(indexes);
    get_map_view().get_scene()->redraw_entities(indexes);
  }

  void redo() override {
    MapModel& map = get_map();
    for (const EntityIndex& index : indexes) {
      map.set_entity_field(index, "pattern", pattern_id_after);
      const QSize& size = map.get_entity_closest_base_size_multiple(index);
      if (map.is_entity_size_valid(index, size)) {
        map.set_entity_size(index, size);
      }
    }
    get_map_view().set_selected_entities(indexes);
    get_map_view().get_scene()->redraw_entities(indexes);
  }

private:
  EntityIndexes indexes;
  QStringList pattern_ids_before;
  QList<QSize> sizes_before;
  QString pattern_id_after;
};

/**
 * @brief Changing the direction of entities on the map.
 *
 * For some entities, the size is also changed if it becomes invalid.
 */
class SetEntitiesDirectionCommand : public MapEditorCommand {

public:
  SetEntitiesDirectionCommand(MapEditor& editor, const EntityIndexes& indexes, int direction) :
    MapEditorCommand(editor, MapEditor::tr("Set direction")),
    indexes(indexes),
    directions_before(),
    direction_after(direction) {
  }

  void undo() override {
    int i = 0;
    for (const EntityIndex& index : indexes) {
      get_map().set_entity_direction(index, directions_before.at(i));
      get_map().set_entity_size(index, sizes_before.at(i));
      ++i;
    }
    get_map_view().set_selected_entities(indexes);
    get_map_view().get_scene()->redraw_entities(indexes);
  }

  void redo() override {

    MapModel& map = get_map();

    // Change the direction.
    directions_before.clear();
    sizes_before.clear();
    for (const EntityIndex& index : indexes) {
      bool was_size_valid = map.is_entity_size_valid(index);
      directions_before.append(map.get_entity_direction(index));
      sizes_before.append(map.get_entity_size(index));

      map.set_entity_direction(index, direction_after);

      // Check that the size is still okay in the new direction.
      if (was_size_valid && !map.is_entity_size_valid(index)) {
        // The entity size is no longer valid in the new direction:
        // set a new size right now if there is only one entity selected.
        map.set_entity_size(index, map.get_entity_valid_size(index));
      }

      map.get_entity(index).reload_sprite();
    }

    // Select impacted entities.
    get_map_view().set_selected_entities(indexes);

    // Tell the scene to redraw the entities.
    get_map_view().get_scene()->redraw_entities(indexes);
  }

private:
  EntityIndexes indexes;
  QList<int> directions_before;
  int direction_after;
  QList<QSize> sizes_before;
};

/**
 * @brief Changing the layer of entities on the map.
 */
class SetEntitiesLayerCommand : public MapEditorCommand {

public:
  SetEntitiesLayerCommand(MapEditor& editor, const EntityIndexes& indexes, int layer) :
    MapEditorCommand(editor, MapEditor::tr("Set layer")),
    indexes_before(indexes),
    indexes_after(),
    layer_after(layer) {

    qSort(this->indexes_before);
  }

  void undo() override {

    get_map().undo_set_entities_layer(indexes_after, indexes_before);
    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_before);
  }

  void redo() override {

    QList<int> layers_after;
    for (int i = 0; i < indexes_before.size(); ++i) {
      layers_after << layer_after;
    }
    indexes_after = get_map().set_entities_layer(indexes_before, layers_after);
    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_after);
  }

private:
  EntityIndexes indexes_before;  // Sorted indexes before the change.
  EntityIndexes indexes_after;  // Indexes after the change, in the same order as before.
  int layer_after;
};

/**
 * @brief Moving entities one layer up.
 */
class IncreaseEntitiesLayerCommand : public MapEditorCommand {

public:
  IncreaseEntitiesLayerCommand(MapEditor& editor, const EntityIndexes& indexes) :
    MapEditorCommand(editor, MapEditor::tr("Increment layer")),
    indexes_before(indexes),
    indexes_after() {

    qSort(this->indexes_before);
  }

  void undo() override {

    get_map().undo_set_entities_layer(indexes_after, indexes_before);
    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_before);
  }

  void redo() override {

    QList<int> layers_after;
    for (const EntityIndex& index_before : indexes_before) {
      int layer_after = std::min(index_before.layer + 1, get_map().get_max_layer());
      layers_after << layer_after;
    }
    indexes_after = get_map().set_entities_layer(indexes_before, layers_after);
    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_after);
  }

private:
  EntityIndexes indexes_before;  // Sorted indexes before the change.
  EntityIndexes indexes_after;  // Indexes after the change, in the same order as before.
};

/**
 * @brief Moving entities one layer down.
 */
class DecreaseEntitiesLayerCommand : public MapEditorCommand {

public:
  DecreaseEntitiesLayerCommand(MapEditor& editor, const EntityIndexes& indexes) :
    MapEditorCommand(editor, MapEditor::tr("Decrement layer")),
    indexes_before(indexes),
    indexes_after() {

    qSort(this->indexes_before);
  }

  void undo() override {

    get_map().undo_set_entities_layer(indexes_after, indexes_before);
    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_before);
  }

  void redo() override {

    QList<int> layers_after;
    for (const EntityIndex& index_before : indexes_before) {
      int layer_after = std::max(index_before.layer - 1, get_map().get_min_layer());
      layers_after << layer_after;
    }
    indexes_after = get_map().set_entities_layer(indexes_before, layers_after);
    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_after);
  }

private:
  EntityIndexes indexes_before;  // Sorted indexes before the change.
  EntityIndexes indexes_after;  // Indexes after the change, in the same order as before.
};

/**
 * @brief Bringing some entities to the front.
 */
class BringEntitiesToFrontCommand : public MapEditorCommand {

public:
  BringEntitiesToFrontCommand(MapEditor& editor, const EntityIndexes& indexes) :
    MapEditorCommand(editor, MapEditor::tr("Bring to front")),
    indexes_before(indexes),
    indexes_after(indexes) {

    qSort(this->indexes_before);
  }

  void undo() override {

    MapModel& map = get_map();
    QList<EntityModel*> entities;
    for (const EntityIndex& index_after: indexes_after) {
      entities.append(&map.get_entity(index_after));
    }

    int i = 0;
    for (const EntityModel* entity : entities) {
      map.set_entity_order(entity->get_index(), indexes_before[i].order);
      ++i;
    }

    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_before);
  }

  void redo() override {

    MapModel& map = get_map();
    QList<EntityModel*> entities;
    for (const EntityIndex& index_before: indexes_before) {
      entities.append(&map.get_entity(index_before));
    }

    for (const EntityModel* entity : entities) {
      map.bring_entity_to_front(entity->get_index());
    }

    indexes_after.clear();
    for (const EntityModel* entity : entities) {
      indexes_after.append(entity->get_index());
    }

    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_after);
  }

private:
  EntityIndexes indexes_before;  // Sorted indexes before the change.
  EntityIndexes indexes_after;  // Indexes after the change, in the same order as before.
};

/**
 * @brief Bringing some entities to the back.
 */
class BringEntitiesToBackCommand : public MapEditorCommand {

public:
  BringEntitiesToBackCommand(MapEditor& editor, const EntityIndexes& indexes) :
    MapEditorCommand(editor, MapEditor::tr("Bring to back")),
    indexes_before(indexes),
    indexes_after(indexes) {

    qSort(this->indexes_before);
  }

  void undo() override {

    MapModel& map = get_map();
    QList<EntityModel*> entities;
    for (const EntityIndex& index_after: indexes_after) {
      entities.append(&map.get_entity(index_after));
    }

    for (int i = entities.size() - 1; i >= 0; --i) {
      EntityModel* entity = entities[i];
      map.set_entity_order(entity->get_index(), indexes_before[i].order);
    }

    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_before);
  }

  void redo() override {

    MapModel& map = get_map();
    QList<EntityModel*> entities;
    for (const EntityIndex& index_before: indexes_before) {
      entities.append(&map.get_entity(index_before));
    }

    // Iterate from the end to preserve the relative order of entities.
    for (auto it = entities.end(); it != entities.begin();) {
      --it;
      EntityModel* entity = *it;
      map.bring_entity_to_back(entity->get_index());
    }

    indexes_after.clear();
    for (const EntityModel* entity : entities) {
      indexes_after.append(entity->get_index());
    }

    // Select impacted entities.
    get_map_view().set_selected_entities(indexes_after);
  }

private:
  EntityIndexes indexes_before;  // Sorted indexes before the change.
  EntityIndexes indexes_after;  // Indexes after the change, in the same order as before.
};

/**
 * @brief Adding entities to the map.
 */
class AddEntitiesCommand : public MapEditorCommand {

public:
  AddEntitiesCommand(MapEditor& editor, AddableEntities&& entities, bool replace_selection) :
    MapEditorCommand(editor, MapEditor::tr("Add entities")),
    entities(std::move(entities)),
    indexes(),
    previous_selected_indexes() {

    std::sort(this->entities.begin(), this->entities.end());

    // Store indexes alone in a separate list (useful for undo and for selection).
    for (const AddableEntity& entity : this->entities) {
      indexes.append(entity.index);
    }

    if (!replace_selection) {
      previous_selected_indexes = get_map_view().get_selected_entities();
    }
  }

  void undo() override {
    // Remove entities that were added, keep them in this class.
    entities = get_map().remove_entities(indexes);
    get_map_view().set_selected_entities(previous_selected_indexes);
  }

  void redo() override {
    // Add entities and make them selected.
    get_map().add_entities(std::move(entities));

    EntityIndexes selected_indexes = indexes;
    for (const EntityIndex& index : previous_selected_indexes) {
      selected_indexes.append(index);
    }
    get_map_view().set_selected_entities(selected_indexes);
  }

private:
  AddableEntities entities;    // Entities to be added and where (sorted).
  EntityIndexes indexes;  // Indexes where they should be added (redundant info).
  EntityIndexes previous_selected_indexes;  // Selection to keep after adding entities.
};

/**
 * @brief Removing entities from the map.
 */
class RemoveEntitiesCommand : public MapEditorCommand {

public:
  RemoveEntitiesCommand(MapEditor& editor, const EntityIndexes& indexes) :
    MapEditorCommand(editor, MapEditor::tr("Delete entities")),
    entities(),
    indexes(indexes) {

    std::sort(this->indexes.begin(), this->indexes.end());
  }

  void undo() override {
    // Restore entities with their old index.
    get_map().add_entities(std::move(entities));
    get_map_view().set_selected_entities(indexes);
  }

  void redo() override {
    // Remove entities from the map, keep them and their index in this class.
    entities = get_map().remove_entities(indexes);
  }

private:
  AddableEntities entities;    // Entities to remove and their indexes before removal (sorted).
  EntityIndexes indexes;  // Indexes before removal (redundant info).
};

}  // Anonymous namespace.

/**
 * @brief Creates a map editor.
 * @param quest The quest containing the file.
 * @param path Path of the map data file to open.
 * @param parent The parent object or nullptr.
 * @throws EditorException If the file could not be opened.
 */
MapEditor::MapEditor(Quest& quest, const QString& path, QWidget* parent) :
  Editor(quest, path, parent),
  map_id(),
  map(nullptr),
  entity_creation_toolbar(nullptr),
  status_bar(nullptr) {

  ui.setupUi(this);
  build_entity_creation_toolbar();
  build_status_bar();

  // Get the map.
  ResourceType resource_type;
  QString map_id;
  quest.check_exists(path);
  if (!quest.is_resource_element(path, resource_type, map_id) ||
      resource_type != ResourceType::MAP) {
    throw EditorException(tr("File '%1' is not a map").arg(path));
  }
  this->map_id = map_id;

  // Editor properties.
  set_title(tr("Map %1").arg(get_file_name_without_extension()));
  set_icon(QIcon(":/images/icon_resource_map.png"));
  set_close_confirm_message(
        tr("Map '%1' has been modified. Save changes?").arg(map_id));
  set_select_all_supported(true);
  set_zoom_supported(true);
  set_grid_supported(true);
  set_traversables_visibility_supported(true);
  set_obstacles_visibility_supported(true);
  set_entity_type_visibility_supported(true);
  set_export_to_image_supported(true);

  // Shortcuts.
  QAction* open_script_action = new QAction(this);
  open_script_action->setShortcut(tr("F4"));
  open_script_action->setShortcutContext(Qt::WindowShortcut);
  connect(open_script_action, SIGNAL(triggered(bool)),
          this, SLOT(open_script_requested()));
  addAction(open_script_action);

  // Open the file.
  map = new MapModel(quest, map_id, this);
  get_undo_stack().setClean();

  // Prepare the gui.
  const int side_width = 350;
  ui.splitter->setSizes({ side_width, width() - side_width });
  ui.map_side_splitter->setStretchFactor(0, 0);  // Don't expand the map properties view
  ui.map_side_splitter->setStretchFactor(1, 1);  // but only the tileset view.
  ui.tileset_field->set_resource_type(ResourceType::TILESET);
  ui.tileset_field->set_quest(quest);
  ui.music_field->set_quest(quest);
  ui.music_field->get_selector().add_special_value("none", tr("<No music>"), 0);
  ui.music_field->get_selector().add_special_value("same", tr("<Same as before>"), 1);
  ui.tileset_view->set_read_only(true);
  ui.map_view->set_map(map);
  ui.map_view->set_view_settings(get_view_settings());
  ui.map_view->set_common_actions(&get_common_actions());

  ui.tileset_view->set_view_settings(tileset_view_settings);

  ui.size_field->config("x", 0, 99999, 8);
  ui.size_field->set_tooltips(
    tr("Width of the map in pixels"),
    tr("Height of the map in pixels"));

  ui.location_field->config(",", 0, 99999, 8);
  ui.location_field->set_tooltips(
    tr("Coordinates of the map in its world (useful to make adjacent scrolling maps)"),
    tr("Coordinates of the map in its world (useful to make adjacent scrolling maps)"));

  set_layers_supported(map->get_min_layer(), map->get_max_layer());

  update();
  load_settings();

  // Make connections.
  connect(&get_database(), SIGNAL(element_description_changed(ResourceType, QString, QString)),
          this, SLOT(update_description_to_gui()));
  connect(ui.description_field, SIGNAL(editingFinished()),
          this, SLOT(set_description_from_gui()));

  connect(ui.size_field, SIGNAL(editing_finished()),
          this, SLOT(change_size_requested()));
  connect(map, SIGNAL(size_changed(QSize)),
          this, SLOT(update_size_field()));

  connect(ui.min_layer_field, SIGNAL(editingFinished()),
          this, SLOT(change_min_layer_requested()));
  connect(ui.max_layer_field, SIGNAL(editingFinished()),
          this, SLOT(change_max_layer_requested()));
  connect(map, SIGNAL(layer_range_changed(int, int)),
          this, SLOT(layer_range_changed()));

  connect(ui.world_check_box, SIGNAL(stateChanged(int)),
          this, SLOT(world_check_box_changed()));
  connect(ui.world_field, SIGNAL(editingFinished()),
          this, SLOT(change_world_requested()));
  connect(map, SIGNAL(world_changed(QString)),
          this, SLOT(update_world_field()));

  connect(ui.floor_check_box, SIGNAL(stateChanged(int)),
          this, SLOT(floor_check_box_changed()));
  connect(ui.floor_field, SIGNAL(editingFinished()),
          this, SLOT(change_floor_requested()));
  connect(map, SIGNAL(floor_changed(int)),
          this, SLOT(update_floor_field()));

  connect(ui.location_field, SIGNAL(editing_finished()),
          this, SLOT(change_location_requested()));
  connect(map, SIGNAL(location_changed(QPoint)),
          this, SLOT(update_location_field()));

  connect(ui.tileset_field, SIGNAL(activated(QString)),
          this, SLOT(tileset_selector_activated()));
  connect(map, SIGNAL(tileset_id_changed(QString)),
          this, SLOT(tileset_id_changed(QString)));
  connect(ui.tileset_refresh_button, SIGNAL(clicked()),
          this, SLOT(refresh_tileset_requested()));
  connect(ui.tileset_edit_button, SIGNAL(clicked()),
          this, SLOT(open_tileset_requested()));

  connect(ui.music_field, SIGNAL(activated(QString)),
          this, SLOT(music_selector_activated()));
  connect(map, SIGNAL(music_id_changed(QString)),
          this, SLOT(update_music_field()));

  connect(ui.current_border_sets_selector, SIGNAL(activated(QString)),
          this, SLOT(border_set_selector_activated()));

  connect(ui.open_script_button, SIGNAL(clicked()),
          this, SLOT(open_script_requested()));

  connect(ui.map_view, SIGNAL(edit_entity_requested(EntityIndex, EntityModelPtr&)),
          this, SLOT(edit_entity_requested(EntityIndex, EntityModelPtr&)));
  connect(ui.map_view, SIGNAL(move_entities_requested(EntityIndexes, QPoint, bool)),
          this, SLOT(move_entities_requested(EntityIndexes, QPoint, bool)));
  connect(ui.map_view, SIGNAL(resize_entities_requested(QMap<EntityIndex, QRect>, bool)),
          this, SLOT(resize_entities_requested(QMap<EntityIndex, QRect>, bool)));
  connect(ui.map_view, SIGNAL(convert_tiles_requested(EntityIndexes)),
          this, SLOT(convert_tiles_requested(EntityIndexes)));
  connect(ui.map_view, SIGNAL(change_tiles_pattern_requested(EntityIndexes)),
          this, SLOT(change_tiles_pattern_requested(EntityIndexes)));
  connect(ui.map_view, SIGNAL(set_entities_direction_requested(EntityIndexes, int)),
          this, SLOT(set_entities_direction_requested(EntityIndexes, int)));
  connect(ui.map_view, SIGNAL(set_entities_layer_requested(EntityIndexes, int)),
          this, SLOT(set_entities_layer_requested(EntityIndexes, int)));
  connect(ui.map_view, SIGNAL(increase_entities_layer_requested(EntityIndexes)),
          this, SLOT(increase_entities_layer_requested(EntityIndexes)));
  connect(ui.map_view, SIGNAL(decrease_entities_layer_requested(EntityIndexes)),
          this, SLOT(decrease_entities_layer_requested(EntityIndexes)));
  connect(ui.map_view, SIGNAL(bring_entities_to_front_requested(EntityIndexes)),
          this, SLOT(bring_entities_to_front_requested(EntityIndexes)));
  connect(ui.map_view, SIGNAL(bring_entities_to_back_requested(EntityIndexes)),
          this, SLOT(bring_entities_to_back_requested(EntityIndexes)));
  connect(ui.map_view, SIGNAL(add_entities_requested(AddableEntities&, bool)),
          this, SLOT(add_entities_requested(AddableEntities&, bool)));
  connect(ui.map_view, SIGNAL(remove_entities_requested(EntityIndexes)),
          this, SLOT(remove_entities_requested(EntityIndexes)));
  connect(ui.map_view, SIGNAL(stopped_state()),
          this, SLOT(uncheck_entity_creation_buttons()));
  connect(ui.map_view, SIGNAL(undo_requested()),
          this, SLOT(undo()));

  connect(ui.map_view->get_scene(), SIGNAL(selectionChanged()),
          this, SLOT(map_selection_changed()));
}

/**
 * @brief Returns the map model being edited.
 * @return The map model.
 */
MapModel& MapEditor::get_map() {
  return *map;
}

/**
 * @brief Returns the map graphics view.
 * @return The graphics view.
 */
MapView& MapEditor::get_map_view() {
  return *ui.map_view;
}

/**
 * @brief Initializes the entity creation toolbar.
 *
 * The entity creation toolbar is not made with Qt designer because
 * - one cannot create QToolBar widgets with Qt designer,
 * - we iterate on entity types to build all of them more easily.
 */
void MapEditor::build_entity_creation_toolbar() {

  entity_creation_toolbar = new QToolBar(this);

  // List of types proposed in the toolbar.
  // The list is specified here manually because we want to control the order
  // and all types are not included (dynamic tiles are omitted).
  const std::vector<std::pair<EntityType, QString>> types_in_toolbar = {
    { EntityType::TILE, tr("Add tile") },
    { EntityType::DESTINATION, tr("Add destination") },
    { EntityType::TELETRANSPORTER, tr("Add teletransporter") },
    { EntityType::PICKABLE, tr("Add pickable") },
    { EntityType::DESTRUCTIBLE, tr("Add destructible") },
    { EntityType::CHEST, tr("Add chest") },
    { EntityType::JUMPER, tr("Add jumper") },
    { EntityType::ENEMY, tr("Add enemy") },
    { EntityType::NPC, tr("Add non-playing character") },
    { EntityType::BLOCK, tr("Add block") },
    { EntityType::SWITCH, tr("Add switch") },
    { EntityType::WALL, tr("Add wall") },
    { EntityType::SENSOR, tr("Add sensor") },
    { EntityType::CRYSTAL, tr("Add crystal") },
    { EntityType::CRYSTAL_BLOCK, tr("Add crystal block") },
    { EntityType::SHOP_TREASURE, tr("Add shop treasure") },
    { EntityType::STREAM, tr("Add stream") },
    { EntityType::DOOR, tr("Add door") },
    { EntityType::STAIRS, tr("Add stairs") },
    { EntityType::SEPARATOR, tr("Add separator") },
    { EntityType::CUSTOM, tr("Add custom entity") }
  };

  for (const auto& pair : types_in_toolbar) {
    EntityType type = pair.first;
    QString text = pair.second;
    QString icon_name = ":/images/entity_" + EntityTraits::get_lua_name(type) + ".png";
    QAction* action = new QAction(QIcon(icon_name), text, nullptr);
    action->setCheckable(true);
    action->setData(static_cast<int>(type));
    entity_creation_toolbar->addAction(action);
    connect(action, &QAction::triggered, [this, type](bool checked) {
      entity_creation_button_triggered(type, checked);
    });
  }
  entity_creation_toolbar->setIconSize(QSize(32, 32));
  entity_creation_toolbar->setStyleSheet("spacing: 0");

  ui.entity_creation_layout->insertWidget(0, entity_creation_toolbar);
}

/**
 * @brief Creates a status bar in the map view.
 *
 * The status bar is not made with Qt designer because
 * one cannot create QStatusBar widgets with Qt designer.
 */
void MapEditor::build_status_bar() {

  status_bar = new QStatusBar();
  ui.entity_creation_layout->addWidget(status_bar);

  connect(ui.map_view, SIGNAL(mouse_map_coordinates_changed(QPoint)),
          this, SLOT(update_status_bar()));
  connect(ui.map_view, SIGNAL(mouse_left()),
          this, SLOT(update_status_bar()));
}

/**
 * @copydoc Editor::save
 */
void MapEditor::save() {

  map->save();
}

/**
 * @copydoc Editor::export_to_image
 */
void MapEditor::export_to_image() {

  QString map_id_without_dirs = map->get_map_id().section("/", -1, -1);
  const QString& file_name = QFileDialog::getSaveFileName(
        this,
        tr("Save map as PNG file"),
        QString("%1/%2.png").arg(get_quest().get_root_path(), map_id_without_dirs),
        tr("PNG image (*.png)"));

  if (!file_name.isEmpty()) {
    ui.map_view->export_to_image(file_name);
  }
}

/**
 * @copydoc Editor::can_cut
 */
bool MapEditor::can_cut() const {
  return can_copy();
}

/**
 * @copydoc Editor::cut
 */
void MapEditor::cut() {

  ui.map_view->cut();
}

/**
 * @copydoc Editor::can_copy
 */
bool MapEditor::can_copy() const {

  return !ui.map_view->is_selection_empty();
}

/**
 * @copydoc Editor::copy
 */
void MapEditor::copy() {

  ui.map_view->copy();
}

/**
 * @copydoc Editor::can_paste
 */
bool MapEditor::can_paste() const {
  return true;
}

/**
 * @copydoc Editor::paste
 */
void MapEditor::paste() {

  ui.map_view->paste();
}

/**
 * @copydoc Editor::select_all
 */
void MapEditor::select_all() {

  MapScene* scene = ui.map_view->get_scene();
  if (scene != nullptr) {
    scene->select_all_except_locked();
  }
}

/**
 * @copydoc Editor::unselect_all
 */
void MapEditor::unselect_all() {

  MapScene* scene = ui.map_view->get_scene();
  if (scene != nullptr) {
    scene->unselect_all();
  }
}

/**
 * @copydoc Editor::reload_settings
 */
void MapEditor::reload_settings() {

  EditorSettings settings;

  MapScene* main_scene = ui.map_view->get_scene();
  if (main_scene != nullptr) {
    QBrush brush(settings.get_value_color(EditorSettings::map_main_background));
    main_scene->setBackgroundBrush(brush);
  }

  TilesetScene* tileset_scene = ui.tileset_view->get_scene();
  if (tileset_scene != nullptr) {
    QBrush brush(
      settings.get_value_color(EditorSettings::map_tileset_background));
    tileset_scene->setBackgroundBrush(brush);
  }

  get_view_settings().set_grid_style(static_cast<GridStyle>(
    settings.get_value_int(EditorSettings::map_grid_style)));
  get_view_settings().set_grid_color(
    settings.get_value_color(EditorSettings::map_grid_color));
}

/**
 * @brief Updates everything in the gui.
 */
void MapEditor::update() {

  update_map_id_field();
  update_description_to_gui();
  update_size_field();
  update_min_layer_field();
  update_max_layer_field();
  update_world_field();
  update_floor_field();
  update_location_field();
  update_tileset_field();
  update_music_field();
  tileset_id_changed(map->get_tileset_id());
}

/**
 * @brief Updates the map id displaying.
 */
void MapEditor::update_map_id_field() {

  ui.map_id_field->setText(map_id);
}

/**
 * @brief Slot called when the user wants to open the map script.
 */
void MapEditor::open_script_requested() {

  emit open_file_requested(
    get_quest(), get_quest().get_map_script_path(map->get_map_id()));
}

/**
 * @brief Updates the content of the map description text edit.
 */
void MapEditor::update_description_to_gui() {

  QString description = get_database().get_description(
    ResourceType::MAP, map_id);
  if (ui.description_field->text() != description) {
    ui.description_field->setText(description);
  }
}

/**
 * @brief Modifies the map description in the quest resource list with
 * the new text entered by the user.
 *
 * If the new description is invalid, an error dialog is shown.
 */
void MapEditor::set_description_from_gui() {

  QString description = ui.description_field->text();
  if (description == get_database().get_description(ResourceType::MAP, map_id)) {
    return;
  }

  if (description.isEmpty()) {
    GuiTools::error_dialog(tr("Invalid description"));
    update_description_to_gui();
    return;
  }

  const bool was_blocked = blockSignals(true);
  try {
    get_database().set_description(ResourceType::MAP, map_id, description);
    get_database().save();
  }
  catch (const EditorException& ex) {
    ex.print_message();
  }
  update_description_to_gui();
  blockSignals(was_blocked);
}

/**
 * @brief Updates the size field with the data from the model.
 */
void MapEditor::update_size_field() {

  ui.size_field->set_size(map->get_size());
}

/**
 * @brief Modifies the map size with new values entered by the user.
 */
void MapEditor::change_size_requested() {

  QSize size = ui.size_field->get_size();
  if (size == map->get_size()) {
    return;
  }
  try_command(new SetSizeCommand(*this, size));
}

/**
 * @brief Updates the minimum layer field with the data from the model.
 */
void MapEditor::update_min_layer_field() {

  ui.min_layer_field->setValue(map->get_min_layer());
}

/**
 * @brief Modifies the minimum layer with new values entered by the user.
 */
void MapEditor::change_min_layer_requested() {

  int min_layer = ui.min_layer_field->value();

  if (min_layer == map->get_min_layer()) {
    return;
  }

  if (min_layer > map->get_min_layer()) {
    // Reducing the number of layers: ask the user confirmation if entities are removed.
    int num_entities_removed = 0;
    for (int layer = map->get_min_layer(); layer < min_layer; ++layer) {
      num_entities_removed += map->get_num_entities(layer);
    }
    if (num_entities_removed > 0) {
      // Block spinbox signals to avoid reentrant calls to this function
      // (because the dialog box takes focus from the spinbox, trigerring
      // its signal again).
      const bool was_blocked = ui.min_layer_field->signalsBlocked();
      ui.min_layer_field->blockSignals(true);

      QMessageBox::StandardButton answer = QMessageBox::warning(
          this,
          tr("Layer not empty"),
          tr("This layer is not empty: %1 entities will be destroyed.").arg(num_entities_removed),
          QMessageBox::Ok | QMessageBox::Cancel,
          QMessageBox::Ok
      );

      ui.min_layer_field->blockSignals(was_blocked);

      if (answer == QMessageBox::Cancel) {
        update_min_layer_field();
        return;
      }
    }
  }

  try_command(new SetMinLayerCommand(*this, min_layer));
}

/**
 * @brief Updates the maximum layer field with the data from the model.
 */
void MapEditor::update_max_layer_field() {

  ui.max_layer_field->setValue(map->get_max_layer());
}

/**
 * @brief Updates the UI when the range of layers in the model has changed.
 */
void MapEditor::layer_range_changed() {

  const int min_layer = get_map().get_min_layer();
  const int max_layer = get_map().get_max_layer();

  // Update the spinboxes.
  update_min_layer_field();
  update_max_layer_field();

  // Notify the editor.
  set_layers_supported(min_layer, max_layer);

  // Notify the view settings.
  get_view_settings().set_layer_range(min_layer, max_layer);
}

/**
 * @brief Modifies the maximum layer with new values entered by the user.
 */
void MapEditor::change_max_layer_requested() {

  int max_layer = ui.max_layer_field->value();

  if (max_layer == map->get_max_layer()) {
    return;
  }

  if (max_layer < map->get_max_layer()) {
    // Reducing the number of layers: ask the user confirmation if entities are removed.
    int num_entities_removed = 0;
    for (int layer = max_layer + 1; layer <= map->get_max_layer(); ++layer) {
      num_entities_removed += map->get_num_entities(layer);
    }
    if (num_entities_removed > 0) {
      // Block spinbox signals to avoid reentrant calls to this function
      // (because the dialog box takes focus from the spinbox, trigerring
      // its signal again).
      const bool was_blocked = ui.max_layer_field->signalsBlocked();
      ui.max_layer_field->blockSignals(true);

      QMessageBox::StandardButton answer = QMessageBox::warning(
          this,
          tr("Layer not empty"),
          tr("This layer is not empty: %1 entities will be destroyed.").arg(num_entities_removed),
          QMessageBox::Ok | QMessageBox::Cancel,
          QMessageBox::Ok
      );

      ui.max_layer_field->blockSignals(was_blocked);

      if (answer == QMessageBox::Cancel) {
        update_max_layer_field();
        return;
      }
    }
  }

  try_command(new SetMaxLayerCommand(*this, max_layer));
}

/**
 * @brief Updates the world field with the data from the model.
 */
void MapEditor::update_world_field() {

  const QString& world = map->get_world();
  if (world.isEmpty()) {
    ui.world_check_box->setChecked(false);
    ui.world_field->setEnabled(false);
  }
  else {
    ui.world_check_box->setChecked(true);
    ui.world_field->setEnabled(true);
    ui.world_field->setText(world);
  }
}

/**
 * @brief Slot called when the user clicks the "Set a world" checkbox.
 */
void MapEditor::world_check_box_changed() {

  bool checked = ui.world_check_box->isChecked();
  if (checked) {
    ui.world_field->setEnabled(true);
    if (!map->has_world() &&
        !ui.world_field->text().isEmpty()) {
      // Use the text that was still in the disabled field.
      try_command(new SetWorldCommand(*this, ui.world_field->text()));
    }
  }
  else {
    ui.world_field->setEnabled(false);
    if (map->has_world()) {
      // Remove the world but keep the text in the field.
      try_command(new SetWorldCommand(*this, ""));
    }
  }
}

/**
 * @brief Changes the world value with the new text entered by the user.
 */
void MapEditor::change_world_requested() {

  QString world = ui.world_field->text();
  if (world == map->get_world()) {
    return;
  }
  try_command(new SetWorldCommand(*this, world));
}

/**
 * @brief Updates the floor field with the data from the model.
 */
void MapEditor::update_floor_field() {

  int floor = map->get_floor();
  if (floor == MapModel::NO_FLOOR) {
    ui.floor_check_box->setChecked(false);
    ui.floor_field->setEnabled(false);
  }
  else {
    ui.floor_check_box->setChecked(true);
    ui.floor_field->setEnabled(true);
    ui.floor_field->setValue(floor);
  }
}

/**
 * @brief Slot called when the user clicks the "Set a floor" checkbox.
 */
void MapEditor::floor_check_box_changed() {

  bool checked = ui.floor_check_box->isChecked();
  if (checked) {
    ui.floor_field->setEnabled(true);
    if (!map->has_floor()) {
      // Use the value that was still in the disabled field.
      try_command(new SetFloorCommand(*this, ui.floor_field->value()));
    }
  }
  else {
    ui.floor_field->setEnabled(false);
    if (map->has_floor()) {
      // Remove the floor but keep the value in the field.
      try_command(new SetFloorCommand(*this, MapModel::NO_FLOOR));
    }
  }
}

/**
 * @brief Changes the floor value with the new text entered by the user.
 */
void MapEditor::change_floor_requested() {

  int floor = ui.floor_field->value();
  if (floor == map->get_floor()) {
    return;
  }
  try_command(new SetFloorCommand(*this, floor));
}

/**
 * @brief Updates the location field with the data from the model.
 */
void MapEditor::update_location_field() {

  ui.location_field->set_point(map->get_location());
}

/**
 * @brief Modifies the map location with new values entered by the user.
 */
void MapEditor::change_location_requested() {

  QPoint location = ui.location_field->get_point();
  if (location == map->get_location()) {
    return;
  }
  try_command(new SetLocationCommand(*this, location));
}

/**
 * @brief Updates the tileset selector with the data from the model.
 */
void MapEditor::update_tileset_field() {

  ui.tileset_field->set_selected_id(map->get_tileset_id());
}

/**
 * @brief Slot called when the user changes the tileset in the selector.
 */
void MapEditor::tileset_selector_activated() {

  const QString& old_tileset_id = map->get_tileset_id();
  const QString& new_tileset_id = ui.tileset_field->get_selected_id();
  if (new_tileset_id == old_tileset_id) {
    // No change.
    return;
  }

  try_command(new SetTilesetCommand(*this, new_tileset_id));
}

/**
 * @brief Slot called when the user wants to refresh the selected tileset.
 */
void MapEditor::refresh_tileset_requested() {

  // Refresh the map model.
  get_map().reload_tileset();

  // Rebuild the tileset view.
  update_tileset_view();
}

/**
 * @brief Slot called when the user wants to open the selected tileset.
 */
void MapEditor::open_tileset_requested() {

  emit open_file_requested(
        get_quest(), get_quest().get_tileset_data_file_path(map->get_tileset_id()));
}

/**
 * @brief Updates the music selector with the data from the model.
 */
void MapEditor::update_music_field() {

  // Update the music selector.
  const QString& music_id = map->get_music_id();
  ui.music_field->set_selected_id(music_id);
}

/**
 * @brief Slot called when the user changes the music in the selector.
 */
void MapEditor::music_selector_activated() {

  const QString& old_music_id = map->get_music_id();
  const QString& new_music_id = ui.music_field->get_selected_id();
  if (new_music_id == old_music_id) {
    // No change.
    return;
  }

  // Stop playing any music if there is a change.
  Audio::stop_music(get_quest());

  try_command(new SetMusicCommand(*this, new_music_id));
}

/**
 * @brief Updates the content of the tileset view.
 */
void MapEditor::update_tileset_view() {

  TilesetModel* tileset = map->get_tileset_model();
  ui.tileset_view->set_model(tileset);
}

/**
 * @brief Updates the content of the border set panel.
 */
void MapEditor::update_border_set_view() {

  const QString& tileset_id = map->get_tileset_id();
  if (ui.current_border_sets_selector->get_tileset_id() == tileset_id) {
    // No change.
    return;
  }
  ui.current_border_sets_selector->set_tileset_id(get_quest(), tileset_id);
  ui.current_border_sets_selector->build();

  ui.current_border_sets_selector->set_selected_border_set_id(map->get_current_border_set_id());

  if (ui.current_border_sets_selector->get_selected_border_set_id() != map->get_current_border_set_id()) {
    map->set_current_border_set_id(ui.current_border_sets_selector->get_selected_border_set_id());
  }
}

/**
 * @brief Slot called when the user changes the current border set selected.
 */
void MapEditor::border_set_selector_activated() {

  const QString& old_border_set_id = map->get_current_border_set_id();
  const QString& new_border_set_id = ui.current_border_sets_selector->get_selected_border_set_id();
  if (new_border_set_id == old_border_set_id) {
    // No change.
    return;
  }

  map->set_current_border_set_id(new_border_set_id);
}

/**
 * @brief Slot called when another tileset is set on the map.
 * @param tileset_id The new tileset id.
 */
void MapEditor::tileset_id_changed(const QString& tileset_id) {

  Q_UNUSED(tileset_id);

  // Show the correct tileset in the combobox.
  update_tileset_field();

  // Notify the tileset view.
  update_tileset_view();

  // Notify the border sets view.
  update_border_set_view();

  // Watch the pattern selection of the tileset view to correctly add new tiles.
  connect(ui.tileset_view, SIGNAL(selection_changed_by_user()),
          this, SLOT(tileset_selection_changed()));
}

/**
 * @brief Slot called when the user changes the patterns selection in the tileset view.
 */
void MapEditor::tileset_selection_changed() {

  uncheck_entity_creation_buttons();
  ui.map_view->tileset_selection_changed();
}

/**
 * @brief Slot called when the user changes the selection in the map.
 */
void MapEditor::map_selection_changed() {

  // Update whether cut/copy are available.
  bool empty_selection = ui.map_view->is_selection_empty();
  can_cut_changed(!empty_selection);
  can_copy_changed(!empty_selection);

  // Nofify the tileset view of selected tile patterns.
  TilesetModel* tileset = ui.tileset_view->get_model();
  if (tileset != nullptr) {
    const EntityIndexes& entity_indexes = ui.map_view->get_selected_entities();
    MapModel& map = get_map();
    QList<int> pattern_indexes;
    for (const EntityIndex& entity_index : entity_indexes) {
      QString pattern_id = map.get_entity_field(entity_index, "pattern").toString();
      if (!pattern_id.isEmpty()) {
        pattern_indexes << tileset->id_to_index(pattern_id);
      }
    }
    tileset->set_selected_indexes(pattern_indexes);
  }
}

/**
 * @brief Updates existing teletransporters in all maps when a destination
 * of this map is renamed.
 * @param command The edit entity command that changes a destination's name.
 * @param name_before The old destination name.
 * @param name_after The new destination name.
 */
void MapEditor::refactor_destination_name(
    QUndoCommand* command,
    const QString& name_before,
    const QString& name_after
) {
  Refactoring refactoring([=]() {

    // Perform the entity edition.
    if (!try_command(command)) {
      return QStringList();
    }

    // Update teletransporters in this map.
    const EntityIndexes& teletransporter_indexes =
        map->find_entities_of_type(EntityType::TELETRANSPORTER);
    for (const EntityIndex& index : teletransporter_indexes) {
      const QString& destination_map_id =
          map->get_entity_field(index, "destination_map").toString();
      const QString& destination_name =
          map->get_entity_field(index, "destination").toString();

      if (destination_map_id == this->map_id &&
          destination_name == name_before) {
        map->set_entity_field(index, "destination", name_after);
      }
    }

    // Save the map and clear the undo history.
    save();
    get_undo_stack().clear();

    // Update teletransporters in all other maps.
    return update_destination_name_in_other_maps(name_before, name_after);
  });

  refactoring.set_file_unsaved_allowed(get_file_path(), true);

  emit refactoring_requested(refactoring);
}

/**
 * @brief Updates existing teletransporters in other maps when a destination
 * of this map is renamed.
 * @param name_before The old destination name.
 * @param name_after The new destination name.
 * @return The list of files that were modified.
 * @throws EditorException In case of error.
 */
QStringList MapEditor::update_destination_name_in_other_maps(
    const QString& name_before,
    const QString& name_after
) {
  QStringList modified_paths;
  const QStringList& map_ids = get_database().get_elements(ResourceType::MAP);
  for (const QString& map_id : map_ids) {
    if (map_id != this->map_id) {
      if (update_destination_name_in_map(map_id, name_before, name_after)) {
        modified_paths << get_quest().get_map_data_file_path(map_id);
      }
    }
  }
  return modified_paths;
}

/**
 * @brief Updates existing teletransporters in a map when a destination
 * of this map is renamed.
 * @param map_id Id of the map to update.
 * @param name_before The old destination name.
 * @param name_after The new destination name.
 * @return @c true if there was a change.
 * @throws EditorException In case of error.
 */
bool MapEditor::update_destination_name_in_map(
    const QString& map_id,
    const QString& name_before,
    const QString& name_after
) {
  // We don't load the entire map with all its entities for performance.
  // Instead, we just find and replace the appropriate text in the map
  // data file.

  QString path = get_quest().get_map_data_file_path(map_id);

  QString pattern = QString(
        "\n  destination_map = \"?%1\"?,\n"
        "  destination = \"%2\",\n").arg(
        QRegularExpression::escape(this->map_id), QRegularExpression::escape(name_before));

  QString replacement;
  if (name_after.isEmpty()) {
    replacement = QString(
        "\n  destination_map = \"%1\",\n").arg(this->map_id);
  }
  else {
    replacement = QString(
        "\n  destination_map = \"%1\",\n"
        "  destination = \"%2\",\n").arg(
              this->map_id, name_after);
  }

  return FileTools::replace_in_file(path, QRegularExpression(pattern), replacement);
}

/**
 * @brief Shows in the status bar information about the cursor.
 */
void MapEditor::update_status_bar() {

  if (status_bar == nullptr) {
    return;
  }

  // Show mouse coordinates and information about the entity under the mouse.
  QString mouse_coordinates_string;
  QPoint view_xy = ui.map_view->mapFromGlobal(QCursor::pos());
  int layer_under_mouse = map->get_min_layer();

  QString entity_string;
  EntityIndex index = ui.map_view->get_entity_index_under_cursor();
  if (index.is_valid()) {
    QString name = map->get_entity_name(index);
    QString type_name = EntityTraits::get_friendly_name(map->get_entity_type(index));
    entity_string = tr(" - %1").arg(type_name);
    if (!name.isEmpty()) {
      entity_string += tr(": %1").arg(name);
    }
    layer_under_mouse = map->get_entity_layer(index);
  }

  if (view_xy.x() >= 0 &&
      view_xy.x() < ui.map_view->width() &&
      view_xy.y() >= 0 &&
      view_xy.y() < ui.map_view->height()) {
    QPoint map_xy = ui.map_view->mapToScene(view_xy).toPoint() - MapScene::get_margin_top_left();
    QPoint snapped_xy(Point::round_8(map_xy));
    mouse_coordinates_string = tr("%1,%2,%3 ").
        arg(snapped_xy.x()).
        arg(snapped_xy.y()).
        arg(layer_under_mouse);
  }

  QString message = mouse_coordinates_string + entity_string;
  if (message.isEmpty()) {
    status_bar->clearMessage();
  }
  else {
    status_bar->showMessage(message);
  }
}

/**
 * @brief Slot called when the user wants to edit an entity.
 * @param index Index of the entity to change.
 * @param entity_after An entity representing the new values to set.
 */
void MapEditor::edit_entity_requested(const EntityIndex& index,
                                      EntityModelPtr& entity_after) {

  const QString& name_before = map->get_entity_name(index);
  const QString& name_after = entity_after->get_name();
  bool update_teletransporters = false;

  if (!name_before.isEmpty() &&
      name_after != name_before) {
    // When renaming a destination, the user may want to update existing teletransporters.
    const EntityModel& entity_before = map->get_entity(index);
    if (entity_before.get_type() == EntityType::DESTINATION) {
      const Destination& destination = static_cast<const Destination&>(entity_before);
      update_teletransporters = destination.get_update_teletransporters();
    }
  }

  EditEntityCommand* command = new EditEntityCommand(*this, index, std::move(entity_after));
  if (!update_teletransporters) {
    try_command(command);
  }
  else {
    refactor_destination_name(command, name_before, name_after);
  }
}

/**
 * @brief Slot called when the user wants to move entities.
 * @param indexes Indexes of the entities to move.
 * @param translation XY translation to make.
 * @param allow_merge_to_previous @c true to merge this move with the previous one if any.
 */
void MapEditor::move_entities_requested(const EntityIndexes& indexes,
                                        const QPoint& translation,
                                        bool allow_merge_to_previous) {

  if (indexes.isEmpty()) {
    return;
  }

  try_command(new MoveEntitiesCommand(*this, indexes, translation, allow_merge_to_previous));
}

/**
 * @brief Slot called when the user wants to resize entities.
 * @param boxes New bounding box of each entity to change.
 * @param allow_merge_to_previous @c true to merge this resizing with the previous one if any.
 */
void MapEditor::resize_entities_requested(const QMap<EntityIndex, QRect>& boxes,
                                          bool allow_merge_to_previous) {

  if (boxes.isEmpty()) {
    return;
  }

  try_command(new ResizeEntitiesCommand(*this, boxes, allow_merge_to_previous));
}

/**
 * @brief Slot called when the user wants to convert tiles to or from dynamic ones.
 * @param indexes Indexes of the tiles or dynamic tiles to convert.
 */
void MapEditor::convert_tiles_requested(const EntityIndexes& indexes) {

  if (indexes.isEmpty()) {
    return;
  }

  const bool dynamic = map->get_entity(indexes.first()).is_dynamic();

  for (const EntityIndex& index : indexes) {
    EntityType current_type = map->get_entity_type(index);
    if (current_type != EntityType::TILE && current_type != EntityType::DYNAMIC_TILE) {
      return;
    }
    if (map->get_entity(index).is_dynamic() != dynamic) {
      return;
    }
  }

  if (dynamic) {
    try_command(new ConvertTilesFromDynamicCommand(*this, indexes));
  }
  else {
    try_command(new ConvertTilesToDynamicCommand(*this, indexes));
  }
}

/**
 * @brief Slot called when the user wants to change the pattern of some tiles.
 * @param indexes Indexes of the tiles or dynamic tiles to change.
 */
void MapEditor::change_tiles_pattern_requested(const EntityIndexes& indexes) {

  if (indexes.isEmpty()) {
    return;
  }

  QString tileset_id = map->get_entity_field(indexes.first(), "tileset").toString();
  for (const EntityIndex& index : indexes) {
    EntityType type = map->get_entity_type(index);
    if (type != EntityType::TILE && type != EntityType::DYNAMIC_TILE) {
      return;
    }
    if (map->get_entity_field(index, "tileset") != tileset_id) {
      return;
    }
  }

  if (tileset_id.isEmpty()) {
    tileset_id = map->get_tileset_id();
  }

  PatternPickerDialog dialog(*get_quest().get_tileset(tileset_id));
  int result = dialog.exec();

  if (result != QDialog::Accepted) {
    return;
  }

  QString pattern_id = dialog.get_pattern_id();

  bool pattern_changed = false;
  for (const EntityIndex& index : indexes) {
    if (pattern_id != map->get_entity_field(index, "pattern").toString()) {
      pattern_changed = true;
      break;
    }
  }
  if (!pattern_changed) {
    // No change.
    return;
  }

  try_command(new ChangeTilesPatternCommand(*this, indexes, pattern_id));
}

/**
 * @brief Slot called when the user wants to change the direction of some entities.
 * @param indexes Indexes of the entities to change.
 * @param direction The direction to set.
 */
void MapEditor::set_entities_direction_requested(const EntityIndexes& indexes,
                                                 int direction) {

  if (indexes.isEmpty()) {
    return;
  }

  int num_directions;
  QString no_direction_text;
  if (!map->is_common_direction_rules(indexes, num_directions, no_direction_text)) {
    // Incompatible direction rules.
    return;
  }

  try_command(new SetEntitiesDirectionCommand(*this, indexes, direction));
}

/**
 * @brief Slot called when the user wants to change the layer of some entities.
 * @param indexes Indexes of the entities to change.
 * @param layer The layer to set.
 */
void MapEditor::set_entities_layer_requested(const EntityIndexes& indexes,
                                             int layer) {

  if (indexes.isEmpty()) {
    return;
  }

  int common_layer = 0;
  if (map->is_common_layer(indexes, common_layer) &&
      layer == common_layer) {
    // Nothing to do.
    return;
  }

  try_command(new SetEntitiesLayerCommand(*this, indexes, layer));
}

/**
 * @brief Slot called when the user wants to bring some entities one layer up.
 * @param indexes Indexes of the entities to change.
 */
void MapEditor::increase_entities_layer_requested(const EntityIndexes& indexes) {

  if (indexes.isEmpty()) {
    return;
  }

  try_command(new IncreaseEntitiesLayerCommand(*this, indexes));
}

/**
 * @brief Slot called when the user wants to bring some entities one layer down.
 * @param indexes Indexes of the entities to change.
 */
void MapEditor::decrease_entities_layer_requested(const EntityIndexes& indexes) {

  if (indexes.isEmpty()) {
    return;
  }

  try_command(new DecreaseEntitiesLayerCommand(*this, indexes));
}

/**
 * @brief Slot called when the user wants to bring some entities to the front.
 * @param indexes Indexes of the entities to change.
 */
void MapEditor::bring_entities_to_front_requested(const EntityIndexes& indexes) {

  if (indexes.isEmpty()) {
    return;
  }

  try_command(new BringEntitiesToFrontCommand(*this, indexes));
}

/**
 * @brief Slot called when the user wants to bring some entities to the back.
 * @param indexes Indexes of the entities to change.
 */
void MapEditor::bring_entities_to_back_requested(const EntityIndexes& indexes) {

  if (indexes.isEmpty()) {
    return;
  }

  try_command(new BringEntitiesToBackCommand(*this, indexes));
}

/**
 * @brief Slot called when the user wants to add entities.
 * @param entities Entities ready to be added to the map.
 * @param replace_selection @c true to clear the previous selection.
 * Newly created entities will be selected in all cases.
 */
void MapEditor::add_entities_requested(AddableEntities& entities, bool replace_selection) {

  if (entities.empty()) {
    return;
  }

  try_command(new AddEntitiesCommand(*this, std::move(entities), replace_selection));
}

/**
 * @brief Slot called when the user wants to delete entities.
 * @param indexes Indexes of entities to remove.
 */
void MapEditor::remove_entities_requested(const EntityIndexes& indexes) {

  if (indexes.empty()) {
    return;
  }

  try_command(new RemoveEntitiesCommand(*this, indexes));
}

/**
 * @brief This function is called when the user checks or unchecks a button of
 * the entity creation toolbar.
 * @param type Type of entity corresponding to the button.
 * @param checked Whether the button is checked or unchecked.
 */
void MapEditor::entity_creation_button_triggered(EntityType type, bool checked) {

  if (checked) {
    // Create a new entity of this type.
    EntityModels entities;
    entities.emplace_back(EntityModel::create(*map, type));
    const bool guess_layer = true;
    ui.map_view->start_state_adding_entities(std::move(entities), guess_layer);

    // Unselect patterns in the tileset.
    TilesetModel* tileset = map->get_tileset_model();
    if (tileset != nullptr) {
      tileset->clear_selection();
    }

    // Uncheck other entity creation buttons.
    const QList<QAction*>& actions = entity_creation_toolbar->actions();
    for (QAction* action : actions) {
      bool ok = false;
      EntityType action_type = static_cast<EntityType>(action->data().toInt(&ok));
      if (ok) {
        bool action_checked = (action_type == type);
        action->setChecked(action_checked);
      }
    }
  }
  else {
    // Stop adding entities.
    ui.map_view->start_state_doing_nothing();
  }
}

/**
 * @brief Unchecks all buttons of the entity creation toolbar.
 */
void MapEditor::uncheck_entity_creation_buttons() {

  const QList<QAction*>& actions = entity_creation_toolbar->actions();
  for (QAction* action : actions) {
    action->setChecked(false);
  }
}

/**
 * @brief Loads settings.
 */
void MapEditor::load_settings() {

  ViewSettings& view = get_view_settings();
  EditorSettings settings;

  view.set_zoom(settings.get_value_double(EditorSettings::map_main_zoom));
  view.set_grid_visible(
    settings.get_value_bool(EditorSettings::map_grid_show_at_opening));
  view.set_grid_size(settings.get_value_size(EditorSettings::map_grid_size));

  tileset_view_settings.set_zoom(
    settings.get_value_double(EditorSettings::map_tileset_zoom));

  reload_settings();
}

}
