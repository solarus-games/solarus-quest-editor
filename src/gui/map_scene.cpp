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
#include "gui/entity_item.h"
#include "gui/map_scene.h"
#include "map_model.h"
#include "tileset_model.h"
#include "view_settings.h"
#include <QDebug>
#include <QPainter>

/**
 * @brief Creates a map scene.
 * @param model The map data to represent in the scene.
 * @param parent The parent object or nullptr.
 */
MapScene::MapScene(MapModel& model, QObject* parent) :
  QGraphicsScene(parent),
  model(model) {

  build();

  connect(&model, SIGNAL(entity_added(EntityIndex)),
          this, SLOT(entity_added(EntityIndex)));
  connect(&model, SIGNAL(entity_xy_changed(EntityIndex, QPoint)),
          this, SLOT(entity_xy_changed(EntityIndex, QPoint)));
}

/**
 * @brief Returns the map represented in the scene.
 * @return The map.
 */
const MapModel& MapScene::get_model() const {
  return model;
}

/**
 * @brief Returns the quest the map belongs to.
 * @return The quest.
 */
const Quest& MapScene::get_quest() const {
  return model.get_quest();
}

/**
 * @brief Returns the coordinates of the point (0,0) of the map on the scene.
 *
 * It is different due to the margin.
 *
 * @return The top-left map corner coordinates on the scene.
 */
QPoint MapScene::get_margin_top_left() {

  const QSize margin = get_margin_size();
  return QPoint(margin.width(), margin.height());
}

/**
 * @brief Returns the size of the margin around the map.
 * @return The margin in scene coordinates.
 */
QSize MapScene::get_margin_size() {

  const QSize margin(64, 64);
  return margin;
}

/**
 * @brief Create all entity items in the scene.
 */
void MapScene::build() {

  setSceneRect(QRectF(QPoint(0, 0), (get_margin_size() * 2) + model.get_size()));
  setBackgroundBrush(Qt::gray);

  for (int i = 0; i < Layer::LAYER_NB; ++i) {
    Layer layer = static_cast<Layer>(i);
    entity_items[i].clear();
    for (int j = 0; j < model.get_num_entities(layer); ++j) {
      EntityModel& entity = model.get_entity(EntityIndex(layer, j));
      create_entity_item(entity);
    }
  }

}

/**
 * @brief Creates a graphic item for the specified entity.
 * @param model A map entity.
 */
void MapScene::create_entity_item(EntityModel& entity) {

  if (!entity.is_on_map()) {
    qCritical() << "This entity is not on the map";
    return;
  }

  EntityItem* item = new EntityItem(entity);
  addItem(item);
  entity_items[entity.get_layer()].append(item);
}

/**
 * @brief Returns the graphic item of the specified entity.
 * @param index Index of a map entity.
 * @return The corresponding item or nullptr if there is no such entity.
 */
EntityItem* MapScene::get_entity_item(const EntityIndex& index) {

  if (!index.is_valid()) {
    return nullptr;
  }

  const EntityList& entities = get_entities(index.layer);
  if (index.index < 0 || index.index >= entities.size()) {
    // Index out of range.
    return nullptr;
  }

  return entities.at(index.index);
}

/**
 * @brief Returns the entity items on the specified layer.
 * @param layer A layer.
 * @return Items of entities on that layer.
 */
const MapScene::EntityList& MapScene::get_entities(Layer layer) {
  return entity_items[layer];
}

/**
 * @brief Shows or hides entities on a layer.
 * @param layer The layer to update.
 * @param view_settings The new view settings to apply.
 */
void MapScene::update_layer_visibility(Layer layer, const ViewSettings& view_settings) {

  for (EntityItem* item : get_entities(layer)) {
    item->update_visibility(view_settings);
  }
}

/**
 * @brief Shows or hides entities of the specified type.
 * @param type The entity type to change.
 * @param view_settings The new view settings to apply.
 */
void MapScene::update_entity_type_visibility(EntityType type, const ViewSettings& view_settings) {

  for (int i = 0; i < Layer::LAYER_NB; ++i) {
    Layer layer = static_cast<Layer>(i);
    for (EntityItem* item : get_entities(layer)) {
      if (item->get_entity_type() == type) {
        item->update_visibility(view_settings);
      }
    }
  }
}

/**
 * @brief Draws the tileset's background color as background of the map.
 * @param painter The painter.
 * @param rect The exposed rectangle in scene coordinates.
 * It may be larger than the scene.
 */
void MapScene::drawBackground(QPainter* painter, const QRectF& rect) {

  // Call the parent class to have the correct color in margins.
  QGraphicsScene::drawBackground(painter, rect);

  // Draw the background color from the tileset.
  TilesetModel* tileset = model.get_tileset_model();
  if (tileset == nullptr) {
    return;
  }

  QRect rect_no_margins = rect.toRect().intersected(QRect(get_margin_top_left(), model.get_size()));
  painter->fillRect(rect_no_margins, tileset->get_background_color());
}

/**
 * @brief Returns the entity represented by the specified item.
 * @param item An item of the scene.
 * @return The entity, or nullptr if the item does not
 * represent a map entity.
 */
EntityModel* MapScene::get_entity_from_item(const QGraphicsItem& item) {

  const EntityItem* entity_item = qgraphicsitem_cast<const EntityItem*>(&item);
  if (entity_item == nullptr) {
   // Not a map entity.
    return nullptr;
  }

  return &entity_item->get_entity();
}

/**
 * @brief Slot called when an entity is added to the map.
 *
 * An item on the scene is created accordingly.
 *
 * @param index Index of the new entity.
 */
void MapScene::entity_added(const EntityIndex& index) {

  if (!model.entity_exists(index)) {
    // Bug in the map editor.
    qCritical() << "Cannot find added entity";
    return;
  }

  create_entity_item(model.get_entity(index));
}

/**
 * @brief Slot called when the position of an entity has changed.
 *
 * Its item on the scene is updated accordingly.
 *
 * @param index Index of an entity.
 * @param xy Its new position.
 */
void MapScene::entity_xy_changed(const EntityIndex& index, const QPoint& xy) {

  Q_UNUSED(xy);

  EntityItem* item = get_entity_item(index);
  if (item == nullptr) {
    // Bug in the map editor.
    qCritical() << "Missing entity item";
  }

  item->update_xy();
}

/**
 * @brief Returns the indexes of selected entities.
 * @return The selected entities.
 */
QList<EntityIndex> MapScene::get_selected_entities() {

  QList<EntityIndex> result;
  for (QGraphicsItem* item : selectedItems()) {
    EntityModel* entity = get_entity_from_item(*item);
    if (entity == nullptr) {
      continue;
    }
    EntityIndex index = entity->get_index();
    if (!index.is_valid()) {
      continue;
    }
    result.append(index);
  }
  return result;
}


/**
 * @brief Selects the specified entities and unselect the rest.
 * @param indexes Indexes of the entities to make selecteded.
 */
void MapScene::set_selected_entities(const QList<EntityIndex>& indexes) {

  clearSelection();
  for (const EntityIndex& index : indexes) {
    EntityItem* item = get_entity_item(index);
    item->setSelected(true);
  }
}
