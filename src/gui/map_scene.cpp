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
#include "gui/gui_tools.h"
#include "gui/map_scene.h"
#include "map_model.h"
#include <QDebug>
#include <QGraphicsItem>
#include <QPainter>

/**
 * @brief Graphic item representing a map entity.
 */
class EntityItem : public QGraphicsItem {

public:

  EntityItem(MapModel& model, const EntityIndex& index);

  // Enable the use of qgraphicsitem_cast with this item.
  enum {
    Type = UserType + 2
  };

  virtual int type() const override {
    return Type;
  }

  EntityIndex get_index() const;
  void set_index(const EntityIndex& index);

  QRectF boundingRect() const override;

protected:

  void paint(QPainter* painter,
                     const QStyleOptionGraphicsItem* option,
                     QWidget* widget = nullptr) override;

private:

  MapModel& map;              /**< The map this entity belongs to. */
  EntityIndex index;          /**< Index of the entity in the map. */

};

/**
 * @brief Creates a map scene.
 * @param model The map data to represent in the scene.
 * @param parent The parent object or nullptr.
 */
MapScene::MapScene(MapModel& model, QObject* parent) :
  QGraphicsScene(parent),
  model(model) {

  build();

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
 * @brief Converts quest coordinates to scene coordinates.
 * @param coordinate The value to convert.
 * @return The converted value.
 */
int MapScene::quest_to_scene(int coordinate) {

  return coordinate * quest_to_scene_factor;
}

/**
 * @brief Converts quest coordinates to scene coordinates.
 * @param coordinate The value to convert.
 * @return The converted value.
 */
int MapScene::scene_to_quest(int coordinate) {

  return coordinate / quest_to_scene_factor;
}

/**
 * @brief Converts quest coordinates to scene coordinates.
 * @param point The value to convert.
 * @return The converted value.
 */
QPoint MapScene::quest_to_scene(const QPoint& point) {

  return point * quest_to_scene_factor;
}

/**
 * @brief Converts quest coordinates to scene coordinates.
 * @param point The value to convert.
 * @return The converted value.
 */
QPoint MapScene::scene_to_quest(const QPoint& point) {

  return point / quest_to_scene_factor;
}

/**
 * @brief Converts quest coordinates to scene coordinates.
 * @param rectangle The value to convert.
 * @return The converted value.
 */
QRect MapScene::quest_to_scene(const QRect& rectangle) {

  return QRect(rectangle.topLeft() * quest_to_scene_factor,
               rectangle.size() * quest_to_scene_factor);
}

/**
 * @brief Converts quest coordinates to scene coordinates.
 * @param rectangle The value to convert.
 * @return The converted value.
 */
QRect MapScene::scene_to_quest(const QRect& rectangle) {

  return QRect(rectangle.topLeft() / quest_to_scene_factor,
               rectangle.size() / quest_to_scene_factor);
}

/**
 * @brief Converts quest coordinates to scene coordinates.
 * @param rectangle The value to convert.
 * @return The converted value.
 */
QSize MapScene::quest_to_scene(const QSize& size) {

  return size * quest_to_scene_factor;
}

/**
 * @brief Converts quest coordinates to scene coordinates.
 * @param rectangle The value to convert.
 * @return The converted value.
 */
QSize MapScene::scene_to_quest(const QSize& size) {

  return size / quest_to_scene_factor;
}

/**
 * @brief Create all entity items in the scene.
 */
void MapScene::build() {


  setSceneRect(QRectF(QPoint(0, 0), model.get_size()));
  for (int i = 0; i < Layer::LAYER_NB; ++i) {
    Layer layer = static_cast<Layer>(i);
    entity_items[i].clear();
    for (int j = 0; j < model.get_num_entities(layer); ++j) {
      create_entity_item(EntityIndex(layer, j));
    }
  }

}

/**
 * @brief Creates a graphic item for the specified entity.
 * @param index Index of a map entity.
 */
void MapScene::create_entity_item(const EntityIndex& index) {

  EntityItem* item = new EntityItem(model, index);
  addItem(item);
  entity_items[index.layer].append(item);
}

/**
 * @brief Creates an entity item.
 * @param map The map.
 * @param index Index of the entity on the map.
 */
EntityItem::EntityItem(MapModel& map, const EntityIndex& index) :
  QGraphicsItem(),
  map(map),
  index(index) {

  QRect frame = map.get_entity_bounding_box(index);
  setPos(MapScene::quest_to_scene(frame.topLeft()));
  setFlags(ItemIsSelectable | ItemIsFocusable);
}

/**
 * @brief Returns the index of the entity on the map.
 * @return The entity index.
 */
EntityIndex EntityItem::get_index() const {
  return index;
}

/**
 * @brief Sets the index of the entity on the map.
 * @param index The new index.
 */
void EntityItem::set_index(const EntityIndex& index) {
  this->index = index;
}

/**
 * @brief Returns the bounding rectangle of the entity item.
 * @return The bounding rectangle.
 */
QRectF EntityItem::boundingRect() const {

  return QRect(QPoint(0, 0), MapScene::quest_to_scene(map.get_entity_size(index)));
}

/**
 * @brief Paints the pattern item.
 *
 * Draws our own selection marker.
 *
 * @param painter The painter.
 * @param option Style option of the item.
 * @param widget The widget being painted or nullptr.
 */
void EntityItem::paint(QPainter* painter,
                        const QStyleOptionGraphicsItem* option,
                        QWidget* /* widget */) {

  if (!map.entity_exists(index)) {
    // Bug in the editor.
    qCritical() << MapScene::tr("No such entity index on layer %1: %2").arg(index.layer, index.index);
    return;
  }

  const bool selected = option->state & QStyle::State_Selected;

  // First, paint the item like if there was no selection, to avoid
  // Qt's built-in selection marker.
  QStyleOptionGraphicsItem option_deselected = *option;
  option_deselected.state &= ~QStyle::State_Selected;
  map.draw_entity(index, *painter);

  // Add our selection marker.
  if (selected) {
    GuiTools::draw_rectangle_outline(*painter, boundingRect().toRect(), Qt::blue);
  }
}
