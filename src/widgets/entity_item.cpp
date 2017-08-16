/*
 * Copyright (C) 2014-2017 Christopho, Solarus - http://www.solarus-games.org
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
#include "widgets/entity_item.h"
#include "widgets/gui_tools.h"
#include "widgets/map_scene.h"
#include "entities/entity_model.h"
#include "view_settings.h"
#include <QPainter>

namespace SolarusEditor {

/**
 * @brief Creates an entity item.
 * @param entity The entity to represent.
 * @param parent The parent item or nullptr.
 */
EntityItem::EntityItem(EntityModel& entity, QGraphicsItem* parent) :
  QGraphicsItem(parent),
  entity(entity),
  size(entity.get_size()) {

  update_xy();
  setFlags(ItemIsSelectable | ItemIsFocusable);
}

/**
 * @brief Returns the entity represented by this item.
 * @return The entity.
 */
EntityModel& EntityItem::get_entity() const {
  return entity;
}

/**
 * @brief Returns the index of the entity on the map.
 * @return The entity index or an invalid index if the entity is not on the map.
 */
EntityIndex EntityItem::get_index() const {
  return entity.get_index();
}

/**
 * @brief Returns the type of map entity represented by this item.
 * @return The entity type.
 */
EntityType EntityItem::get_entity_type() const {

  return entity.get_type();
}

/**
 * @brief Returns the bounding rectangle of the entity item.
 * @return The bounding rectangle.
 */
QRectF EntityItem::boundingRect() const {

  return QRect(QPoint(), size);
}

/**
 * @brief Shows or hides this entity item according to view settings.
 * @param view_settings The settings to apply.
 */
void EntityItem::update_visibility(const ViewSettings& view_settings) {

  int layer = entity.get_layer();
  EntityType type = get_entity_type();
  bool traversable = entity.is_traversable();

  const bool visible = view_settings.is_layer_visible(layer) &&
      view_settings.is_entity_type_visible(type) &&
      ((traversable && view_settings.are_traversables_visible()) ||
       (!traversable && view_settings.are_obstacles_visible()));
  setVisible(visible);
}

/**
 * @brief Sets the position of this entity item according to the model.
 */
void EntityItem::update_xy() {

  const QPoint& entity_top_left = entity.get_top_left();
  setPos(MapScene::get_margin_top_left() + entity_top_left);
}

/**
 * @brief Sets the size of this entity item according to the model.
 */
void EntityItem::update_size() {

  // prepareGeometryChange() tells Qt the result of boundingRect() will change.
  prepareGeometryChange();
  this->size = entity.get_size();  // TODO this is not true for entities whose sprite is larger, like NPCs
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

  // First, paint the item like if there was no selection, to avoid
  // Qt's built-in selection marker.
  const bool selected = option->state & QStyle::State_Selected;
  QStyleOptionGraphicsItem option_deselected = *option;
  option_deselected.state &= ~QStyle::State_Selected;
  entity.draw(*painter);

  // Add our selection marker.
  if (selected) {
    QColor line_color = Qt::green;
    QRect bounding_rect = boundingRect().toRect();

    // Draw both a thick line and a a line with thickness zero
    // so that the thickness of the selection rectangle grows with the zoom,
    // but is also still visible with low zoom settings.
    GuiTools::draw_rectangle_border(
          *painter,
          bounding_rect,
          line_color,
          1);

    painter->setPen(QPen(line_color, 0, Qt::SolidLine));
    painter->drawRect(bounding_rect);
  }
}

}
